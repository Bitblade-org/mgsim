#include "../../tlbLib/tlbControl.h"
#include "../../tlbLib/tlbRef.h"
#include "../test_lib.h"
#include "../tester.h"

#include <svp/abort.h>
#include <svp/testoutput.h>

const char* action_name(){
	return "ActionInterface dev platform";
}

int action_pre(tlbRef_t tlbReference, char quiet){
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

	uint64_t timerValue = 10000;
	uint64_t startCount;
	uint64_t endCount;
	uint64_t overshoot;

	asm volatile("ldq %0, 168($31)" : "=r"(startCount));
	asm volatile("stq %0, 688($31)" :: "r"(timerValue));
	asm volatile("ldq %0, 688($31)" : "=r"(overshoot));
	asm volatile("ldq %0, 168($31)" : "=r"(endCount));

	snprintf(result->resultText, 80, "Test ran for %d cycles. Reported overshoot: %d", endCount - startCount, overshoot);

	if(endCount - startCount < timerValue || endCount - startCount > timerValue + 100){
		addSResult(result, 1);
	}else{
		addSResult(result, 0);
	}

	sl_setp(result, result); //Every time we forget to do this, a compiler
	sl_setp(abort, abort);	 //somewhere in the world throws a null pointer
	sl_setp(quiet, quiet);	 //exception.
}
sl_enddef;


