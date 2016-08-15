#include "tests.h"

static uint64_t calculateExpected64(uint64_t start);

const char* b_4k_t_p_name(){
	return "Bench read, page: 4k, cached: TLB DATA";
}


int b_4k_t_p_pre(tlbRef_t tlbReference, char quiet){
	invalidateTlb(tlbReference);
	return 0;
}

void b_4k_t_p_run(sl_place_t destination, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			b_4k_t_p,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int b_4k_t_p_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(b_4k_t_p,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	uint64_t loc;
	uint64_t expect;

	// Make sure the data is present in both cache and TLB
	loc = COUNT8R_START;
	expect = calculateExpected64(0);
	addSResult(result, read8(loc, expect, abort, quiet));


	// Time the read
//	svp_abort();
	int cycles = timedRead_asm(loc, expect);
//	svp_abort();
	if(cycles < 0){
		if(abort == 1){ svp_abort(); }
		snprintf(result->resultText, 80, "Read did not return expected value");
	}

	addSResult(result, 0);
	snprintf(result->resultText, 80, "CYCLES=%d", cycles);

	sl_setp(result, result); //So the compiler doesn't feel left out
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
