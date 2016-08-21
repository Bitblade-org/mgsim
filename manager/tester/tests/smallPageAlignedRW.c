#include <svp/abort.h>

#include "tests.h"

const char* testSmallPageAlignedRW_name(){
	return "Small Page Aligned RW";
}

int testSmallPageAlignedRW_pre(tlbRef_t tlbReference, char quiet){
	return 0;
}

void testSmallPageAlignedRW_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			testSmallPageAlignedRW,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int testSmallPageAlignedRW_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(testSmallPageAlignedRW,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	write64(SMALL_SANDBOXW, 0x12345, quiet);
	addSResult(result, read64 (SMALL_SANDBOXR, 0x12345, abort, quiet));
	addSResult(result, read64 (SMALL_SANDBOXR_0, 0x12345, abort, quiet));
	addSResult(result, read64 (SMALL_SANDBOXRW_0, 0x12345, abort, quiet));
	write64(SMALL_SANDBOXRW_0, 0x23456, quiet);
	addSResult(result, read64 (SMALL_SANDBOXR, 0x23456, abort, quiet));

	sl_setp(result, result);
	sl_setp(abort, abort);	//To give the compiler a warm feeling
	sl_setp(quiet, quiet);	//To give the compiler a warm feeling
}
sl_enddef;
