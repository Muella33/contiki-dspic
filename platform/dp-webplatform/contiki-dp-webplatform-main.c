#include <stdint.h>
#include <stdio.h>
#include <sys/process.h>
#include <dev/leds.h>
#include <sys/procinit.h>
#include <etimer.h>
#include <sys/autostart.h>
#include <clock.h>
#include <p33Fxxxx.h>

//it's important to keep configuration bits that are compatible with the bootloader
//if you change it from the internall/PLL clock, the bootloader won't run correctly

_FOSCSEL(FNOSC_FRCPLL)            //INT OSC with PLL (always keep this setting)
_FOSC(OSCIOFNC_OFF & POSCMD_NONE) //disable external OSC (always keep this setting)
_FWDT(FWDTEN_OFF)                 //watchdog timer off
_FICD(JTAGEN_OFF & ICS_PGD1);         //JTAG debugging off, debugging on PG1 pins enabled

#define SD_TRIS                       TRISAbits.TRISA10
#define SD_IO                         LATAbits.LATA10


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

	//UART1 SETUP
	//UART can be placed on any RPx pin
	//configure for RP14/RP15 to use the FTDI usb->serial converter
	//assign pin 14 to the UART1 RX input register
	//RX PR14 (input)
	RPINR18bits.U1RXR = 14;
	//assign UART1 TX function to the pin 15 output register
	//TX RP15 (output)
	RPOR7bits.RP15R = 3; // U1TX_O

	//InitializeUART1();	
	//setup UART
    U1BRG = 85;//86@80mhz, 85@79.xxx=115200
    U1MODE = 0; //clear mode register
    U1MODEbits.BRGH = 1; //use high percison baud generator
    U1STA = 0;	//clear status register
    U1MODEbits.UARTEN = 1; //enable the UART RX
    IFS0bits.U1RXIF = 0;  //clear the receive flag
    U1STAbits.UTXEN = 1;  //enable UART TX

	// with PIC30 compiler standard library the default printf calls write, which
	// is soft-linked to a write() function outputing to UART1

//  dbg_setup_uart();
  printf("Initialising\n");
	
    leds_init();
  
  clock_init();
  process_init();
  process_start(&etimer_process, NULL);
  autostart_start(autostart_processes);
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




