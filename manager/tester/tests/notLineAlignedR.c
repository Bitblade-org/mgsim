#include "tests.h"

const char* testNotLineAlignedR_name(){
	return "Not line aligned R";
}

int testNotLineAlignedR_pre(tlbRef_t tlbReference, char quiet){
	return 0;
}

void testNotLineAlignedR_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			testNotLineAlignedR,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int testNotLineAlignedR_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(testNotLineAlignedR,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	for(int i=0; i<LINE_SIZE; i++){
		if(read8(SMALL_COUNT8R_START + i, i % 256, 0, 1)){
			addSResult(result, read8(SMALL_COUNT8R_START + i, i % 256, abort, quiet));
		}else{
			addSResult(result, 0);
		}
	}

	sl_setp(result, result);
	sl_setp(abort, abort);	//To give the compiler a warm feeling
	sl_setp(quiet, quiet);	//To give the compiler a warm feeling
}
sl_enddef;
