/**
 * \file
 * Support for DP Webplatform on board Microchip 25AA1024 SPI eeprom
 * \author Chris Shucksmith <chris@shucksmith.co.uk> 
 */

/* Copyright (c) 2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * $Id: eeprom.c,v 1.1 2006/06/18 07:49:33 shuckc Exp $
 *
 * Author: Chris Shucksmith <chris@shucksmith.co.uk>
 *
 */

#include "contiki.h"
#include "dev/eeprom.h"
#include "rtimer-arch.h"
#include <p33Fxxxx.h>
#include <stdio.h>
#include <string.h>

//EEPROM setup
#define EEPROM_CS_TRIS          (TRISCbits.TRISC7)
#define EEPROM_CS_IO            (LATCbits.LATC7)
#define EEPROM_SCK_TRIS         (TRISCbits.TRISC6)
#define EEPROM_SDI_TRIS         (TRISBbits.TRISB9)
#define EEPROM_SDO_TRIS         (TRISCbits.TRISC8)

#define EEPROM_SPI_IF           (IFS2bits.SPI2IF)
#define EEPROM_BUF              (SPI2BUF)
#define EEPROM_SPICON1          (SPI2CON1)
#define EEPROM_SPICON1bits      (SPI2CON1bits)
#define EEPROM_SPICON2          (SPI2CON2)
#define EEPROM_SPISTAT          (SPI2STAT)
#define EEPROM_SPISTATbits      (SPI2STATbits)

void eeprom_write_page(unsigned short addr, unsigned char *buf, int size);
void eeprom_config( void );
void eeprom_unconfig( void );

/* 
 * The DP Webplatform has a Microchip 25AA1024 SPI eeprom chip on board,
 * offering 128kb (131,072 bytes) of storage. The device is logically
 * arranged as 512 x 256 byte pages. Each write operation rewrites 
 * exactly one page and incurs the full page write-wear independant of
 * byte count written. The page that holds the first byte written is 
 * selected for the write operation, serial data bytes beyond the page
 * boundary are written to the start of the page (modulus the page size).
 *
 * Reads can begin at any address and read any size, reads continue
 * over page boundaries to the next page.
 *
 * The Contiki eeprom.h functions define eeprom_addr_t as 'unsigned short' which
 * limits us to the first 256 pages (64kb). An overload is implemented to set 
 * the highest bit to read the top 256 pages.
 * 
 * A hardware SPI controller is used. We use controller 0 for the ethernet chip, so
 * use controller 1
 *            EP pin#   PICpin#  RPport
 *     CS       1 CS     3  RC7  RP23
 *     MISO     2 SO     4  RC8  RP24  
 *              3 WP     -             tied high
 *              4 VSS    -             tied Vss
 *     MOSI     5 SI     1  RB9  RP9   (SDA1**)
 *     CLK      6 SCK    2  RC6  RP22
 *              7 HOLD   -             tied high
 *              8 Vcc    -             tied Vcc
 *
 *   ** RB9 is used by the I2C hardware with higher priority than the
 *   reconfigurable peripheral (RP) hardware. We must disable the I2C
 *   bus for the duration of eeprom operations
 *    RB8  SCL1
 *    RB9  SDA1
 *
 */ 

#define EEPROM_CS_LOW()  { EEPROM_CS_IO = 0; }
#define EEPROM_CS_HIGH() { EEPROM_CS_IO = 1; }
#define EEPROM_SPITXRX() 	while ((EEPROM_SPISTATbits.SPITBF == 1) || (EEPROM_SPISTATbits.SPIRBF == 0));

#define INSTR_READ  0x03 // Read data from memory array beginning at selected address
#define INSTR_WRITE 0x02 // Write data to memory array beginning at selected address
#define INSTR_WREN  0x06 // Set the write enable latch (enable write operations)
#define INSTR_WRDI  0x04 // Reset the write enable latch (disable write operations)
#define INSTR_RDSR  0x05 // Read STATUS register
#define INSTR_WRSR  0x01 // Write STATUS register
#define INSTR_PE    0x42 // Page Erase – erase one page in memory array
#define INSTR_SE    0xD8 // Sector Erase – erase one sector in memory array
#define INSTR_CE    0xC7 // Chip Erase – erase all sectors in memory array
#define INSTR_RDID  0xAB // Release from Deep power-down and read electronic signature
#define INSTR_DPD   0xB9 // Deep Power-Down mode

// SPI2 Clock Input        SCK2 RPINR22 SCK2R<4:0>
// SPI2 Slave Select Input  SS2 RPINR23  SS2R<4:0>
// DCI  Serial Data Input  CSDI RPINR24 CSDIR<4:0>
//
// SDO2 01010 RPn tied to SPI2 Data Output
// SCK2 01011 RPn tied to SPI2 Clock Output
//  SS2 01100 RPn tied to SPI2 Slave Select Output

static volatile eeprom_configured = 0;

