//
#ifndef LIBISIXSTM32DRV_ETHERNETIF_H_
#define LIBISIXSTM32DRV_ETHERNETIF_H_

//
#include "lwip/err.h"
#include "lwip/netif.h"

//
#ifdef __cplusplus
extern "C" {
#endif

//! Forward decl
struct phy_device;


//
/* Initialize ethernet if interface */
err_t stm32_emac_if_init_callback(struct netif *netif);


//
/** Create and setup ethernet if interface
 * @param [in] hw_addr Hardware ethernet addresss
 */
struct netif* stm32_emac_netif_create( const uint8_t *hw_addr );

//
/* Destroy the netif stm32 interface
 * @param[in] netif interface shudown
 * @note allocated netif struct must be deleted manualy,
 * netif interface must be deassert using LWIP netif API */
int stm32_emac_netif_shutdown( struct netif *netif);



//
#ifdef __cplusplus
}
#endif

//
#endif

//
