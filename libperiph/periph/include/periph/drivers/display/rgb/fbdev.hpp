/**
 * =====================================================================================
 * 	File: fbdev.hpp
 *  Description: Framebuffer generic driver for all platforms
 * 	Created Date: Sunday, August 18th 2019, 7:40:19 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */
#include <cstddef>
#include <cstdint>

namespace periph::display {
    struct fb_info;
    struct layer_info;
}

namespace periph::display {
    class fbdev
    {
    public:
        //! Default constructor
        fbdev(const char dev_name[]);
        //! Default destructor
        ~fbdev() {}
        //! noncopyable stuff
        fbdev(fbdev&) = delete;
        fbdev& operator=(fbdev&) = delete;
        /** Open and enable framebuffer device
         * @return error code;
         */
        int open();
        /** Close and disable framebuffer device
         * @return error code
         */
        int close();
        /** Get framebuffer base memory
         * @return framebuffer address
         */
        void* fbmem(size_t idx=0) const noexcept;
    private:
        //! Hardware setup
        void hardware_setup(const layer_info& info, int i) noexcept;
        //! IOREAD and io write ops
        void iow(uint32_t reg, uint32_t val) const noexcept;
        uint32_t ior(uint32_t reg) const noexcept;
        auto io() const noexcept {
            return reinterpret_cast<void*>(m_base);
        }
        //! LTDC configure single layer
        void layer(const layer_info& info, int i) noexcept;
    private:
       uintptr_t m_base;
       const fb_info* m_fbinfo {};
    };
}