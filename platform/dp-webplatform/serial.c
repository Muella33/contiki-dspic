/*
 * Copyright (c) 2005, 
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: serial.c, $
 */
/**
 * \file
 *         Serial port setup for Web Platform
 * \author
 *         Chris Shucksmith <chris@shucksmith.co.uk>
 */

#include <p33Fxxxx.h>

void dbg_setup_uart(void)
{

	//UART1 SETUP
	//UART can be placed on any RPx pin
	//configure for RP14/RP15 to use the FTDI usb->serial converter
	//assign pin 14 to the UART1 RX input register
	//RX PR14 (input)
	RPINR18bits.U1RXR = 14;
	//assign UART1 TX function to the pin 15 output register
	//TX RP15 (output)
	RPOR7bits.RP15R = 3; // U1TX_O

	//peripheral setup
    U1BRG = 85;              //86@80mhz, 85@79.xxx=115200
    U1MODE = 0;              //clear mode register
    U1MODEbits.BRGH = 1;     //use high percison baud generator
    U1STA = 0;               //clear status register
    U1MODEbits.UARTEN = 1;   //enable the UART RX
    IFS0bits.U1RXIF = 0;     //clear the receive flag
    U1STAbits.UTXEN = 1;     //enable UART TX

	// with PIC30 compiler standard library the default printf() calls 
	/// write(), which is soft-linked to the library default write() function 
	// outputing to UART1
  
}
