#include "tests.h"

static uint64_t calculateExpected64(uint64_t start);


void testUnalignedR(result_t *result, char abort, char quiet){
	uint64_t loc;
	uint64_t expect;

	for(int i=0; i<8; i++){
		loc = COUNT8R_START + i;
		expect = calculateExpected64(i);
		addSResult(result, read64(loc, expect, abort, quiet));
	}
}

static uint64_t calculateExpected64(uint64_t start){
	uint64_t result;
	result = (start + 7) % 256; result <<= 8;
	result |= (start + 6) % 256; result <<= 8;
	result |= (start + 5) % 256; result <<= 8;
	result |= (start + 4) % 256; result <<= 8;
	result |= (start + 3) % 256; result <<= 8;
	result |= (start + 2) % 256; result <<= 8;
	result |= (start + 1) % 256; result <<= 8;
	result |= (start) % 256;
	return result;
}

