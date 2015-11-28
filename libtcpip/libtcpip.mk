#Lwip library version
LWIP_VERSION := 1.4.1

#LWIP base dir
LWIPDIR := $(LIBTCPIP_DIR)/lwip-$(LWIP_VERSION)/src

#Architecture dependend dir
LWIPPORTDIR := $(LIBTCPIP_DIR)/port

#Export TCPIPlib is enabled
ISIX_TCPIPLIB_ENABLED=y

#CFLAGS for hardware checksum
CFLAGS 	 += -DWITH_ISIX_TCPIP_LIB
CXXFLAGS += -DWITH_ISIX_TCPIP_LIB
ifeq ($(ISIX_TCPIPLIB_ETHERNET),y)
CFLAGS 	 += -DISIX_TCPIPLIB_CHECKSUM_BY_HARDWARE -DISIX_TCPIPLIB_ETHERNET
CXXFLAGS += -DISIX_TCPIPLIB_CHECKSUM_BY_HARDWARE -DISIX_TCPIPLIB_ETHERNET
endif


# COREFILES, CORE4FILES: The minimum set of files needed for lwIP.
COREFILES=$(LWIPDIR)/core/mem.c $(LWIPDIR)/core/memp.c $(LWIPDIR)/core/netif.c \
	$(LWIPDIR)/core/pbuf.c $(LWIPDIR)/core/raw.c $(LWIPDIR)/core/stats.c \
	$(LWIPDIR)/core/sys.c $(LWIPDIR)/core/tcp.c $(LWIPDIR)/core/tcp_in.c \
	$(LWIPDIR)/core/tcp_out.c $(LWIPDIR)/core/udp.c $(LWIPDIR)/core/dhcp.c \
	$(LWIPDIR)/core/init.c $(LWIPDIR)/core/timers.c $(LWIPDIR)/core/def.c \
	$(LWIPDIR)/core/dns.c

CORE4FILES=$(wildcard $(LWIPDIR)/core/ipv4/*.c) $(LWIPDIR)/core/ipv4/inet.c \
	$(LWIPDIR)/core/ipv4/inet_chksum.c

# SNMPFILES: Extra SNMPv1 agent
SNMPFILES=$(LWIPDIR)/core/snmp/asn1_dec.c $(LWIPDIR)/core/snmp/asn1_enc.c \
	$(LWIPDIR)/core/snmp/mib2.c $(LWIPDIR)/core/snmp/mib_structs.c \
	$(LWIPDIR)/core/snmp/msg_in.c $(LWIPDIR)/core/snmp/msg_out.c

# APIFILES: The files which implement the sequential and socket APIs.
APIFILES=$(LWIPDIR)/api/api_lib.c $(LWIPDIR)/api/api_msg.c $(LWIPDIR)/api/tcpip.c \
	$(LWIPDIR)/api/err.c $(LWIPDIR)/api/sockets.c $(LWIPDIR)/api/netbuf.c $(LWIPDIR)/api/netdb.c

# NETIFFILES: Files implementing various generic network interface functions.'
NETIFFILES=$(LWIPDIR)/netif/etharp.c $(LWIPDIR)/netif/slipif.c

# PPP interface: Add PPP 
LIBPPP_SRC=$(LWIPDIR)/netif/ppp/auth.c $(LWIPDIR)/netif/ppp/chap.c \
	$(LWIPDIR)/netif/ppp/chpms.c $(LWIPDIR)/netif/ppp/fsm.c \
	$(LWIPDIR)/netif/ppp/ipcp.c $(LWIPDIR)/netif/ppp/lcp.c \
	$(LWIPDIR)/netif/ppp/magic.c $(LWIPDIR)/netif/ppp/md5.c \
	$(LWIPDIR)/netif/ppp/pap.c $(LWIPDIR)/netif/ppp/ppp.c \
	$(LWIPDIR)/netif/ppp/randm.c $(LWIPDIR)/netif/ppp/vj.c
LIBPPP_CPPSRC=$(LWIPPORTDIR)/sio_ppp.cpp

#Arch specific 
LWIPPORTFILES = $(LWIPPORTDIR)/sys_arch.c

#TCPIP source files
LIBTCPIP_SRC = $(COREFILES) $(CORE4FILES) $(APIFILES) $(NETIFFILES) $(LWIPPORTFILES)

#TCPIP include files
LIBTCPIP_INC = -I$(LWIPDIR)/include -I$(LWIPDIR)/include/ipv4 -I$(LWIPPORTDIR)/include


LIBTCPIP_LIB = $(LIBTCPIP_DIR)/libtcpip.a
LIBIFPPP_LIB = $(LIBTCPIP_DIR)/libifppp.a

LIBIFPPP_OBJS += $(LIBPPP_SRC:%.c=%.o) $(LIBPPP_CPPSRC:%.cpp=%.o)
LIBTCPIP_OBJS += $(LIBTCPIP_SRC:%.c=%.o) $(LIBTCPIP_CPPSRC:%.cpp=%.o)
DEPFILES += $(LIBTCPIP_SRC:%.c=%.dep) $(LIBTCPIP_CPPSRC:%.cpp=%.dep) 
DEPFILES += $(LIBPPP_SRC:%.c=%.dep) $(LIBPPP_CPPSRC:%.cpp=%.dep) 

.ONESHELL:
$(LIBTCPIP_LIB): $(LIBTCPIP_OBJS)
	    $(AR) $(ARFLAGS) $@ $^
	    
.ONESHELL:
$(LIBIFPPP_LIB): $(LIBIFPPP_OBJS)
	    $(AR) $(ARFLAGS) $@ $^
	    

LIBS += $(LIBIFPPP_LIB) $(LIBTCPIP_LIB)
LIBS_OBJS += $(LIBIFPPP_OBJS) $(LIBTCPIP_OBJS) 
COMMON_FLAGS += $(LIBTCPIP_INC)
