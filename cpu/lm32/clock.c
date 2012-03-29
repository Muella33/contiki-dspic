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
*	  lm32 clock using divide from the global clock
* \author   Chris Shucksmith
*			
*/
/*---------------------------------------------------------------------------*/

#include "sys/clock.h"

#include <stdio.h>
#include <string.h>

static volatile unsigned long current_seconds = 0;
void readRTC(void);

/*
void __attribute__ ((interrupt, auto_psv)) _RTCCInterrupt() {
	IFS3bits.RTCIF = 0;
	doRead = 1;
	current_seconds++;
}
*/

void clock_init(void)
{ 
	# enable timer peripheral
}
 
/**
 * Blocking delay for a multiple of milliseconds - moved to rtimer-arch.c
 *  void clock_delay(unsigned int i) { };
 */

unsigned long clock_seconds(void)
{
  return current_seconds;
}





