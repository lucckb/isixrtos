/*
 * =====================================================================================
 *
 *       Filename:  Fifo cpp wrapper for isix rtos
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02.04.2015 22:50:49
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#ifdef __cplusplus
#include <cstddef>

namespace isix {


    //! The base class for fifo contains only data
    class fifo_base
    {
    public:
            explicit fifo_base(osfifo_t hwnd_) : hwnd(hwnd_) {}
    protected:
            osfifo_t hwnd;
    private:
            fifo_base(const fifo_base&);
            fifo_base& operator=(const fifo_base&);
    };


    //! The C++ class wrapper for the queue
    template <typename T> class fifo : public fifo_base
    {
    public:
            /** Construct fifo object with the selected elements
            * @param n_elem Number of elements in the fifo
            */
            explicit fifo(std::size_t n_elem, unsigned flags=0)
                    : fifo_base(isix_fifo_create_ex(n_elem,sizeof(T),flags))
            {
            }
            //! Destruct fifo object
            ~fifo()
            {
                    isix_fifo_destroy(hwnd);
            }
            /** Check if the fifo object is in valid state
            * @return true if object is ok else return false
            */
            bool is_valid() { return hwnd!=0; }
            /* Push data in the queue
            * @param[in] c Reference to the object
            * @param[in] timeout Wait time when queue is not empty
            * @return ISIX_EOK if success else return an error
            */
            int push(const T &c,ostick_t timeout=0)
            {
                    return isix_fifo_write( hwnd, &c, timeout );
            }
            /* Push data in the queue from the ISR
            * @param[in] c Reference to the object
            * @param[in] timeout Wait time when queue is not empty
            * @return ISIX_EOK if success else return an error
            */
            int push_isr(const T &c)
            {
                    return isix_fifo_write_isr( hwnd, &c );
            }
            /** Get available elements in the fifo
            * @return available elements in the fifo
            */
            int size() const
            {
                    return isix_fifo_count( hwnd );
            }
            /** Pop the element from the queue
            * @param[in] c Reference to the buffer
            * @param[in] timeout Max waiting time
            * @return ISIX_EOK if success else return an error
            */
            int pop(T &c, ostick_t timeout=0)
            {
                    return isix_fifo_read( hwnd, &c, timeout );
            }
            /** Pop the element from the queue. Called from the ISR
            * @param[in] c Reference to the buffer
            * @param[in] timeout Max waiting time
            * @return ISIX_EOK if success else return an error
            */
            int pop_isr(T &c)
            {
                    return isix_fifo_read_isr( hwnd, &c );
            }
    private:
            fifo(const fifo&);
            fifo& operator=(const fifo&);
    };


}
#endif /* __cplusplus */
