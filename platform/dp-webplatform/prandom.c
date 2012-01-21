
#include <prandom.h>
#include <p33Fxxxx.h>
#include <stdio.h>
#include <rand31-park-miller-carta-int.h>

void randomSeed( void ) {
	
	unsigned char count;
	long unsigned int seedin;
	
	printf("a2d: Gathering entropy from analogue input AN0\n");
	
	// Web platform v1.1+ hardware exposes header IO2 which contains
	// ADC pins AN0 and AN1, ideal for A2D as they are only 3.3v tolerant
	//  
	// IO2  Label  PIC   Port#  RP#  Output  MaxInput   Description
	// 1    GND    -     -      -    GND     -          ground pin
	// 2    IO9    31    A3     -    3.3v    3.3v       digital IO pin
	// 3    IO10   5     C9     -    3.3v    3.3v       digital IO pin
	// 4    AN1    20    A1     -    3.3v    3.3v       digital IO with ADC
	// 5    IO11   13    A7     -    3.3v    3.3v       digital IO pin
	// 6    AN0    19    A0     -    3.3v    3.3v       digital IO with ADC
	// http://dangerousprototypes.com/docs/Web_Platform:_I/O_Header

	// ADC Port Configuration Regisert
	//    configure AN0 and AN1 for A2D by clearing bits 0,1 from AD1PCFGL
    AD1PCFGL = 0xFFFC;    // ...11 1100
	// AD1PCFGH = 0xFFFF;    // ...11 1111
		
	// Converting 4 Channels, Auto-Sample Start,
	// TAD Conversion Start, Simultaneous Sampling Code

	AD1CON1 = 0x0000;    // ADC1 Control Register
						 // .ADON    1  = module operating
						 // .ADSIDL  0  = continue in Idle mode
						 // .ADDMABM 0  = DMA in Scatter/Gather 
						 // .AD12B   0  = 10bit, 4ch ADC operation
						 // .FORM    00 = unsigned integer
	AD1CON1bits.SSRC=7;	 // .SSRC    111 = int. counter ends sampling/starts conv.
						 // .SIMSAM  0 sample multi channels invidivually in sequence
						 // .ASAM    0 sample when SAMP bit is set
						 // .SAMP    write 1 to start conversion
						 // .DONE    ADC conversion status 1=complete, 0=in progress
						
	
	// Connect AN0 as CH0 input
	AD1CHS0 = 0x0000; 	// ADC input channel select register (Ch0)
						// .CH0NB
						// .CH0SB
						// .CH0NA    0 = negative input A = V.REFL
						// .CH0SA    0 = CH0+=AN0
						
	AD1CHS123 = 0x0;	// ADC input channel select register (Ch1,2,3)
						// .CH123NB  0 = negative input sample B = V.REFL
						// .CH123SB  0 = CH1+=AN0, CH2+=AN1, CH3+=AN2
						// .CH123NA  0 = negative input sample A = V.REFL
						// .CH123SA  0 = CH1+=AN0, CH2+=AN1, CH3+=AN2
						
	AD1CSSL = 0x0001;	// Input Scan Select Register 	0= Skip ANx for input scan
	// AD1CSSH = 0x0000;		
	
	AD1CON2 = 0x0000;   // .VCFG 000 = auto internal voltages
				        // .CSCNA 0 = do not scan inputs
					    // .CHPS = 00 convert using CH0
					    // .SMPI = 0 increment DMA address after every sample
					    // .BUFM = 0 fill from start address
					    // .ALTS = 0 always use input selects for sample A
					  
	AD1CON3 = 0x0000;    // .ADRC = 0  ADC clock derived from system clock
	AD1CON3bits.SAMC=5;	 // .SAMC = 5  Auto sample time 7xTAD
	AD1CON3bits.ADCS=10; // .ADCS = 0  Conversion clock select  10 * TCY = TAD
	
	AD1CON4 = 0x0000;   // DMABL = 0 Allocates 1 word of buffer to each input
	
	AD1CON1bits.ADON = 1; // turn ADC ON
	
	for(count = 0; count < 16; count++) {
		
		// start conversion
		AD1CON1bits.SAMP = 1;		
		while (AD1CON1bits.DONE == 0) {	// poll
		
		}
			
		// result format is unsigned  decimal
		// take least significant 2 bits each iteration	(most noise)			
		seedin = (seedin << 2) | (ADC1BUF0 & 0x0003) ;
		
	}
	
	AD1CON1bits.ADON = 0; // turn ADC Off
	
	// seed generator
	rand31pmc_seedi(seedin);
	
	// print first few - should differ on each boot if any entropy was captured
	for (count = 0; count < 8; count++) {
		printf("a2d: %04x %04x %04x %04x\n", rand31pmc_ranlui(), rand31pmc_ranlui(), rand31pmc_ranlui(), rand31pmc_ranlui());
	}
	printf("a2d: done\n");
	
	// configure A2D pins back to digitial IO
	AD1PCFGL = 0xFFFF; //digital pins
	
}	



