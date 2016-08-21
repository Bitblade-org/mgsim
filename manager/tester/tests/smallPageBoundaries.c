#include "tests.h"

const char* testSmallPageBoundariesR_name(){
	return "Page boundaries of smallest page R";
}

int testSmallPageBoundariesR_pre(tlbRef_t tlbReference, char quiet){
	return 0;
}

void testSmallPageBoundariesR_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			testSmallPageBoundariesR,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int testSmallPageBoundariesR_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(testSmallPageBoundariesR,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	uint64_t baseR = SMALL_COUNT64R_START;
	uint64_t mod;
	uint64_t data;

	mod = 0;
	data = 0;
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = SMALL_PAGE_SIZE - 8;
	data = mod / 8;
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = SMALL_PAGE_SIZE;
	data = mod / 8;
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 2) - 8;
	data = mod / 8;
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 2);
	data = mod / 8;
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 3) - 8;
	data = mod / 8;
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	sl_setp(result, result);
	sl_setp(abort, abort);	//To give the compiler a warm feeling
	sl_setp(quiet, quiet);	//To give the compiler a warm feeling
}
sl_enddef;

const char* testSmallPageBoundariesRW_name(){
	return "Page boundaries of smallest page RW";
}

int testSmallPageBoundariesRW_pre(tlbRef_t tlbReference, char quiet){
	return 0;
}

void testSmallPageBoundariesRW_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			testSmallPageBoundariesRW,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int testSmallPageBoundariesRW_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(testSmallPageBoundariesRW,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	uint64_t mod;
	uint64_t baseR = SMALL_SANDBOXR;
	uint64_t baseW = SMALL_SANDBOXW;
	uint64_t data;
	//Cannot use random numbers according to SL17:
	// 	However, the following library features are not supported on the MGSim targets:
	//		...
	//		random number generation (oversight; expect support soon)
	//		...

	mod = 0;
	data = 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = SMALL_PAGE_SIZE - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = SMALL_PAGE_SIZE;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 2) - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 2);
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	mod = (SMALL_PAGE_SIZE * 3) - 8;
	data += 1337;
	write64(baseW + mod, data, quiet);
	addSResult(result, read64(baseR + mod, data, abort, quiet));

	sl_setp(result, result);
	sl_setp(abort, abort);	//To give the compiler a warm feeling
	sl_setp(quiet, quiet);	//To give the compiler a warm feeling
}
sl_enddef;
