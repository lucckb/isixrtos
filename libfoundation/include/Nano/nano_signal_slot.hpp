/*------------------------------------------------------------------------------
----------------------// Version: 1.06 // License: MIT //-----------------------
--------------------------------------------------------------------------------

Copyright (c) 2012-2013 ApEk, Nano-signal-slot Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

--------------------------------------------------------------------------------
----------------------// Version: 1.06 // License: MIT //-----------------------
------------------------------------------------------------------------------*/

#ifndef NANO_SIGNAL_SLOT_HPP
#define NANO_SIGNAL_SLOT_HPP

#include <unordered_map>
#include <vector>

namespace Nano
{

typedef std::pair<std::uintptr_t, std::uintptr_t> delegate_key_t;

template <typename Re_t> class function;
template <typename Re_t, typename... Args> class function<Re_t(Args...)>
{
    void *m_this_ptr;
    Re_t(*m_stub_ptr)(void*, Args...);

    template <typename I, typename F>
    function (I&& this_ptr, F&& stub_ptr) :
        m_this_ptr { std::forward<I>(this_ptr) },
        m_stub_ptr { std::forward<F>(stub_ptr) } { }

    public:

    template <Re_t (*func_ptr)(Args...)>
    static inline function bind()
    {
        return { nullptr, [] (void *, Args... args) {
            return (*func_ptr)(args...); } };
    }
    template <typename T, Re_t (T::*meth_ptr)(Args...)>
    static inline function bind(T* pointer)
    {
        return { pointer, [] (void *this_ptr, Args... args) {
            return (static_cast<T*>(this_ptr)->*meth_ptr)(args...); } };
    }
    template <typename T, Re_t (T::*meth_ptr)(Args...) const>
    static inline function bind(T* pointer)
    {
        return { pointer, [] (void *this_ptr, Args... args) {
            return (static_cast<const T*>(this_ptr)->*meth_ptr)(args...); } };
    }

    Re_t operator()(Args&&... args) const
    {
        return (*m_stub_ptr)(m_this_ptr, args...);
    }
    bool operator== (delegate_key_t const& other) const
    {
        return this->operator delegate_key_t() == other;
    }
    bool operator!= (delegate_key_t const& other) const
    {
        return this->operator delegate_key_t() != other;
    }
    operator delegate_key_t ( ) const
    {
        return { reinterpret_cast<std::uintptr_t>(m_this_ptr),
                 reinterpret_cast<std::uintptr_t>(m_stub_ptr) };
    }
};

} // namespace Nano ------------------------------------------------------------

namespace std // std::hash specialization
{

template <> struct hash<Nano::delegate_key_t>
{
    inline std::size_t operator() (Nano::delegate_key_t const& key) const
    {
        return key.first + 0x9E3779B9 + (key.second << 6) + (key.second >> 2);
    }
};

} // namespace std -------------------------------------------------------------

namespace Nano
{

struct tracked
{
    template <typename T> friend class signal;

    private:

    std::unordered_map<delegate_key_t, tracked*> tracked_connections;

    virtual void remove_tracked(delegate_key_t const& key)
    {
        tracked_connections.erase(key);
    }

    protected:

   ~tracked ()
    {
        for (auto const& connection : tracked_connections)
        {
            connection.second->remove_tracked(connection.first);
        }
    }
};

//------------------------------------------------------------------------------

template <typename Re_t> class signal;
template <typename Re_t, typename... Args>
class signal<Re_t(Args...)> : public Nano::tracked
{
    typedef Nano::function<Re_t(Args...)> function;
    std::unordered_map<delegate_key_t, function> m_slots;

    template <typename T>
    void sfinae_con(delegate_key_t const& key, typename T::tracked* instance)
    {
        instance->tracked_connections.emplace(key, this);
        tracked_connections.emplace(key, instance);
    }
    template <typename T>
    void sfinae_dis(delegate_key_t const& key, typename T::tracked* instance)
    {
        instance->tracked_connections.erase(key);
        tracked_connections.erase(key);
    }

    template <typename T> void sfinae_con(...) { }
    template <typename T> void sfinae_dis(...) { }

    virtual void remove_tracked(delegate_key_t const& key)
    {
        tracked_connections.erase(key);
        m_slots.erase(key);
    }

    public:

    template <Re_t (*func_ptr)(Args...)>
    void connect()
    {
        auto delegate = function::template bind<func_ptr>();
        m_slots.emplace(delegate, delegate);
    }
    template <typename T, Re_t (T::*meth_ptr)(Args...)>
    void connect(T* instance)
    {
        auto delegate = function::template bind<T, meth_ptr>(instance);
        m_slots.emplace(delegate, delegate);
        sfinae_con<T>(delegate, instance);
    }
    template <typename T, Re_t (T::*meth_ptr)(Args...) const>
    void connect(T* instance)
    {
        auto delegate = function::template bind<T, meth_ptr>(instance);
        m_slots.emplace(delegate, delegate);
        sfinae_con<T>(delegate, instance);
    }

//------------------------------------------------------------------------------

    template <Re_t (*func_ptr)(Args... )>
    void disconnect()
    {
        auto delegate = function::template bind<func_ptr>();
        m_slots.erase(delegate);
    }
    template <typename T, Re_t (T::*meth_ptr)(Args...)>
    void disconnect(T* instance)
    {
        auto delegate = function::template bind<T, meth_ptr>(instance);
        sfinae_dis<T>(delegate, instance);
        m_slots.erase(delegate);
    }
    template <typename T, Re_t (T::*meth_ptr)(Args...) const>
    void disconnect(T* instance)
    {
        auto delegate = function::template bind<T, meth_ptr>(instance);
        sfinae_dis<T>(delegate, instance);
        m_slots.erase(delegate);
    }

//------------------------------------------------------------------------------

    void operator()(Args... args) // TODO deprecated!
    {
        for (auto const& slot : m_slots)
        {
            slot.second(std::forward<Args>(args)...);
        }
    }
    void emit(Args... args) const
    {
        for (auto const& slot : m_slots)
        {
            slot.second(std::forward<Args>(args)...);
        }
    }
    template <typename Transform>
    Re_t emit(Args... args) const
    {
        std::vector<Re_t> srv;
        srv.reserve(m_slots.size());

        for (auto const& slot : m_slots)
        {
            srv.emplace_back(slot.second(std::forward<Args>(args)...));
        }
        return Transform()(srv.cbegin(), srv.cend());
    }
};

} // namespace Nano ------------------------------------------------------------

#endif // NANO_SIGNAL_SLOT_HPP
