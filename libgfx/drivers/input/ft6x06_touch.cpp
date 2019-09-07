/**
 * =====================================================================================
 * 	File: ft6x06_touch.cpp
 * 	Created Date: Monday, September 2nd 2019, 10:05:20 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#include <gfx/drivers/input/ft6x06_touch.hpp>
#include <foundation/sys/dbglog.h>
#include <periph/drivers/i2c/i2c_master.hpp>
#include <periph/dt/dts.hpp>
#include <periph/drivers/display/rgb/display_config.hpp>
#include "ft6x06_regs.hpp"


namespace gfx::drv {

namespace {
    // Print touch tag for debug purposes
    void print_touch(const gfx::input::detail::touch_tag& tag) {
        dbg_info("Num touches %i getid %i",tag.num_touches,tag.gestureid);
        for(int i=0;i<tag.num_touches;++i) {
            dbg_info("xy: %i,%i weight: %i area %i",tag.x[i],tag.y[i],tag.weight[i],tag.area[i]);
            dbg_info("event_id: %i", tag.eventid[i]);
        }
    }
}

//! Swap status
enum ts_swap : uint8_t {
    TS_SWAP_NONE = 1,
    TS_SWAP_X = 2,
    TS_SWAP_Y  = 4,
    TS_SWAP_XY = 8
};

//Constructor
ft6x06_touch::ft6x06_touch(const char name[], periph::drivers::i2c_master& i2c,gui::frame& frame)
    : input_touchpad(frame),m_i2c(i2c),
       m_thr( isix::thread_create( std::bind(&ft6x06_touch::thread,std::ref(*this)))),
       m_name(name)
{}

// Start the main thread
void ft6x06_touch::start() noexcept
{
    m_thr.start_thread(1024,isix::get_min_priority());
}
// Detect device qand assign address
int ft6x06_touch::detect_device() noexcept
{
	using namespace detail::regs::ft6x06;
    int ret {};
	uint8_t value {};
    do {
       ret =  read_reg(c_ts_addr,CHIP_ID_REG,value);
	   if(ret||value!=ID_VALUE) {
		   dbg_info("Invalid id reg bus_err: %i val: %i!=%i", ret,value,ID_VALUE);
		   ret = read_reg(c_ts_addr2,CHIP_ID_REG,value);
		   if(ret||value!=ID_VALUE) {
               dbg_info("Invalid id reg bus_err: %i val: %i!=%i", ret,value,ID_VALUE);
               if(!ret) ret = periph::error::init;
			   break;
		   } else {
               m_addr = c_ts_addr2;
           }
	   } else {
           m_addr = c_ts_addr;
       }
    } while (0);
    return ret;
}

//! Calibrate device
int ft6x06_touch::calibrate() noexcept
{
	using namespace detail::regs::ft6x06;
    uint8_t rd_data;
    uint8_t reg_val;
    int status {};

    /* >> Calibration sequence start */
    do {
        /* Switch FT6206 back to factory mode to calibrate */
        reg_val = (FT6206_DEV_MODE_FACTORY & FT6206_DEV_MODE_MASK) << FT6206_DEV_MODE_SHIFT;
        status = write_reg(m_addr, FT6206_DEV_MODE_REG, reg_val); /* 0x40 */
        if(status) break;

        /* Read back the same register FT6206_DEV_MODE_REG */
        status = read_reg(m_addr, FT6206_DEV_MODE_REG, rd_data);
        if (status) break;
        isix::wait_ms(300); /* Wait 300 ms */

        if (((rd_data & (FT6206_DEV_MODE_MASK << FT6206_DEV_MODE_SHIFT)) >> FT6206_DEV_MODE_SHIFT) != FT6206_DEV_MODE_FACTORY)
        {
            /* Return error to caller */
            status = periph::error::inval;
            break;
        }
        /* Start calibration command */
        status = write_reg(m_addr, FT6206_TD_STAT_REG, 0x04);
        if(status) break;
        isix::wait_ms(300); /* Wait 300 ms */

        /* 100 attempts to wait switch from factory mode (calibration) to working mode */
        bool end_cal {};
        for ( int retries = 0; ((retries < 100) && (!end_cal)); retries++)
        {
            status = read_reg(m_addr, FT6206_DEV_MODE_REG, rd_data);
            if(status) break;
            rd_data = (rd_data & (FT6206_DEV_MODE_MASK << FT6206_DEV_MODE_SHIFT)) >> FT6206_DEV_MODE_SHIFT;
            if (rd_data == FT6206_DEV_MODE_WORKING)
            {
                /* Auto Switch to FT6206_DEV_MODE_WORKING : means calibration have ended */
                end_cal = true; /* exit for loop */
            }
            isix::wait_ms(200); /* Wait 200 ms */
        }
    } while(0);
    /* Calibration sequence end << */
    return status;
}
// Disable it
int ft6x06_touch::disable_it() noexcept
{

	using namespace detail::regs::ft6x06;
    uint8_t reg_val = 0;
    reg_val = (FT6206_G_MODE_INTERRUPT_POLLING & (FT6206_G_MODE_INTERRUPT_MASK >> FT6206_G_MODE_INTERRUPT_SHIFT)) << FT6206_G_MODE_INTERRUPT_SHIFT;
    /* Set interrupt polling mode in FT6206_GMODE_REG */
    return write_reg(m_addr, FT6206_GMODE_REG, reg_val);
}

