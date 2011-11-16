/*
 * Copyright (c) 2010, C Shucksmith.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
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
 * This file is part of the Contiki OS
 *
 */
/*---------------------------------------------------------------------------*/
/**
* \file
*			dspic Clock using RTC interrupts.
*			here one system tick is half a second, so 
*			ensure CLOCK_SECOND = 2
* \author
*			
*/
/*---------------------------------------------------------------------------*/

#include "sys/clock.h"
#include "sys/etimer.h"
#include <p33Fxxxx.h>

static volatile clock_time_t count;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;

void rtcc_reg_lock(void);
void rtcc_reg_unlock(void);

// TODO: move the etimer poll loop to a higher resolution timer than Timer0 RTC
void tickinterrupt(void)
{
  count++;

  if(etimer_pending()) {
    etimer_request_poll();
  }
  if (--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }
}

void clock_init(void)
{ 
  //reset tick count
  count = 0;
  
  //start seconary osc, and enable rtcc
  //OSCCONbits.LPOSCEN = 1;
  unsigned char oscconl = OSCCON & 0x0f;
  oscconl |= 0x02; //LPOSCEN
  __builtin_write_OSCCONL(oscconl);
  
   rtcc_reg_unlock();
   RCFGCALbits.RTCEN = 1;
   rtcc_reg_lock();
}

clock_time_t clock_time(void)
{
  return count;
}

/**
 * Blocking delay for a multiple of milliseconds
 */
 // TODO: move clock_delay to a higher resolution timer than RTC interrupt
void clock_delay(unsigned int i)
{
	unsigned long waitfor = current_seconds + (i / 1000);
	if (i < 1000) {
		// unimplemented
	} else {
		while (current_seconds < waitfor) { };
	}
	
}

unsigned long clock_seconds(void)
{
  return current_seconds;
}

void inline rtcc_reg_unlock(void)
{
        //TODO - disable interupts for this section
        //diable all ints
        asm volatile (  "mov #0x55,w0 \n"
                                        "mov w0, NVMKEY \n"
                                        "mov #0xAA, w0 \n"
                                        "mov w0, NVMKEY \n");
        RCFGCALbits.RTCWREN = 1;
        //enable all ints
}

void inline rtcc_reg_lock(void)
{
        asm volatile (  "mov #0x55,w0 \n"
                                        "mov w0, NVMKEY \n"
                                        "mov #0xAA, w0 \n"
                                        "mov w0, NVMKEY \n");

        RCFGCALbits.RTCWREN = 0;  
}

