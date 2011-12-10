/*
 * This file contains an example of how a Contiki program looks.
 *
 * The program opens a UDP broadcast connection and sends one packet
 * every second.
 */

#include "contiki.h"
#include "contiki-net.h"
#include "net/dhcpc.h"

#include <stdint.h>
#include <stdio.h>

/*
 * This process hosts the dhcpc client
 */
PROCESS(dhcp_process, "DHCP process");
static struct etimer timer;
extern uint8_t eth_mac_addr[];

PROCESS_THREAD(dhcp_process, ev, data)
{
  
  PROCESS_BEGIN();

   /*
     * We set a timer that wakes us up after 5 seconds. 
     */
	etimer_set(&timer, 5*CLOCK_SECOND); 
  
  while(1) {
  
    PROCESS_WAIT_EVENT();
    
    if(ev == PROCESS_EVENT_MSG) {
		printf("event: PROCESS_EVENT_MSG\n");
    } else if(ev == tcpip_event) {
	   printf("event: TCPIP_EVENT\n");
       dhcpc_appcall(ev, data);
    } else if(ev == PROCESS_EVENT_TIMER) {
	    printf("dhcp init\n");
	    dhcpc_init(eth_mac_addr, 6);

      	printf("event: dhcp request\n");
		dhcpc_request();
	}
    
  }

  PROCESS_END();
}

