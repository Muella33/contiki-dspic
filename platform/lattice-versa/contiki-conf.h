#ifndef __CONTIKI_CONF_H__DPWEBPLATFORM__
#define __CONTIKI_CONF_H__DPWEBPLATFORM__

#include <stdint.h>

#define CCIF
#define CLIF

#define TELNETD_CONF_NUMLINES 10
#define WITH_DNS 1
#define WITH_UIP 1
#define UIP_UDP 1
#define UIP_TCP 1

#define WITH_ASCII 1
#define WITH_PHASE_OPTIMIZATION 0

#define PROCESS_CONF_NO_PROCESS_NAMES 0
 
#define F_CPU 8000000
#define RTIMER_ARCH_PRESCALER 256
#define CLOCK_CONF_SECOND 100

// protocol debug for the 1wire bus
#define AN192_OW_DEBUG 0

typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;

typedef unsigned int clock_time_t;
typedef unsigned int uip_stats_t;

// webserver log requests to serial port
#define LOG_CONF_ENABLED 1

/* uIP configuration */
#define UIP_CONF_LLH_LEN 14
#define UIP_CONF_BROADCAST 1
#define UIP_CONF_LOGGING 1

// this breaks DHCP for me   #define UIP_CONF_DHCP_LIGHT 1
#define UIP_CONF_BUFFER_SIZE     1500

// my cellphone advertises a large TCP MSS of 1460 which causes uIP to emit 
// frames of 1500 bytes (including Ethernet 2 header) which are dropped
// by either the ENC28J60 chip OR my switch. Not sure which, but using 
// this config to ensure the TCP MSS is negotiated down to something that
// gets off the board OK. 
// There are some "send large frame" config bits in the ethernet chip - to investigate
// if this increments a counter when triggered
#define UIP_CONF_TCP_MSS     1430

// we don't bother with the packet queue, zero the size
#define QUEUEBUF_CONF_NUM        0
#define PACKETBUF_CONF_SIZE      0
#define PACKETBUF_CONF_HDR_SIZE  0

// don't forward serial-line events to processess, we'll 
// poll the hardware directly
#define SERIAL_LINE_CONF_BUFSIZE 0

#define UIP_CONF_MAX_CONNECTIONS 8
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       8
// we don't forward any packets, so zero this
#define UIP_CONF_FWCACHE_SIZE    0
#define UIP_CONF_BROADCAST       1
//#define UIP_ARCH_IPCHKSUM      1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_PINGNOADDRESSDROP    1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_TCP_FORWARD     0
#define UIP_CONF_IPV6_QUEUE_PKT  0


/* Prefix for relocation sections in ELF files */

#endif /* __CONTIKI_CONF_H__DPWEBPLATFORM__ */
