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

#include "net/enc28j60-drv.h"
#include "dev/slip.h"
#include "sicslowmac.h"

//it's important to keep configuration bits that are compatible with the bootloader
//if you change it from the internall/PLL clock, the bootloader won't run correctly

_FOSCSEL(FNOSC_FRCPLL)            //INT OSC with PLL (always keep this setting)
_FOSC(OSCIOFNC_OFF & POSCMD_NONE) //disable external OSC (always keep this setting)
_FWDT(FWDTEN_OFF)                 //watchdog timer off
_FICD(JTAGEN_OFF & ICS_PGD1);         //JTAG debugging off, debugging on PG1 pins enabled

#define SD_TRIS                       TRISAbits.TRISA10
#define SD_IO                         LATAbits.LATA10

uint8_t mac_address[8] = {0x99, 0xaa, 0x99, 0xaa, 0x99, 0xaa, 0x99, 0xaa};

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
	
  leds_init();
  
  clock_init();
  
  enc28j60_init();
  
  process_init();
  process_start(&etimer_process, NULL);
  autostart_start(autostart_processes);
  // process_start(&enc28j60_process, NULL);
  printf("Processes running\n");
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




