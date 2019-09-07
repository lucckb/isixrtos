/**
 * =====================================================================================
 * 	File: ft6x06_touch.hpp
 * 	Created Date: Monday, September 2nd 2019, 9:50:30 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#pragma once

#include "input_touchpad.hpp"
#include <isix/thread.hpp>

//! FWD decl
namespace periph::drivers {
    class i2c_master;
}
namespace gfx::drv {

    // ft6x06_touchpad driver for graphics library
    class ft6x06_touch final : protected input_touchpad {
        //Two types of touch screen i2c addresses
        static constexpr auto c_ts_addr = 0x54;
        //Two types of touch screen i2c addresses
        static constexpr auto c_ts_addr2 = 0x70;
        //Number of active touch
        static constexpr auto c_max_nb_touch = 2;
    public:
        // Alias
        using touch_stat = gfx::input::detail::touch_tag;
        //Constructor
        ft6x06_touch(const char disp_name[], periph::drivers::i2c_master& i2c,gui::frame& frame);
        // Start the main thread
        void start() noexcept;
    private:
        //! Calibrate device
        int calibrate() noexcept;
        // Disable it
        int disable_it() noexcept;
        //! Detect device and use address
        int detect_device() noexcept;
        //! Start probe 
        int touch_enable(unsigned short sizex, unsigned short sizey) noexcept;
        // Get identified gesture
        int get_gesture_code() noexcept;
        //Check if touch is detected or not
        //Return number of active touches 0,1,2
        int detect_touch() noexcept;
        //Get XY position 
        int get_xy(uint16_t& x, uint16_t& y) noexcept;
        //Get touch info
        int get_info(uint32_t touch_idx,uint32_t& weight, uint32_t& area, uint32_t& event) noexcept;
        // Configure the touchpad
        int initialize() noexcept;
        //Main i2c thread
        void thread();
        //Read reg helper function
        int read_reg(int addr, int reg, unsigned char& value) noexcept;
        int read_reg(int addr, int reg, unsigned char* regs, unsigned short regs_siz) noexcept;
        //Write register
        int write_reg(int addr, int reg, unsigned char value) noexcept;
    private:    // Touch ID external API
        // Get gesture state
        int get_state(touch_stat&) noexcept;
    private:
        periph::drivers::i2c_master& m_i2c;     //I2c controller
        isix::thread m_thr;                     //Thread handler
        unsigned char m_addr {};                //Used hardware address
        unsigned char m_orientation {};         //Touch screen orientation
        unsigned char m_curr_act_touch_nb {};   //Current active touch numbers
        unsigned char m_curr_act_touch_id {};   //Current active touch identifier
        const char * m_name;                    //Driver device name
        uint32_t m_x[c_max_nb_touch] {};         //Temporary touch buffer
        uint32_t m_y[c_max_nb_touch] {};         //Temporary touch buffer
    };
}

