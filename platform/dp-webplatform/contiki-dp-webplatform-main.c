#include <stdint.h>
#include <stdio.h>
#include <sys/process.h>
#include <dev/leds.h>
#include <sys/procinit.h>
#include <etimer.h>
#include <sys/autostart.h>
#include <clock.h>
#include <p33Fxxxx.h>
#include <serial.h>
#include <reset.h>

#include "net/enc28j60-drv.h"
#include "dev/slip.h"
#include "sicslowmac.h"

// #include "contiki.h"
#include "contiki-net.h"
#include <net/dhcpc.h>
#include "apps/pdebug.h"

//it's important to keep configuration bits that are compatible with the bootloader
//if you change it from the internall/PLL clock, the bootloader won't run correctly

_FOSCSEL(FNOSC_FRCPLL)            //INT OSC with PLL (always keep this setting)
_FOSC(OSCIOFNC_OFF & POSCMD_NONE) //disable external OSC (always keep this setting)
_FWDT(FWDTEN_OFF)                 //watchdog timer off
_FICD(JTAGEN_OFF & ICS_PGD1);         //JTAG debugging off, debugging on PG1 pins enabled

#define SD_TRIS                       TRISAbits.TRISA10
#define SD_IO                         LATAbits.LATA10

uint8_t eth_mac_addr[6] = {0x02, 0xaa, 0xbb, 0xcc, 0xdd, 0xee};

unsigned int idle_count = 0;

int main()
{

	AD1PCFGL = 0xFFFF; //digital pins

	//setup internal clock for 80MHz/40MIPS
	//7.37/2=3.685*43=158.455/2=79.2275
	CLKDIVbits.PLLPRE=0; // PLLPRE (N2) 0=/2 
	PLLFBD=41; //pll multiplier (M) = +2
	CLKDIVbits.PLLPOST=0;// PLLPOST (N1) 0=/2
    while(!OSCCONbits.LOCK);//wait for PLL ready

  dbg_setup_uart();
  printf("Initialising\n");

  resetCheck();
  rtimer_init();
  
  printf("leds init\n");
  leds_init();
  printf("clock init\n");
  clock_init();
  printf("process init\n");
  process_init();
  printf("etimer init\n");
  process_start(&etimer_process, NULL);
  
  uip_init();
  printf("eth init\n");
  enc28j60_init(eth_mac_addr);
  
  // uip_setethaddr( eaddr );
  
  printf("eth start\n");
  process_start(&enc28j60_process, NULL);
  printf("autostart init\n");
  autostart_start(autostart_processes);
  printf("tcpip start\n");
  process_start(&tcpip_process, NULL);
  
  printf("dhcp init\n");
  dhcpc_init(eth_mac_addr, 6);
  
  printf("Processes running\n");

  process_start(&example_program_process, NULL);

  
  while(1) {
    do {
    } while(process_run() > 0);
    idle_count++;
	
    /* Idle! */
  }
  return 0;
}

void uip_log(char *m)
{
  printf("uIP: '%s'\n", m);
}
void
dhcpc_configured(const struct dhcpc_state *s)
{
  uip_sethostaddr(&s->ipaddr);
  uip_setnetmask(&s->netmask);
  uip_setdraddr(&s->default_router);
#if WITH_DNS
  resolv_conf(&s->dnsaddr);
#endif /* WITH_DNS */

  printf("DHCP Configured.");
  process_post(PROCESS_CURRENT(), PROCESS_EVENT_MSG, NULL);
}
/*-----------------------------------------------------------------------------------*/
void
dhcpc_unconfigured(const struct dhcpc_state *s)
{
  static uip_ipaddr_t nulladdr;

  uip_sethostaddr(&nulladdr);
  uip_setnetmask(&nulladdr);
  uip_setdraddr(&nulladdr);
#if WITH_DNS
  resolv_conf(&nulladdr);
#endif /* WITH_DNS */

  printf("DHCP Unconfigured.");
  process_post(PROCESS_CURRENT(), PROCESS_EVENT_MSG, NULL);
}



