#include <stdint.h>
#include <stdio.h>
#include <sys/process.h>
#include <dev/leds.h>
#include <sys/procinit.h>
#include <etimer.h>
#include <sys/autostart.h>
#include <clock.h>
#include <serial.h>
#include <reset.h>
#include <prandom.h>
#include <dev/eeprom.h>
#include <string.h>
#include <rand31-park-miller-carta-int.h>

#include "dev/slip.h"
#include "sicslowmac.h"

// #include "contiki.h"
#include "contiki-net.h"
#include <net/dhcpc.h>
#include "apps/pdebug.h"
#include "apps/pdhcp.h"
#include "apps/ntpclient.h"

uint8_t eth_mac_addr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const char* version = "CONTIKI_VERSION_STRING";

unsigned int idle_count = 0;

#define EEPROM_CONFIG_BASE 0x0000
void readMAC(void);

int main()
{

  dbg_setup_uart();
  printf("Versa Contiki Platform - %s started.\n", version);
  resetCheck();
  randomSeed();
  
  rtimer_init();  
  printf("main: leds init\n");  
  leds_init();
  printf("main: process init\n");
  process_init();
  printf("main: etimer init\n");
  process_start(&etimer_process, NULL);
  printf("main: clock init\n");
  clock_init();

  // Note, readMAC() uses eeprom routines, which require clock_init() to setup clock hardware
  readMAC();  
  // printf("main: eth start\n");
  // process_start(&enc28j60_process, NULL);
   
  printf("main: tcpip start\n");
  process_start(&tcpip_process, NULL);		// invokes uip_init();
  
  process_start(&announce_process, NULL);
  process_start(&dhcp_process, NULL);
  process_start(&resolv_process, NULL);
  process_start(&ntp_process, NULL);
  
  printf("main: autostart\n");
  autostart_start(autostart_processes);
  
  printf("main: all processes running\n");
  
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
// required for #define LOG_CONF_ENABLED
void log_message(const char *part1, const char *part2) {
  printf("log: %s: %s\n", part1, part2);
}

void readMAC(void) {
	uint8_t eeprom_rd_buffer[32];
	
	// If the eeprom (first sector) matches our version string, use the configuration
	// found in the eeprom, otherwise generate a random MAC and persist it for this board.
	
	printf("main: loading from eeprom read sz=%d address=%04x\n", sizeof(eeprom_rd_buffer), EEPROM_CONFIG_BASE);
	eeprom_read(EEPROM_CONFIG_BASE, (unsigned char *)&eeprom_rd_buffer, sizeof(eeprom_rd_buffer));
	
	if (strcmp(version, (char*)&eeprom_rd_buffer ) == 0) {
		printf("main: using stored configuration\n");	
	} else {	
		printf("main: setting new MAC with DHCP\n");
		memset((void *)&eeprom_rd_buffer, 0, sizeof(eeprom_rd_buffer));
		memcpy((unsigned char *)&eeprom_rd_buffer[0], version, strlen(version)+1);
		
		// use microchip OUI database range, since it's a microchip part
		eeprom_rd_buffer[24] = 0x00;
		eeprom_rd_buffer[25] = 0x04;
		eeprom_rd_buffer[26] = 0xA3;		
		eeprom_rd_buffer[27] = (unsigned char) rand31pmc_ranlui() & 0xFF;
		eeprom_rd_buffer[29] = (unsigned char) rand31pmc_ranlui() & 0xFF;
		eeprom_rd_buffer[30] = (unsigned char) rand31pmc_ranlui() & 0xFF;
		
		printf("main: writing to eeprom\n");
		eeprom_write(EEPROM_CONFIG_BASE, (unsigned char *)&eeprom_rd_buffer, sizeof(eeprom_rd_buffer));
				
	}
	
	memcpy((unsigned char *)&eth_mac_addr, (char*)&eeprom_rd_buffer[24], sizeof(eth_mac_addr));
}
