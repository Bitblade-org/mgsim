#include "tests.h"
#include "../tester.h"

const char* b_4k_ptd_name(){
	return "Bench read, page: 4k, cached: PT, TLB & DATA";
}


int b_4k_ptd_pre(tlbRef_t tlbReference, char quiet){

	uint64_t loc = ADDR64_START+8*15;
	uint64_t* ptr = (uint64_t*)loc;
	uint64_t expect = 240;

	if(*ptr != expect){
		printf("Test setup failed, unexpected value in memory: %d / 0x%X\n", *ptr, *ptr);
	}

	// Invalidate the TLB
//	invalidateTlb(tlbReference);

	// Flush the local cache
//	flushDCache(5);

	// Flush the page walker cache
//	flushDCache(4);

	// Make sure everything is done
	for(int i=0; i<200; i++){
		asm volatile("nop");
	}

	return 0;
}

void b_4k_ptd_run(sl_place_t destination, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			b_4k_ptd,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int b_4k_ptd_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(b_4k_ptd,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	uint64_t loc = ADDR64_START+8*15;
	uint64_t* ptr = (uint64_t*)loc;
	uint64_t expect = 240;

	// Time the read
//	svp_abort();
	int cycles = timedRead_asm(loc, expect);
//	svp_abort();
	if(cycles < 0){
		if(abort == 1){ svp_abort(); }
		addSResult(result, 1);
		snprintf(result->resultText, 80, "Read did not return expected value");
		return;
	}

	addSResult(result, 0);
	snprintf(result->resultText, 80, "CYCLES=%d", cycles);

	sl_setp(result, result); //So the compiler doesn't feel left out
	sl_setp(abort, abort);
	sl_setp(quiet, quiet);
}
sl_enddef;
