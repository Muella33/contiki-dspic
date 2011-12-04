
#include <reset.h>
#include <p33Fxxxx.h>
#include <stdio.h>

unsigned char g_resetcode __attribute__((persistent));

void resetCheck( void ) {
	printf("Last system reset caused by one or more of: ");
	if (RCON & 0x8082) printf("\r\n");
	if (RCONbits.POR) printf(" Power On reset,");
	if (RCONbits.BOR) printf(" Brown Out reset,");
	if (RCONbits.EXTR) printf(" Reset Button press,");
	if (RCONbits.TRAPR) printf(" Trap Conflict reset,");
	if (RCONbits.IOPUWR) printf("\r\n Illegal Opcode or Uninitialized indirect memory access reset,");
	if (RCONbits.SWR){
		printf("\r\n Software initiated reset due to ");
		switch(g_resetcode){
			case STACK_ERROR:
				printf("a Stack Error.");
				break;
			case ADDRS_ERROR:
				printf("an Address Error.");
				break;
			case RTOS_STACK:
				printf("an RTOS Stack overflow.");
				break;
			case RTOS_MALLOC:
				printf("an RTOS out of memory error.");
				break;
			default:
				printf("an unknown reason.");
				break;
		}
		printf("\r\nSystem halted.");
		g_resetcode = UNKNOWN;
		// SetErrorLED();
		while(1);
	}
	printf("\x08.\r\n");
	RCON = 0;
	g_resetcode = UNKNOWN;
	
}	

/*****************************************************************************
 * Error handling and debugging, could be removed if space is needed.
 *****************************************************************************/
void __attribute__((interrupt, auto_psv)) _StackError (void) {
	g_resetcode = STACK_ERROR;
	asm("RESET");
}

void __attribute__((interrupt, auto_psv)) _AddressError (void) {
	g_resetcode = ADDRS_ERROR;
	asm("RESET");
} 