//Touch screen enable
int ft6x06_touch::touch_enable(unsigned short sizex, unsigned short sizey) noexcept
{
    if(sizex<sizey) {
        m_orientation = TS_SWAP_NONE;
    } else {
        m_orientation = TS_SWAP_XY | TS_SWAP_Y;
    }
    //Start the device
    int ret {};
    do {
        ret = calibrate();
        if(ret) break;
        ret = disable_it();
        if(ret) break;
    } while(0);
    return ret;
}
//Initialize touchpad
int ft6x06_touch::initialize() noexcept
{
    int ret {};
    do {
       ret = detect_device(); 
       if(ret) break;
       const periph::dt::device_conf_base* base {};
       ret = periph::dt::get_periph_devconf(m_name,base);
       if(ret) break;
       const auto entry = reinterpret_cast<const periph::display::fb_info*>(base);
       if(entry->n_layers<1) {
            ret = periph::error::inval;
            break;
       }
       ret = touch_enable(entry->layers[0].width, entry->layers[0].height);
       if(ret) break;
    } while(0);
    return ret;
}


// Get identified gesture
int ft6x06_touch::get_gesture_code() noexcept
{
	using namespace detail::regs::ft6x06;
    using namespace gfx::input;
    uint8_t gesture_id {};
    int ret {};
    ret = read_reg(m_addr,FT6206_GEST_ID_REG, gesture_id);
    if(ret) return ret;
    /* Remap gesture Id to a TS_GestureIdTypeDef value */
    switch (gesture_id)
    {
    case FT6206_GEST_ID_NO_GESTURE:
        ret = touchgestures::undefined;
        break;
    case FT6206_GEST_ID_MOVE_UP:
        ret = touchgestures::move_up;
        break;
    case FT6206_GEST_ID_MOVE_RIGHT:
        ret = touchgestures::move_right;
        break;
    case FT6206_GEST_ID_MOVE_DOWN:
        ret = touchgestures::move_down;
        break;
    case FT6206_GEST_ID_MOVE_LEFT:
        ret = touchgestures::move_left;
        break;
    case FT6206_GEST_ID_ZOOM_IN:
        ret = touchgestures::zoom_in;
        break;
    case FT6206_GEST_ID_ZOOM_OUT:
        ret = touchgestures::zoom_out;
        break;
    default:
        ret = gfx::error::error_not_supported;
        break;
    } /* of switch(gestureId) */
    return ret;
}

