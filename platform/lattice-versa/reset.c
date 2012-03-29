
#include <reset.h>
#include <stdio.h>

unsigned char g_resetcode __attribute__((persistent));

void resetCheck( void ) {
	printf("Last system reset caused by one or more of: ");
	
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
	g_resetcode = UNKNOWN;
	
}	

/*****************************************************************************
 * Error handling and debugging, could be removed if space is needed.
 *****************************************************************************/
//void __attribute__((interrupt, auto_psv)) _StackError (void) {
//	g_resetcode = STACK_ERROR;
//}

//void __attribute__((interrupt, auto_psv)) _AddressError (void) {
//	g_resetcode = ADDRS_ERROR;
//} 



