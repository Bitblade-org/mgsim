#include "tests.h"

static uint64_t calculateExpected64(uint64_t start);


void testLineBoundariesR(result_t *result, char abort, char quiet){
	uint64_t loc;
	uint64_t expect;

	loc = COUNT8R_START + LINE_SIZE - 8;
	expect = calculateExpected64(LINE_SIZE - 8);
	addSResult(result, read8(loc, expect, abort, quiet));

	loc = COUNT8R_START + LINE_SIZE;
	expect = calculateExpected64(LINE_SIZE);
	addSResult(result, read8(loc, expect, abort, quiet));

	loc = COUNT8R_START + LINE_SIZE + LINE_SIZE - 8;
	expect = calculateExpected64(LINE_SIZE + LINE_SIZE - 8);
	addSResult(result, read8(loc, expect, abort, quiet));

	loc = COUNT8R_START + LINE_SIZE + LINE_SIZE;
	expect = calculateExpected64(LINE_SIZE + LINE_SIZE);
	addSResult(result, read8(loc, expect, abort, quiet));

	loc = COUNT8R_START + LINE_SIZE + LINE_SIZE + LINE_SIZE - 8;
	expect = calculateExpected64(LINE_SIZE + LINE_SIZE + LINE_SIZE - 8);
	addSResult(result, read8(loc, expect, abort, quiet));
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

