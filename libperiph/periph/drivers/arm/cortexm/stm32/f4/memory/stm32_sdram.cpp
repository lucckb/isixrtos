/*
 * =====================================================================================
 *
 *       Filename:  stm32_dma_v2.hpp
 *
 *    Description:  SDRAM driver for ISIX devices
 *
 *        Version:  1.0
 *        Created:  10.08.2019 13:58:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <periph/drivers/memory/sdram.hpp>
#include <periph/memory/sdram_dtypes.hpp>
#include <periph/dt/dts.hpp>
#include <periph/clock/clocks.hpp>
#include <periph/gpio/gpio.hpp>
#include <periph/core/error.hpp>
#include <isix/arch/io.h>

namespace periph::memory {
#ifdef FMC_R_BASE
namespace {

constexpr auto	FMC_BCR1	= 0x00; /*   */
constexpr auto	FMC_BCR2	= 0x08; /*   */
constexpr auto	FMC_BCR3	= 0x10; /*   */
constexpr auto	FMC_BCR4	= 0x18; /*   */
constexpr auto	FMC_BTR1	= 0x04; /*   */
constexpr auto	FMC_BTR2	= 0x0C; /*   */
constexpr auto	FMC_BTR3	= 0x14; /*   */
constexpr auto	FMC_BTR4	= 0x1C; /*   */
constexpr auto	FMC_BWTR1	= 0x104; /*   */
constexpr auto	FMC_BWTR2	= 0x10C; /*   */
constexpr auto	FMC_BWTR3	= 0x104; /*   */
constexpr auto	FMC_BWTR4	= 0x10C; /*   */
constexpr auto	FMC_PCR2	= 0x60; /*   */
constexpr auto	FMC_PCR3	= 0x80; /*   */
constexpr auto	FMC_PCR4	= 0xA0; /*   */
constexpr auto	FMC_SR2		= 0x64; /*   */
constexpr auto	FMC_SR3		= 0x84; /*   */
constexpr auto	FMC_SR4		= 0xA4; /*   */
constexpr auto	FMC_PMEM2	= 0x68; /*   */
constexpr auto	FMC_PMEM3	= 0x88; /*   */
constexpr auto	FMC_PMEM4	= 0xA8; /*   */
constexpr auto	FMC_PATT2	= 0x6C; /*   */
constexpr auto	FMC_PATT3	= 0x8C; /*   */
constexpr auto	FMC_PATT4	= 0xAC; /*   */
constexpr auto	FMC_PIO4	= 0xB0; /*   */
constexpr auto	FMC_ECCR2	= 0x74; /*   */
constexpr auto	FMC_ECCR3	= 0x94; /*   */
constexpr auto	FMC_SDCR_1	= 0x140; /*   */
constexpr auto	FMC_SDCR_2	= 0x144; /*   */
constexpr auto	 SDCR_WP	= (1 << 9); /* Write protection */
constexpr auto	 RPIPE_S	= 13;
constexpr auto	 RPIPE_M	= 0x3;
constexpr auto	 RBURST_S	= 12;
constexpr auto	 RPIPE_1	= 1; /* One HCLK clock cycle delay */
constexpr auto	 RPIPE_0	= 0; /* No delay */
constexpr auto	 SDCLK_S	= 10;
constexpr auto	 SDCLK_M	= 0x3;
constexpr auto	 SDCLK_2	= 2; /* 2 x HCLK periods */
constexpr auto	 CAS_S		= 7;
constexpr auto	 CAS_M		= 0x3;
constexpr auto	 CAS_3		= 3;
constexpr auto	 CAS_2		= 2;
constexpr auto	 NB_S		= 6;
constexpr auto	 NB_1		= 0;
constexpr auto	 NB_4		= 1;
constexpr auto	 MWID_S		= 4;
constexpr auto	 MWID_M		= 0x3;
constexpr auto	 MWID_8		= 0;
constexpr auto	 MWID_16	= 1;
constexpr auto	 MWID_32	= 2;
constexpr auto	 NR_S		= 2;
constexpr auto	 NR_M		= 0x3;
constexpr auto	 NR_12		= 1;
constexpr auto	 NR_13		= 2;
constexpr auto	 NC_S		= 0;
constexpr auto	 NC_M		= 0x3;
constexpr auto	 NC_8		= 0;
constexpr auto	 NC_9		= 1;
constexpr auto	 NC_10		= 2;
constexpr auto	FMC_SDTR1	= 0x148; /*   */
constexpr auto	FMC_SDTR2	= 0x14C; /*   */
constexpr auto	 TRCD_S		= 24;
constexpr auto	 TRP_S		= 20;
constexpr auto	 TWR_S		= 16;
constexpr auto	 TRC_S		= 12;
constexpr auto	 TRAS_S		= 8;
constexpr auto	 TXSR_S		= 4;
constexpr auto	 TMRD_S		= 0;
constexpr auto	 FMC_SDCMR			= 0x150; /*   */
constexpr auto	 SDCMR_CTB1			= (1 << 4); /* Command Target Bank 1 */
constexpr auto	 SDCMR_CTB2			= (1 << 3); /* Command Target Bank 2 */
constexpr auto	 SDCMR_MODE_S		=	0;
constexpr auto	 SDCMR_MODE_CLKEN	=	1;
constexpr auto	 SDCMR_MODE_PALL	=	2;
constexpr auto	 SDCMR_MODE_AUTO_REFRESH =	3;
constexpr auto	 SDCMR_MODE_LOAD		= 4;
constexpr auto	 SDCMR_MODE_SELF_REFRESH =	5;
constexpr auto	 SDCMR_MODE_PWR_DOWN	=	6;
constexpr auto	 SDCMR_NRFS_S			= 5;
constexpr auto	 SDCMR_MRD_S			= 9;
constexpr auto	 SDCMR_MRD_BURST_LEN_1		= 0x0;
constexpr auto	 SDCMR_MRD_BURST_LEN_2		= 0x1;
constexpr auto	 SDCMR_MRD_BURST_LEN_4		= 0x2;
constexpr auto	 SDCMR_MRD_BURST_LEN_8		= 0x4;
constexpr auto	 SDCMR_MRD_BURST_TYP_SEQ	= 0x0;
constexpr auto	 SDCMR_MRD_BURST_TYP_INT	= 0x8;
constexpr auto	 SDCMR_MRD_CAS_S		= 4;
constexpr auto	 SDCMR_MRD_CAS_2		= 0x20;
constexpr auto	 SDCMR_MRD_CAS_3		= 0x30;
constexpr auto	 SDCMR_MRD_WBURST_SINGLE	= 0x200;
constexpr auto	FMC_SDRTR			= 0x154; /*   */
constexpr auto	FMC_SDSR			= 0x158; /*   */
constexpr auto	 SDSR_BUSY			= (1 << 5);

}

