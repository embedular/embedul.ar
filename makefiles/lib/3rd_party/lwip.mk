#
# embedul.ar™ embedded systems framework - http://embedul.ar
#
# lwip library.
#
# Copyright 2018-2022 Santiago Germino
# <sgermino@embedul.ar> https://www.linkedin.com/in/royconejo
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#

$(call emb_need_var,LIB_EMBEDULAR_ROOT)

# ------------------------------------------------------------------------------
# Library config options (LIB_LWIP_CONFIG_*)
# ------------------------------------------------------------------------------
# No options.
# ------------------------------------------------------------------------------

$(call emb_declare_lib,$\
	LwIP,$\
	LIB_LWIP,$\
	$(LIB_EMBEDULAR_ROOT)/source/3rd_party/lwip-2.1.3,$\
	)


# Source compilation actually controlled by defines inside lwipopts.h :(
OBJS += $(LIB_LWIP)/src/netif/ppp/auth.o \
        $(LIB_LWIP)/src/netif/ppp/ccp.o \
        $(LIB_LWIP)/src/netif/ppp/chap_ms.o \
        $(LIB_LWIP)/src/netif/ppp/chap-md5.o \
        $(LIB_LWIP)/src/netif/ppp/chap-new.o \
        $(LIB_LWIP)/src/netif/ppp/demand.o \
        $(LIB_LWIP)/src/netif/ppp/eap.o \
        $(LIB_LWIP)/src/netif/ppp/ecp.o \
        $(LIB_LWIP)/src/netif/ppp/eui64.o \
        $(LIB_LWIP)/src/netif/ppp/fsm.o \
        $(LIB_LWIP)/src/netif/ppp/ipcp.o \
        $(LIB_LWIP)/src/netif/ppp/ipv6cp.o \
        $(LIB_LWIP)/src/netif/ppp/lcp.o \
        $(LIB_LWIP)/src/netif/ppp/magic.o \
        $(LIB_LWIP)/src/netif/ppp/mppe.o \
        $(LIB_LWIP)/src/netif/ppp/multilink.o \
        $(LIB_LWIP)/src/netif/ppp/ppp.o \
        $(LIB_LWIP)/src/netif/ppp/pppapi.o \
        $(LIB_LWIP)/src/netif/ppp/pppcrypt.o \
        $(LIB_LWIP)/src/netif/ppp/pppoe.o \
        $(LIB_LWIP)/src/netif/ppp/pppol2tp.o \
        $(LIB_LWIP)/src/netif/ppp/pppos.o \
        $(LIB_LWIP)/src/netif/ppp/upap.o \
        $(LIB_LWIP)/src/netif/ppp/utils.o \
        $(LIB_LWIP)/src/netif/ppp/vj.o \
        $(LIB_LWIP)/src/netif/ethernet.o \
        $(LIB_LWIP)/src/netif/slipif.o \
        $(LIB_LWIP)/src/netif/lowpan6.o \
        $(LIB_LWIP)/src/api/err.o \
        $(LIB_LWIP)/src/api/netdb.o \
        $(LIB_LWIP)/src/api/netifapi.o \
        $(LIB_LWIP)/src/api/api_lib.o \
        $(LIB_LWIP)/src/api/netbuf.o \
        $(LIB_LWIP)/src/api/tcpip.o \
        $(LIB_LWIP)/src/api/sockets.o \
        $(LIB_LWIP)/src/api/api_msg.o \
        $(LIB_LWIP)/src/core/tcp.o \
        $(LIB_LWIP)/src/core/memp.o \
        $(LIB_LWIP)/src/core/sys.o \
        $(LIB_LWIP)/src/core/udp.o \
        $(LIB_LWIP)/src/core/pbuf.o \
        $(LIB_LWIP)/src/core/inet_chksum.o \
        $(LIB_LWIP)/src/core/tcp_in.o \
        $(LIB_LWIP)/src/core/ip.o \
        $(LIB_LWIP)/src/core/tcp_out.o \
        $(LIB_LWIP)/src/core/mem.o \
        $(LIB_LWIP)/src/core/dns.o \
        $(LIB_LWIP)/src/core/timeouts.o \
        $(LIB_LWIP)/src/core/stats.o \
        $(LIB_LWIP)/src/core/raw.o \
        $(LIB_LWIP)/src/core/init.o \
        $(LIB_LWIP)/src/core/netif.o \
        $(LIB_LWIP)/src/core/def.o \
        $(LIB_LWIP)/src/core/ipv4/etharp.o \
        $(LIB_LWIP)/src/core/ipv4/ip4.o \
        $(LIB_LWIP)/src/core/ipv4/igmp.o \
        $(LIB_LWIP)/src/core/ipv4/icmp.o \
        $(LIB_LWIP)/src/core/ipv4/dhcp.o \
        $(LIB_LWIP)/src/core/ipv4/autoip.o \
        $(LIB_LWIP)/src/core/ipv4/ip4_addr.o \
        $(LIB_LWIP)/src/core/ipv4/ip4_frag.o \
        $(LIB_LWIP)/src/core/ipv6/ip6.o \
        $(LIB_LWIP)/src/core/ipv6/dhcp6.o \
        $(LIB_LWIP)/src/core/ipv6/inet6.o \
        $(LIB_LWIP)/src/core/ipv6/nd6.o \
        $(LIB_LWIP)/src/core/ipv6/ip6_frag.o \
        $(LIB_LWIP)/src/core/ipv6/ip6_addr.o \
        $(LIB_LWIP)/src/core/ipv6/mld6.o \
        $(LIB_LWIP)/src/core/ipv6/icmp6.o \
        $(LIB_LWIP)/src/core/ipv6/ethip6.o \
        $(LIB_LWIP)/system/OS/sys_arch.o

CFLAGS += -I$(LIB_LWIP)/src/include \
          -I$(LIB_LWIP)/system \
          -I$(LIB_LWIP)/src/include/netif/ppp \
          -I$(LIB_LWIP)/src/include/lwip \
          -I$(LIB_LWIP)/src/include/lwip/apps \
          -I$(LIB_LWIP)/src/include/lwip/priv \
          -I$(LIB_LWIP)/src/include/lwip/prot \
          -I$(LIB_LWIP)/src/include/netif \
          -I$(LIB_LWIP)/src/include/posix \
          -I$(LIB_LWIP)/src/include/posix/sys \
          -I$(LIB_LWIP)/system/arch
