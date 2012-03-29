
#include <prandom.h>
#include <stdio.h>
#include <rand31-park-miller-carta-int.h>

void randomSeed( void ) {
	
	unsigned char count;
	long unsigned int seedin;
	
	printf("a2d: No entropy source\n");
	
	// seed generator
	seedin = 1;
	rand31pmc_seedi(seedin);
	
	// print first few - should differ on each boot if any entropy was captured
	for (count = 0; count < 8; count++) {
		printf("a2d: %04x %04x %04x %04x\n", rand31pmc_ranlui(), rand31pmc_ranlui(), rand31pmc_ranlui(), rand31pmc_ranlui());
	}
	printf("a2d: done\n");
		
}	
