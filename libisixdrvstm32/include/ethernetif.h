#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"

//TODO: Add C++ guard and isix porting
err_t ethernetif_init(struct netif *netif);
err_t ethernetif_input(struct netif *netif);
struct netif *ethernetif_register(void);
int ethernetif_poll(void);


void set_mac_address(unsigned char* macadd);


/*
#define MAC_ADDR0 0x00
#define MAC_ADDR1 0x04
#define MAC_ADDR2 0x00
#define MAC_ADDR3 0x08
#define MAC_ADDR4 0x00
#define MAC_ADDR5 0x01
*/
#endif
