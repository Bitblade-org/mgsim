#include "../../tlbLib/tlbControl.h"
#include "../../tlbLib/tlbRef.h"
#include "../test_lib.h"
#include "../tester.h"

unsigned int __attribute__((optimize("O0"))) clock_asm0();
unsigned int __attribute__((optimize("O0"))) clock_asm1();
unsigned int __attribute__((optimize("O0"))) clock_asm2();
unsigned int __attribute__((optimize("O0"))) clock_asmc();


const char* action_name(){
	return "ActionInterface dev platform";
}

int action_pre(tlbRef_t tlbReference, char quiet){
	invalidateTlb(tlbReference);
	return 0;
}

void action_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			action,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int action_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(action,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	flushDCache(1);
	flushDCache(2);


	sl_setp(result, result); //Every time we forget to do this, a compiler
	sl_setp(abort, abort);	 //somewhere on the world throws a null pointer
	sl_setp(quiet, quiet);	 //exception.
}
sl_enddef;


