/*-----------------------------------------------------------------------------------*/
/*
 * Copyright (c) 2001-2004, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * This is a modified contiki driver example.
 * Author: Maciej Wasilak (wasilak@gmail.com)
 *
 * $Id: enc28j60-drv.c,v 1.2 2007/05/26 23:05:36 oliverschmidt Exp $
 *
 */

#include "contiki-net.h"
#include "enc28j60.h"
#include "enc28j60-drv.h"
#include <stdio.h>

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
#define IPBUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

PROCESS(enc28j60_process, "ENC28J60 driver");

uint8_t eth_mac_addr[6] = {0x02, 0xaa, 0xbb, 0xcc, 0xdd, 0xee};
volatile uint8_t uipDMAactive = 0;
#define cli()
#define sei()
#define PRINTF printf

u8_t enc28j60_output( void )
{
	PRINTF("ENC28 send: %d bytes\n",uip_len);
	
	if(uip_len <= UIP_LLH_LEN + UIP_TCPIP_HLEN) {
		// hwsend(&uip_buf[0], UIP_LLH_LEN);
		// hwsend(&uip_buf[UIP_LLH_LEN], uip_len - UIP_LLH_LEN);		
		enc28j60PacketSend1(uip_buf, uip_len );
	} else {
		//hwsend(&uip_buf[0], UIP_LLH_LEN);
		//hwsend(&uip_buf[UIP_LLH_LEN], UIP_TCPIP_HLEN);
		//hwsend(uip_appdata, uip_len - UIP_TCPIP_HLEN - UIP_LLH_LEN);
		enc28j60PacketSend2(uip_buf, UIP_LLH_LEN + UIP_TCPIP_HLEN, uip_appdata, uip_len - UIP_TCPIP_HLEN - UIP_LLH_LEN);
	}
 		
	return 0;
}

/*
 * Placeholder - switching off enc28 chip wasn't yet considered
 */
void enc28j60_exit(void)
{}

/*
 * Wrapper for lowlevel enc28j60 init code
 * in current configuration it reads the Ethernet driver MAC address
 * from EEPROM memory
 */
void enc28j60_init()
{
	enc28j60Init(eth_mac_addr);
}

static void pollhandler(void) {

#if ENC28J60DMA
	uip_len = enc28j60BeginPacketReceiveDMA(uip_buf, UIP_BUFSIZE);

	uipDMAactive = 1;
	/* Whilst we use DMA to transfer the buffer contents for speed - we must block waiting.
	 * If we returned from the protothread (process) into the Contiki schedular we could not hold
	 * exclusive use of uip_buf. An expiring timer could generate simultaneous
	 * packet data for transmission and corrupt the incoming data.
	 */
	 while (uipDMAActive) {
		// block ;
	 }
	 end28j60EndPacketRecieveDMA();
#else
	uip_len = enc28j60PacketReceive();
#endif
  	PRINTF("ENC28 recv: got %d bytes\n", uip_len);

  
	if(uip_len > 0) {
		PRINTF("POLLHANDLER: ENC28 receive: %d bytes %d\n",uip_len, uip_htons(BUF->type));
	    if(BUF->type == UIP_HTONS(UIP_ETHTYPE_IP)) {
			uip_arp_ipin();
			uip_input();
			if(uip_len > 0) {
				uip_arp_out();
				enc28j60_output();
			}
		} else if(BUF->type == UIP_HTONS(UIP_ETHTYPE_ARP)) {
			uip_arp_arpin();
			if(uip_len > 0) {
				enc28j60_output();
			}
		}
	}	
	
	// if one or more packets remain, requeue this poll handler
	if (enc28j60Read(EPKTCNT) > 0) {
		process_poll(&enc28j60_process);
	}	
}

PROCESS_THREAD(enc28j60_process, ev, data)
{
  PROCESS_POLLHANDLER(pollhandler());

  PROCESS_BEGIN();
  
  // initialize_the_hardware();

  tcpip_set_outputfunc( enc28j60_output);

  process_poll(&enc28j60_process);
  
  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);

  enc28j60_exit();

  PROCESS_END();
}

//process interrupt from ENC26j60. When fired, schedule the polling process
// to interrogate the chip. 
void __attribute__ ((interrupt,auto_psv)) _CNInterrupt(){
	
	if( ENC_INT_IO == 0 ) {
		process_poll(&enc28j60_process);		
	}	
	
	IFS1bits.CNIF = 0; //clear CN int flag
}




