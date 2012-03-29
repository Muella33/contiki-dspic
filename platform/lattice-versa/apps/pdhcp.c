/*
 * This process hosts the dhcpc client. When dhcp completes it configured the DNS resolver and
 * tries to locate an NTP server. Wen DNS callback occurs, we attampt to set the system clock 
 * using NTP result. We then sleep until the DHCP expires.
 */

#include "contiki.h"
#include "contiki-net.h"
#include "net/dhcpc.h"
#include "net/resolv.h"

#include "ntpclient.h"

#include <stdint.h>
#include <stdio.h>

PROCESS(dhcp_process, "DHCP process");
static struct etimer timer;
extern uint8_t eth_mac_addr[];

char* geoip = "http://freegeoip.net/csv/";


PROCESS_THREAD(dhcp_process, ev, data)
{
  uip_ipaddr_t * addr;
  
  PROCESS_BEGIN();

   /*
     * Don't start DHCP until a second passes to allow the interface to come up.
     */
	etimer_set(&timer, 2*CLOCK_SECOND); 
	printf("pdhcp: init\n");
	dhcpc_init(eth_mac_addr, 6);
	while(1) {
  
		PROCESS_WAIT_EVENT();
		
		if(ev == PROCESS_EVENT_MSG) {
			resolv_query("pool.ntp.org");
			printf("pdhcp: PROCESS_EVENT_MSG\n");
		} else if(ev == tcpip_event) {
		   printf("pdhcp: TCPIP_EVENT\n");
		   dhcpc_appcall(ev, data);
		} else if(ev == PROCESS_EVENT_TIMER) {
			printf("pdhcp: PROCESS_EVENT_TIMER\n");
			etimer_stop(&timer);
			dhcpc_request();
					
		} else if (ev == resolv_event_found) {
			printf("pdhcp: resolve event callback: %s\n", (char*)data );
			addr = resolv_lookup(data);
			if (addr != NULL) {
				printf("pdhcp: resolv callback for ntp IP %d.%d.%d.%d\n",
				addr->u8[0],
				addr->u8[1],
				addr->u8[2],
				addr->u8[3]);
				ntpclient_conf(addr);
			}		
			
		}
    
  }

  PROCESS_END();
}

void dhcpc_configured(const struct dhcpc_state *s)
{
  uip_sethostaddr(&s->ipaddr);
  uip_setnetmask(&s->netmask);
  uip_setdraddr(&s->default_router);
  resolv_conf(&s->dnsaddr);

  printf("DHCP Configured\n");
  process_post(PROCESS_CURRENT(), PROCESS_EVENT_MSG, NULL);
  
}

void dhcpc_unconfigured(const struct dhcpc_state *s)
{
  static uip_ipaddr_t nulladdr;

  uip_sethostaddr(&nulladdr);
  uip_setnetmask(&nulladdr);
  uip_setdraddr(&nulladdr);
  resolv_conf(&nulladdr);

  printf("DHCP Unconfigured\n");
  process_post(PROCESS_CURRENT(), PROCESS_EVENT_MSG, NULL);
}
