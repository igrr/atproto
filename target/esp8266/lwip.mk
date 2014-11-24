LWIP_DIR := $(TARGET_DIR)/../../lwip/src

LWIP_OBJ_FILES:= api/api_lib.o \
		api/api_msg.o \
		api/err.o \
		api/netbuf.o \
		api/netdb.o \
		api/netifapi.o \
		api/pppapi.o \
		api/sockets.o \
		api/tcpip.o \
		core/def.o \
		core/dhcp.o \
		core/dns.o \
		core/inet_chksum.o \
		core/init.o \
		core/mem.o \
		core/memp.o \
		core/netif.o \
		core/pbuf.o \
		core/raw.o \
		core/stats.o \
		core/sys.o \
		core/tcp.o \
		core/tcp_in.o \
		core/tcp_out.o \
		core/timers.o \
		core/udp.o \
		core/ipv4/autoip.o \
		core/ipv4/icmp.o \
		core/ipv4/igmp.o \
		core/ipv4/ip4.o \
		core/ipv4/ip4_addr.o \
		core/ipv4/ip_frag.o \
		netif/etharp.o \
		netif/ethernetif.o \
		netif/slipif.o \
		espconn/dhcpserver.o \
		espconn/espconn.o \
		espconn/espconn_tcp.o \
		espconn/espconn_udp.o \
		espconn/netio.o \
		espconn/ping.o 

LWIP_OBJ_PATHS := $(addprefix $(LWIP_DIR)/,$(LWIP_OBJ_FILES))