//Return number of active touches 0,1,2
int ft6x06_touch::detect_touch() noexcept
{
    using namespace detail::regs::ft6x06;
    uint8_t nb_touch {};
    /* Read register FT6206_TD_STAT_REG to check number of touches detection */
    const auto ret = read_reg(m_addr, FT6206_TD_STAT_REG, nb_touch);
    nb_touch &= FT6206_TD_STAT_MASK;
    if (nb_touch > c_max_nb_touch) {
        nb_touch = 0;
    }
    m_curr_act_touch_nb = nb_touch;
    m_curr_act_touch_id = 0;
    return ret?ret:nb_touch;
}

//Get XY position 
int ft6x06_touch::get_xy(uint16_t& x, uint16_t& y)
{
    using namespace detail::regs::ft6x06;
	uint8_t regAddress = 0;
	uint8_t  dataxy[4];
	if(m_curr_act_touch_id < m_curr_act_touch_nb)
	{
		switch(m_curr_act_touch_id)
		{
			case 0 :    
				regAddress = FT6206_P1_XH_REG; 
				break;
			case 1 :
				regAddress = FT6206_P2_XH_REG; 
				break;
			default :
				break;
		}

		/* Read X and Y positions */
		auto ret = read_reg(m_addr, regAddress, dataxy, sizeof(dataxy)); 
        if(ret) {
            return ret;
        }

		/* Send back ready X position to caller */
		x = ((dataxy[0] & FT6206_MSB_MASK) << 8) | (dataxy[1] & FT6206_LSB_MASK);

		/* Send back ready Y position to caller */
		y = ((dataxy[2] & FT6206_MSB_MASK) << 8) | (dataxy[3] & FT6206_LSB_MASK);
		m_curr_act_touch_id++;
	}
    return error::error_ok;
}

// Get gesture state
int ft6x06_touch::get_state(touch_stat& stat) noexcept
{
    using namespace detail::regs::ft6x06;
    using namespace gfx::input;
    int ts_status;
    do {
        /* Check and update the number of touches active detected */
        ts_status = detect_touch();
        if (ts_status<0) break;
        stat.num_touches = ts_status;
        ts_status = 0;
        if (stat.num_touches>0)
        {
            uint16_t Raw_x[c_max_nb_touch];
            uint16_t Raw_y[c_max_nb_touch];
            for (int index = 0; index < stat.num_touches; index++)
            {
                /* Get each touch coordinates */
                ts_status = get_xy( (Raw_x[index]), (Raw_y[index]));
                if(ts_status) break;
                if (m_orientation & TS_SWAP_XY)
                {
                    auto tmp = Raw_x[index];
                    Raw_x[index] = Raw_y[index];
                    Raw_y[index] = tmp;
                }

                if (m_orientation & TS_SWAP_X)
                {
                    Raw_x[index] = FT_6206_MAX_WIDTH - 1 - Raw_x[index];
                }

                if (m_orientation & TS_SWAP_Y)
                {
                    Raw_y[index] = FT_6206_MAX_HEIGHT - 1 - Raw_y[index];
                }

                auto xDiff = Raw_x[index] > m_x[index] ? (Raw_x[index] - m_x[index]) : (m_x[index] - Raw_x[index]);
                auto yDiff = Raw_y[index] > m_y[index] ? (Raw_y[index] - m_y[index]) : (m_y[index] - Raw_y[index]);

                if ((xDiff + yDiff) > 5)
                {
                    m_x[index] = Raw_x[index];
                    m_y[index] = Raw_y[index];
                }

                stat.x[index] = m_x[index];
                stat.y[index] = m_y[index];
                uint32_t weight {};
                uint32_t area {};
                uint32_t event {};
                /* Get touch info related to the current touch */
                ts_status = get_info(index, weight, area, event);
                if(ts_status) break;

                /* Update TS_State structure */
                stat.weight[index] = weight;
                stat.area[index] = area;

                /* Remap touch event */
                switch (event)
                {
                case FT6206_TOUCH_EVT_FLAG_PRESS_DOWN:
                    stat.eventid[index] = touchevents::press_down;
                    break;
                case FT6206_TOUCH_EVT_FLAG_LIFT_UP:
                    stat.eventid[index] = touchevents::lift_up;
                    break;
                case FT6206_TOUCH_EVT_FLAG_CONTACT:
                    stat.eventid[index] = touchevents::contact;
                    break;
                case FT6206_TOUCH_EVT_FLAG_NO_EVENT:
                    stat.eventid[index] = touchevents::undefined;
                    break;
                default:
                    ts_status = error::error_not_supported;
                    break;
                } /* of switch(event) */

            } /* of for(index=0; index < TS_State->touchDetected; index++) */

            /* Get gesture Id */
            ts_status = get_gesture_code();
            if(ts_status<0) break;
        } /* end of if(TS_State->touchDetected != 0) */
    } while (0) ;
    return ts_status?ts_status:stat.num_touches;
}

