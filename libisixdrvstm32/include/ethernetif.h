#ifndef LIBISIXSTM32DRV_ETHERNETIF_H_
#define LIBISIXSTM32DRV_ETHERNETIF_H_


#include "lwip/err.h"
#include "lwip/netif.h"

//TODO: Add C++ guard and isix porting
err_t ethernetif_init(struct netif *netif);
err_t ethernetif_input(struct netif *netif);
struct netif *ethernetif_register(void);
void set_mac_address(unsigned char* macadd);



#endif
