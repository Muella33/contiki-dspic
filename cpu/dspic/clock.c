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

unsigned short year;
unsigned short month_date;
unsigned short wday_hour;
unsigned short min_sec;
volatile unsigned char doRead = 0;

void inline unlockRTC(void);
void inline lockRTC(void);
void readRTC(void);

void __attribute__ ((interrupt, auto_psv)) _RTCCInterrupt() {
	IFS3bits.RTCIF = 0;
    count++;
	doRead = 1;
	
  if (--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }
  
  // TODO: move the etimer poll loop to a higher resolution timer than Timer0 RTC
  if(etimer_pending()) {
    etimer_request_poll();
  }
}

void clock_init(void)
{ 
	//reset tick count
	count = 0;
    unlockRTC();
  
    RCFGCALbits.RTCEN = 0;  // disable RTC
    ALCFGRPTbits.ALRMEN = 0;    // disable Alarm

    // set RTC write pointer to 3
    RCFGCALbits.RTCPTR = 3;
    RTCVAL = 0x0011;    // Year      0x00YY
    RTCVAL = 0x0824;    // Month,Day 0xMMDD
    RTCVAL = 0x0221;    // Wday,Hour 0x0WHH
    RTCVAL = 0x4700;    // Min,Sec   0xMMSS

    RCFGCALbits.CAL = 0;    // calibration 127..0..-128
    
	//start seconary osc
    unsigned char oscconl = OSCCON & 0x0f;
    oscconl |= 0x02; //LPOSCEN
    __builtin_write_OSCCONL(oscconl);

    RCFGCALbits.RTCEN = 1;         // enable RTC
    
    // load alarm ccompare value (0)
	// write to four consecutive registers
    ALCFGRPTbits.ALRMPTR = 3;
    ALRMVAL = 0x0000;
    ALRMVAL = 0x0000;
    ALRMVAL = 0x0000;
    ALRMVAL = 0x0000;

    // Rig alarm interrupt for once per sec, perpetually
    ALCFGRPTbits.AMASK = 0001;
    ALCFGRPTbits.CHIME = 1;

    IEC3bits.RTCIE = 1;     // enable RTC alarm interrupt
    IPC15bits.RTCIP = 2;    // priority 2
        
    ALCFGRPTbits.ALRMEN = 1;    // enable Alarm

    lockRTC();
}

clock_time_t clock_time(void)
{
  return count;
}

/**
 * Blocking delay for a multiple of milliseconds - moved to rtimer-arch.c
 *  void clock_delay(unsigned int i) { };
 */

unsigned long clock_seconds(void)
{
  return current_seconds;
}

void inline unlockRTC(void) {
    // Enable RTCC Timer Access
    asm ("mov #0x55,w0\nmov w0, NVMKEY\nmov #0xAA,w0\nmov w0, NVMKEY"
         : /* no outputd */
         : /* no inputs */
         : "w0" );
    RCFGCALbits.RTCWREN = 1;

}

void inline lockRTC(void){
    RCFGCALbits.RTCWREN = 0;
}

//   RTC functions
unsigned short rtcGetWeekMins( void ) {
    // rolls over Sunday->Monday at 00:00
    unsigned short ans = 0;
    readRTC();
    //  10,800 minutes per week
    // 604,800 seconds per week
    // unsigned short 0 - 65,535
    ans = ((wday_hour >> 8) & 0x0f)   * 24*60;  // day in minutes 24*60
    ans += ((wday_hour >>  4) & 0x0f) * 10*60;  // hour tens 0-2
    ans += ((wday_hour >>  0) & 0x0f) * 60;     // hour units 0-9
    ans += ((min_sec >> 12) & 0x0f) * 10;       // minute tens 0-5
    ans += ((min_sec >>  8) & 0x0f) * 1;        // minute units 0-9
    return ans;
}

void readRTC( void ) {
    if (doRead == 0) return;
    doRead = 0;
    
    // Wait for RTCSYNC bit to become ‘0’
    while(RCFGCALbits.RTCSYNC==1);

    // Read RTCC timekeeping register
    RCFGCALbits.RTCPTR=3;
    year       = RTCVAL;
    month_date = RTCVAL;
    wday_hour  = RTCVAL;
    min_sec    = RTCVAL;
}