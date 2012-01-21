/*
 * This file contains an example of how a Contiki program looks.
 *
 * The program opens a UDP broadcast connection and sends one packet
 * every second.
 */

#include "contiki.h"
#include "contiki-net.h"
#include "pdebug.h"
#include <stdint.h>
#include <stdio.h>

/*
 * All Contiki programs must have a process, and we declare it here.
 */
PROCESS(announce_process, "UDP Announcer");

/*
 * To make the program send a packet once every second, we use an
 * event timer (etimer).
 */
static struct etimer timer;
extern uint8_t eth_mac_addr[];
extern const char* version;

/*---------------------------------------------------------------------------*/
/*
 * Here we implement the process. The process is run whenever an event
 * occurs, and the parameters "ev" and "data" will we set to the event
 * type and any data that may be passed along with the event.
 */
PROCESS_THREAD(announce_process, ev, data)
{
  static struct uip_udp_conn *c;
  unsigned short sz = 0;
  uip_ipaddr_t addr;
  
  PROCESS_BEGIN();
  c = udp_broadcast_new(UIP_HTONS(4321), NULL);
  
   while(1) {

    
    // wake up periodically every second. 
    
    etimer_set(&timer, 4*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    /*
     * To send a UDP packet request the uIP TCP/IP stack process call
     * us back with the buffer
     */
    tcpip_poll_udp(c);
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

	uip_gethostaddr(&addr);
	// str = makebyte(addr->u8[0], str);
	
    // send our packet.
	sz = snprintf((char *)uip_appdata, uip_mss() - sz,
         "HELLO %s ETH %02x:%02x:%02x:%02x:%02x:%02x IP %d.%d.%d.%d RTC ",
         version,
         eth_mac_addr[0], eth_mac_addr[1], eth_mac_addr[2], 
         eth_mac_addr[3], eth_mac_addr[4], eth_mac_addr[5], 
		addr.u8[0],
		addr.u8[1],
		addr.u8[2],
		addr.u8[3]);
	sz += printRTCTime((char*) uip_appdata+sz );
	sz += snprintf((char *)uip_appdata+sz, uip_mss() - sz, "\n" );
	
    uip_send(uip_appdata, sz);

  }

  PROCESS_END();
}

