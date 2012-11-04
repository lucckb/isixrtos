/* ------------------------------------------------------------------ */
#ifndef LIBISIXSTM32DRV_ETHERNETIF_H_
#define LIBISIXSTM32DRV_ETHERNETIF_H_

/* ------------------------------------------------------------------ */
#include "lwip/err.h"
#include "lwip/netif.h"

/* ------------------------------------------------------------------ */
#ifdef __cplusplus
extern C {
#endif

/* ------------------------------------------------------------------ */
/* Initialize ethernet if interface */
err_t stm32_emac_if_init_callback(struct netif *netif);


/* ------------------------------------------------------------------ */
/** Create and setup ethernet if interface
 * @param [in] hw_addr Hardware ethernet addresss
 * @param [in] phy_addr Device phy address
 * @param [in] hclk HCLK core frequency
 * @param [in] is_rmii Use reduced MII interface
 * @param [in] configure_mco Configure MCO pin for provide MCO output
 */
struct netif* stm32_emac_if_setup( const uint8_t *hw_addr, uint16_t phy_addr, uint32_t hclk,
        bool is_rmii, bool configure_mco);

/* ------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif

/* ------------------------------------------------------------------ */
#endif

/* ------------------------------------------------------------------ */
