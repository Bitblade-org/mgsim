#include "../../tlbLib/tlbControl.h"
#include "../../tlbLib/tlbRef.h"
#include "../test_lib.h"
#include "../tester.h"

#include <svp/abort.h>
#include <svp/testoutput.h>

const char* coreId_name(){
	return "Verify Core ID";
}

int coreId_pre(tlbRef_t tlbReference, char quiet){
	return 0;
}

void coreId_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			coreId,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int coreId_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(coreId,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	uint64_t cid;
	asm volatile("getcid %0" : "=r"(cid));

	snprintf(result->resultText, 80, "Test ran on core %d", cid);

	if(cid == 5){
		addSResult(result, 0);
	}else{
		addSResult(result, 1);
	}

	sl_setp(result, result); //Every time we forget to do this, a compiler
	sl_setp(abort, abort);	 //somewhere in the world throws a null pointer
	sl_setp(quiet, quiet);	 //exception.
}
sl_enddef;


