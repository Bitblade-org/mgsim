#include "tests.h"

static uint64_t calculateExpected64(uint64_t start);

const char* testUnalignedR_name(){
	return "Unaligned RW";
}

int testUnalignedR_pre(tlbRef_t tlbReference, char quiet){
	invalidateTlb(tlbReference);
	return 0;
}

void testUnalignedR_run(sl_place_t destination, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			testUnalignedR,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int testUnalignedR_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(testUnalignedR,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	uint64_t loc;
	uint64_t expect;

	for(int i=0; i<8; i++){
		loc = COUNT8R_START + i;
		expect = calculateExpected64(i);
		addSResult(result, read64(loc, expect, abort, quiet));
	}

	sl_setp(result, result); //To assure the compiler this isn't Python
	sl_setp(abort, abort);
	sl_setp(quiet, quiet);
}
sl_enddef;

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