void eeprom_config( void ) {

	if (eeprom_configured == 1) return;
	
    // disable I2C peripheral
	I2C1CONbits.I2CEN = 0;
	
	// Reconfigurable Port setup
	
	//SPI input (SDI) wire to pin RP24 (input)
	RPINR22bits.SDI2R = 24;
	//CLK2 RP22 (output)
	RPOR11bits.RP22R = 0x0B; // SCK2;
	//MOSI2 RP9 (output)
	RPOR4bits.RP9R = 0x0A; // SDO2_O;
	
	// Peripheral setup 
	EEPROM_CS_HIGH();
    EEPROM_CS_TRIS = 0;  //set direction of CS pin as output (master)
	
	EEPROM_SPICON1 = 0;
	EEPROM_SPICON2 = 0;
	
	// SPI eeprom clock is good to 20Mhz. Fcy is 40Mhz, so divide by 4 gives 10Mhz, about right.
    EEPROM_SPICON1bits.CKE     = 1;
	EEPROM_SPICON1bits.MSTEN   = 1;
	EEPROM_SPICON1bits.SPRE    = 7; // 1:1
	EEPROM_SPICON1bits.PPRE    = 2; // 4:1
	EEPROM_SPISTATbits.SPIEN   = 1;

	eeprom_configured = 1;
}

void eeprom_unconfig( void ) {

	// disable spi
	EEPROM_SPISTATbits.SPIEN   = 0;

    // enable I2C peripheral
	I2C1CONbits.I2CEN = 1;
	
	eeprom_configured = 0;
		
}

#define EEPROMADDRESS (0x00) 
#define EEPROMPAGEMASK (0xFF)   /* 256b page writes */

/**
 * Read bytes from the EEPROM using sequential read.
 */
 
void
eeprom_read(unsigned short addr, unsigned char *buf, int size)
{
  volatile uint8_t dummy;
  printf("eeprom: read dev address %04x to buffer %08x sz %d\n", addr, (int)buf, size);
   
  eeprom_config();
   
  unsigned int i;
  if(size <= 0) {
    return;
  }
  
  EEPROM_CS_LOW();
  
  /* Write RD, 24 bytes of address */
  EEPROM_BUF = INSTR_READ;
  EEPROM_SPITXRX();
  dummy = EEPROM_BUF;
  
  EEPROM_BUF = 0;
  EEPROM_SPITXRX();
  dummy = EEPROM_BUF;
  
  EEPROM_BUF = addr >> 8;
  EEPROM_SPITXRX();
  dummy = EEPROM_BUF;
  
  EEPROM_BUF = addr & 0xff;
  EEPROM_SPITXRX();
  dummy = EEPROM_BUF;
  
  for(i = 0; i < size; i++){
    EEPROM_BUF = 0;
	EEPROM_SPITXRX();
	dummy = EEPROM_BUF;
	//if ((i & 0x0F) == 0) printf("\n %04d  ", i);
	//printf(" %02x", dummy);
	*buf++ = dummy;
  }
  //printf("\n");
  EEPROM_CS_HIGH();
  
  eeprom_unconfig();
  
}
  
/**
 * Write bytes to EEPROM using sequencial write.
 */
void
eeprom_write(unsigned short addr, unsigned char *buf, int size) {
    
	eeprom_config();

	// split multi-page writes into page at a time
	//              addr      addr+sz
	//               0  
	//   ------------#############-------
	//      |     |     |     |     |    
	//           base        extent
	unsigned short base = addr & 0xFF00;			// address of first page to write
	unsigned short extent = ((addr+size) & 0xFF00); // address of last page to write
	
	unsigned short start = addr;  // first write may not be page aligned
	unsigned int sz = (size > (base+0x0100 - addr) ) ? (base+0x0100 - addr) : size; // first write ends at page boundry or size
	unsigned int off = 0;
	while ( base <= extent) {
		printf("eeprom: writing %d bytes to eeprom %04x, buffer offset %d\n", sz, start, off);
		eeprom_write_page(start, buf+off, sz);
		base += 0x0100;
		off += sz;
		start = base;
		sz = (addr+size < start+0x0100) ? (addr+size - start) : 0x0100;
		
	}
	
	eeprom_unconfig();
	
}
 
void
eeprom_write_page(unsigned short addr, unsigned char *buf, int size)
{
  unsigned int i = 0;
  volatile uint8_t dummy;

  if(size <= 0) {
    return;
  }
  
  /* Disable write protection. */
  EEPROM_CS_LOW();
  
  EEPROM_BUF = INSTR_WREN;
  EEPROM_SPITXRX();
  dummy = EEPROM_BUF;

  EEPROM_CS_HIGH();
  
  // wait for one millisecond
  clock_delay(1);
 
  /* perform the write */
  EEPROM_CS_LOW();
  
  EEPROM_BUF = INSTR_WRITE;
  EEPROM_SPITXRX();
  dummy = EEPROM_BUF;
  
  EEPROM_BUF = 0;
  EEPROM_SPITXRX();
  dummy = EEPROM_BUF;
  
  EEPROM_BUF = addr >> 8;
  EEPROM_SPITXRX();
  dummy = EEPROM_BUF;
  
  EEPROM_BUF = addr & 0xff;
  EEPROM_SPITXRX();
  dummy = EEPROM_BUF;
  
  for(i = 0; i < size; ++i){
    EEPROM_BUF = buf[i];
	EEPROM_SPITXRX();
	dummy = EEPROM_BUF;
  }
  
  EEPROM_CS_HIGH();
  
  clock_delay(100);

}
