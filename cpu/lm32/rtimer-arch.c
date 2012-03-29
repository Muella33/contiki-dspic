/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *    lm32 rtime specific millisecond timer code, using Lattice Timer peripheral
 *
 * \author
 *    Chris Shucksmith <chris@shucksmith.co.uk>
  */

#include <stdio.h>

#include "sys/energest.h"
#include "sys/rtimer.h"
#include "sys/etimer.h"
#include "rtimer-arch.h"
#include "contiki-conf.h"
#include "sys/clock.h"

volatile unsigned long rcounter = 0;
volatile unsigned long ecounter = 0;
static volatile clock_time_t count;	// milliseconds

/* code for Timer2 ISR - one millisecond interrupt */
/*
void __attribute__((__interrupt__, no_auto_psv)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;  // Clear Timer1 Interrupt Flag

	if (rcounter == 1) rtimer_run_next();
	if (rcounter > 0) rcounter--;
	
	// for use with CLOCK_CONF_SECOND = 100
	if (++ecounter >= 10) {
		ecounter = 0;
		count++;
		
		if(etimer_pending()) {
			etimer_request_poll();
		}
	}
}
*/

clock_time_t clock_time(void)
{
  return count;
}


void rtimer_arch_init(void) {

}

void rtimer_arch_schedule(rtimer_clock_t t) {
	rcounter = t;
}

/**
 * Blocking delay for a multiple of milliseconds
 */
void clock_delay(unsigned int i)
{
	unsigned long waitfor = count + i;
	if (waitfor < count) { 
		while (count > 1000) { }; // wait for overflow		
		while (count < waitfor) { }; // wait for expiry
	} else {
		while (count < waitfor) { };	// wait for expiry
	}
}
