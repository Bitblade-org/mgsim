#include <svp/abort.h>

#include "tests.h"

const char* testNotSmallPageAligned64RW_name(){
	return "Not page aligned RW (small page, 64 bits)";
}

int testNotSmallPageAligned64RW_pre(tlbRef_t tlbReference, char quiet){
	invalidateTlb(tlbReference);
	return 0;
}

void testNotSmallPageAligned64RW_run(sl_place_t destination, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			testNotSmallPageAligned64RW,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int testNotSmallPageAligned64RW_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(testNotSmallPageAligned64RW,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	int fails = 0;
	int testNr = 0;
	for(testNr=0; testNr<(SMALL_PAGE_SIZE / 8); testNr++){
		write64(S_SANDBOXW + testNr * 8, testNr, 1);
		if(read8(S_SANDBOXR + testNr * 8, testNr, 0, 1)){
			fails++;
			write8(S_SANDBOXW + testNr * 8, testNr, quiet);
			read8(S_SANDBOXR + testNr * 8, testNr, abort, quiet);
		}
		if(fails >= 5){
			printString("Ending test prematurely due to too many failures\n", quiet);
			break;
		}
	}
	result_t localResult;
	localResult.nrFailed = fails;
	localResult.nrTests = testNr + 1;

	addResults(result, &localResult);

	sl_setp(result, result);
	sl_setp(abort, abort);	//To give the compiler a warm feeling
	sl_setp(quiet, quiet);	//To give the compiler a warm feeling
}
sl_enddef;

const char* testNotMediumPageAligned4096RW_name(){
	return "Not page aligned RW (medium page, 64 bits per 4096 bytes)";
}

int testNotMediumPageAligned4096RW_pre(tlbRef_t tlbReference, char quiet){
	invalidateTlb(tlbReference);
	return 0;
}

void testNotMediumPageAligned4096RW_run(sl_place_t destination, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			testNotMediumPageAligned4096RW,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int testNotMediumPageAligned4096RW_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(testNotMediumPageAligned4096RW,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	int fails = 0;
	int testNr = 0;
	for(testNr=0; testNr<(MEDIUM_PAGE_SIZE / SMALL_PAGE_SIZE); testNr++){
		write64(M_SANDBOXW + testNr * SMALL_PAGE_SIZE, testNr, 1);
		if(read8(M_SANDBOXR + testNr * SMALL_PAGE_SIZE, testNr, 0, 1)){
			fails++;
			write8(M_SANDBOXW + testNr * SMALL_PAGE_SIZE, testNr, quiet);
			read8(M_SANDBOXR + testNr * SMALL_PAGE_SIZE, testNr, abort, quiet);
		}
		if(fails >= 5){
			printString("Ending test prematurely due to too many failures\n", quiet);
			break;
		}
	}
	result_t localResult;
	localResult.nrFailed = fails;
	localResult.nrTests = testNr + 1;

	addResults(result, &localResult);

	sl_setp(result, result);
	sl_setp(abort, abort);	//To give the compiler a warm feeling
	sl_setp(quiet, quiet);	//To give the compiler a warm feeling
}
sl_enddef;

const char* testNotLargePageAligned2MRW_name(){
	return "Not page aligned RW (large page, 64 bits per 2MiB)";
}

int testNotLargePageAligned2MRW_pre(tlbRef_t tlbReference, char quiet){
	invalidateTlb(tlbReference);
	return 0;
}

void testNotLargePageAligned2MRW_run(sl_place_t destination, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			testNotLargePageAligned2MRW,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int testNotLargePageAligned2MRW_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(testNotLargePageAligned2MRW,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	int fails = 0;
	int testNr = 0;

	for(testNr=0; testNr<(LARGE_PAGE_SIZE / MEDIUM_PAGE_SIZE); testNr++){
//		write64(L_SANDBOXW + testNr * MEDIUM_PAGE_SIZE, testNr, 1);
//		if(read8(L_SANDBOXR + testNr * MEDIUM_PAGE_SIZE, testNr, 0, 1)){
//			fails++;
			write8(L_SANDBOXW + testNr * MEDIUM_PAGE_SIZE, testNr, quiet);
			read8(L_SANDBOXR + testNr * MEDIUM_PAGE_SIZE, testNr, abort, quiet);
//		}
		if(fails >= 5){
			printString("Ending test prematurely due to too many failures\n", quiet);
			break;
		}
	}
	result_t localResult;
	localResult.nrFailed = fails;
	localResult.nrTests = testNr + 1;

	addResults(result, &localResult);

	sl_setp(result, result);
	sl_setp(abort, abort);	//To give the compiler a warm feeling
	sl_setp(quiet, quiet);	//To give the compiler a warm feeling
}
sl_enddef;
