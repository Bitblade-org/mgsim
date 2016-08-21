#include "tests.h"

const char* testUnalignedRW_name(){
	return "Unaligned R";
}

int testUnalignedRW_pre(tlbRef_t tlbReference, char quiet){
	return 0;
}

void testUnalignedRW_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			testUnalignedRW,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int testUnalignedRW_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(testUnalignedRW,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	uint64_t data = 0;

	for(int i=0; i<8; i++){
		write64(SMALL_SANDBOXW + i, data, quiet);
		addSResult(result, read64(SMALL_SANDBOXR + i, data, abort, quiet));
		data = ~data;
	}

	sl_setp(result, result); //Every time we forget to do this, a compiler
	sl_setp(abort, abort);	 //somewhere on the world throws a null pointer
	sl_setp(quiet, quiet);	 //exception.
}
sl_enddef;