//Get touch info
int ft6x06_touch::get_info(uint32_t touch_idx, uint32_t& weight, uint32_t& area, uint32_t& event) noexcept
{
    uint8_t regAddress = 0;
    uint8_t dataxy[3];
    using namespace detail::regs::ft6x06;
    if (touch_idx < m_curr_act_touch_nb)
    {
        switch (touch_idx)
        {
        case 0:
            regAddress = FT6206_P1_WEIGHT_REG;
            break;

        case 1:
            regAddress = FT6206_P2_WEIGHT_REG;
            break;

        default:
            break;

        } /* end switch(touchIdx) */

        /* Read weight, area and Event Id of touch index */
        auto ret = read_reg(m_addr, regAddress, dataxy, sizeof(dataxy));
        if(ret) return ret;

        /* Return weight of touch index */
        weight = (dataxy[0] & FT6206_TOUCH_WEIGHT_MASK) >> FT6206_TOUCH_WEIGHT_SHIFT;
        /* Return area of touch index */
        area = (dataxy[1] & FT6206_TOUCH_AREA_MASK) >> FT6206_TOUCH_AREA_SHIFT;
        /* Return Event Id  of touch index */
        event = (dataxy[2] & FT6206_TOUCH_EVT_FLAG_MASK) >> FT6206_TOUCH_EVT_FLAG_SHIFT;

    } /* of if(touchIdx < ft6x06_handle.currActiveTouchNb) */
    return error::error_ok;
}

// Main thread for read input events
void ft6x06_touch::thread() {
    int ret;
    isix::wait_ms(500);
    //! Initialize touchpad hardware into the poll mode
    if((ret=initialize())) {
        dbg_err("Unable to configure touchpad errno: %i",ret);
        return;
    } else {
        dbg_info("Touchpad initialized");
    }
    for(touch_stat ts={};;ts={}) {
       isix::wait_ms(50);
       ret = get_state(ts);
       if(ret>0) {  //! If number of touches is greater than one
           //Print touch info
           print_touch(ts); 
           //Report touch to the gui library
           report_touch(std::move(ts));
       } else if(ret<0) {
           dbg_err("Touch screen failed");
       }
    }
}

//Read reg helper functi
int ft6x06_touch::read_reg(int addr, int reg, unsigned char& value)
{
    unsigned char baddr = reg&0xff;
    periph::blk::trx_transfer tran(&baddr,&value, sizeof baddr, sizeof value);
    return m_i2c.transaction(addr, tran);
}

int ft6x06_touch::read_reg(int addr, int reg, unsigned char* regs, unsigned short regs_siz) noexcept
{
    unsigned char baddr = reg&0xff;
    periph::blk::trx_transfer tran(&baddr,regs, sizeof baddr, regs_siz);
    return m_i2c.transaction(addr, tran);
}

//Write register
int ft6x06_touch::write_reg(int addr, int reg, unsigned char value)
{
    const unsigned char buf[] = { static_cast<unsigned char>(reg), value };
    periph::blk::tx_transfer tran(buf, sizeof buf);
    return m_i2c.transaction(addr, tran);
}


}

