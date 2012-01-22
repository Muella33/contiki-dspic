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
*	  dspic RTC clock using periodic interrupts and 32khz secondary oscillator.
* \author   Chris Shucksmith
*			
*/
/*---------------------------------------------------------------------------*/

#include "sys/clock.h"
#include "ntpclient.h"
#include <p33Fxxxx.h>
#include <stdio.h>
#include <string.h>

static volatile unsigned long current_seconds = 0;

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
	doRead = 1;
	current_seconds++;
		
  LATAbits.LATA10 = ~LATAbits.LATA10;
}

void clock_init(void)
{ 
	//reset tick count
    unlockRTC();
  
    RCFGCALbits.RTCEN = 0;  	// disable RTC
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
         : /* no outputs */
         : /* no inputs */
         : "w0" );
    RCFGCALbits.RTCWREN = 1;

}

void inline lockRTC(void){
    RCFGCALbits.RTCWREN = 0;
}

//   RTC functions
unsigned short rtcGetWeekMins( void ) {
    // rolls over Sat->Sunday at 00:00
    unsigned short ans = 0;
    readRTC();
    //  10,800 minutes per week
    // 604,800 seconds per week
    // unsigned short 0 - 65,535
    ans = ((wday_hour  >>  8) & 0x0f) * 0x2400;  // day in minutes 24*60
    ans += ((wday_hour >>  4) & 0x0f) * 0x1000;  // hour tens 0-2
    ans += ((wday_hour >>  0) & 0x0f) * 0x100;   // hour units 0-9
    ans += ((min_sec   >> 12) & 0x0f) * 0x10;    // minute tens 0-5
    ans += ((min_sec   >>  8) & 0x0f) * 0x1;     // minute units 0-9
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

unsigned char printRTCTime(char* sbuf) {
	// format RTC to ISO8601 in supplied buffer	
	readRTC();
	strcpy(sbuf, "20##-##-##T##:##:##Z");
    // pack the bcd data in
    sbuf[2] = ((year >> 4) & 0x0f) + '0';
    sbuf[3] = ((year >> 0) & 0x0f) + '0';
    // -
    sbuf[5] = ((month_date >> 12) & 0x0f) + '0';
    sbuf[6] = ((month_date >>  8) & 0x0f) + '0';
    // -
    sbuf[8] = ((month_date >>  4) & 0x0f) + '0';
    sbuf[9] = ((month_date >>  0) & 0x0f) + '0';
    // ' '
    sbuf[11] = ((wday_hour >>  4) & 0x0f) + '0';
    sbuf[12] = ((wday_hour >>  0) & 0x0f) + '0';
    // :
    sbuf[14] = ((min_sec >> 12) & 0x0f) + '0';
    sbuf[15] = ((min_sec >>  8) & 0x0f) + '0';
    // :
    sbuf[17] = ((min_sec >>  4) & 0x0f) + '0';
    sbuf[18] = ((min_sec >>  0) & 0x0f) + '0';

    // sbuf[21] = "SMTWTFS"[(wday_hour >> 8) & 0x0f];
    // sbuf[22] = "uouehra"[(wday_hour >> 8) & 0x0f];
	
	return strlen(sbuf);
}

void clock_set_ntptime(struct ntp_tm *time) {
	uint8_t decade;
	uint16_t bcd;
	char* sbuf = (char*)uip_appdata;	
	
	decade = time->year - 2000;

    unlockRTC();
  
    RCFGCALbits.RTCEN = 0;  	// disable RTC
    ALCFGRPTbits.ALRMEN = 0;    // disable Alarm

    // set RTC write pointer to 3
    RCFGCALbits.RTCPTR = 3;
    
	// Year      0x00YY
	bcd =     ((decade / 10) << 4) 
			| ((decade % 10) << 0);
	RTCVAL = bcd;    
    
	// Month,Day 0xMMDD
	bcd = 	  ((time->month / 10) << 12) 
			| ((time->month % 10) << 8)  
			| ((time->day / 10)   << 4)  
			|  (time->day % 10)   << 0;
	RTCVAL = bcd;    
	
	// Wday,Hour 0x0WHH
	bcd =     (time->weekday     << 8)  
			| ((time->hour / 10) << 4)  
			|  (time->hour % 10) << 0;
	RTCVAL = bcd;
    
	// Min,Sec   0xMMSS
	bcd = 	  ((time->minute / 10) << 12) 
			| ((time->minute % 10) << 8)  
			| ((time->second / 10) << 4)  
			|  (time->second % 10) << 0;
	RTCVAL = bcd;    
	
    RCFGCALbits.CAL = 0;    // calibration 127..0..-128
    
    RCFGCALbits.RTCEN = 1;      // enable RTC
    ALCFGRPTbits.ALRMEN = 1;    // enable Alarm

    lockRTC();
	printRTCTime(sbuf);
	printf("%s\n", sbuf);
}




