#ifndef __CONTIKI_CONF_H__DPWEBPLATFORM__
#define __CONTIKI_CONF_H__DPWEBPLATFORM__

#include <stdint.h>

#define CCIF
#define CLIF

#define WITH_UIP 1
#define WITH_ASCII 1
#define WITH_PHASE_OPTIMIZATION 0

#define PROCESS_CONF_NO_PROCESS_NAMES 0
 
#define F_CPU 8000000
#define RTIMER_ARCH_PRESCALER 256

#define CLOCK_CONF_SECOND 100
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;

typedef unsigned int clock_time_t;
typedef unsigned int uip_stats_t;

/* uIP configuration */
#define UIP_CONF_LLH_LEN 0
#define UIP_CONF_BROADCAST 1
#define UIP_CONF_LOGGING 1
#define UIP_CONF_DHCP_LIGHT 1
#define UIP_CONF_BUFFER_SIZE     116
#define UIP_CONF_RECEIVE_WINDOW  (UIP_CONF_BUFFER_SIZE - 40)
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       8
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1
//#define UIP_ARCH_IPCHKSUM        1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_TCP_FORWARD 0

/* Prefix for relocation sections in ELF files */

#define RAND_MAX 0x7fff
#endif /* __CONTIKI_CONF_H__DPWEBPLATFORM__ */