namespace {
    //! SDRAM ID
    static constexpr auto sdram_id = "sdram";
    //GPIO configuration
    int gpio_conf(bool en)
    {
        const auto mux = dt::get_periph_pin_mux(sdram_id);
        if(mux<0) return mux;
        for(auto it=dt::pinfunc::fmc_sdcke1;it<=dt::pinfunc::fmc_d31;++it) {
            const int pin = dt::get_periph_pin(sdram_id,it);
		    if(pin<0) return pin;
            if(en) {
                /* code */
                gpio::setup(pin, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::high});
            } else {
                gpio::setup(pin, gpio::mode::in{gpio::pulltype::floating});
            }
        }
        return error::success;
    }
    // Write register
    inline __attribute__((always_inline))
    void WR4(uintptr_t reg, uint32_t val) {
         MMIO32(FMC_R_BASE+reg) = val;
    }
    //! Read register
    inline __attribute__((always_inline))
    uint32_t RD4(uintptr_t reg) {
        return MMIO32(FMC_R_BASE+reg);
    }

    void
    fmc_sdram_cmd(uint32_t bank, uint32_t cmd)    
    {
        if (bank == 1)
            cmd |= SDCMR_CTB1;
        else
            cmd |= SDCMR_CTB2;

        while (RD4(FMC_SDSR) & SDSR_BUSY)
            ;

        WR4(FMC_SDCMR, cmd);

        while (RD4(FMC_SDSR) & SDSR_BUSY)
            ;
    }
    static void
    configure_bank(const struct sdram *entry, int b)
    {
        const struct sdram_bank *bank;
	    uint32_t mrd;
    	uint32_t reg;
	    int i;

	    if (b == 1)
		    bank = &entry->bank1;
	    else
		    bank = &entry->bank2;

	    reg =   ( bank->sdcr.rpipe	<< RPIPE_S)	|
		    ( bank->sdcr.rburst	<< RBURST_S)	|
		    ( bank->sdcr.sdclk	<< SDCLK_S)	|
		    ( bank->sdcr.cas	<< CAS_S)	|
		    ( bank->sdcr.nb		<< NB_S )	|
		    ( bank->sdcr.mwid	<< MWID_S)	|
		    ( bank->sdcr.nr		<< NR_S )	|
		    ( bank->sdcr.nc		<< NC_S );

	    if (b == 1)
		    WR4(FMC_SDCR_1, reg);
	    else
		    WR4(FMC_SDCR_2, reg);

	    reg =   ( bank->sdtr.trcd	<< TRCD_S )	|
		    ( bank->sdtr.trp	<< TRP_S )	|
		    ( bank->sdtr.twr	<< TWR_S )	|
		    ( bank->sdtr.trc	<< TRC_S )	|
		    ( bank->sdtr.tras	<< TRAS_S )	|
		    ( bank->sdtr.txsr	<< TXSR_S )	|
		    ( bank->sdtr.tmrd	<< TMRD_S );

	    if (b == 1)
		    WR4(FMC_SDTR1, reg);
	    else
		    WR4(FMC_SDTR2, reg);

	    reg = (SDCMR_MODE_CLKEN << SDCMR_MODE_S);
	    fmc_sdram_cmd(b, reg);

	    for (i = 0; i < 100000; i++)
            __sync_synchronize();

	    reg = (SDCMR_MODE_PALL << SDCMR_MODE_S);
	    fmc_sdram_cmd( b, reg);

	    reg = (SDCMR_MODE_AUTO_REFRESH << SDCMR_MODE_S) |
	        (entry->nrfs << SDCMR_NRFS_S);
	    fmc_sdram_cmd(b, reg);

	    mrd =   ( bank->sdcmr.burstlen ) |
		    ( SDCMR_MRD_BURST_TYP_SEQ ) |
		    ( bank->sdcr.cas << SDCMR_MRD_CAS_S ) |
		    ( SDCMR_MRD_WBURST_SINGLE );

	    reg = (SDCMR_MODE_LOAD << SDCMR_MODE_S) |
	          (mrd << SDCMR_MRD_S);

	    fmc_sdram_cmd(b, reg);
    }

}

// Configure SDRAM
int sdram_setup()
{
    int ret {};
    do {
        //Get periph clock config
        dt::clk_periph pclk;
        ret = dt::get_periph_clock(sdram_id, pclk);
        if(ret) break;
        //Enable device clock
        ret = clock::device_enable(pclk);
        if(ret) break;
        // Configure GPIOS
        ret = gpio_conf(true);
        if(ret) break;
        //Configure controller
        const dt::device_conf_base* base;
        ret = dt::get_periph_devconf(sdram_id,base);
        if(ret) break;
        const auto entry = reinterpret_cast<const sdram*>(base);
    	configure_bank(entry, 1);
	    configure_bank(entry, 2);
	    auto reg = (entry->sdrtr << 1);
	    WR4(FMC_SDRTR, reg);
    } while(0);
    return ret;
}
#else
int sdram_setup()
{
	return error::inval;
}
#endif
}
