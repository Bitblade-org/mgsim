#include "../../clientLib/syscall.h"

#include "tests.h"

int testSyscall_pre(tlbRef_t tlbReference, char quiet){
	invalidateTlb(tlbReference);
	return 0;
}

void testSyscall_run(sl_place_t destination, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			testSyscall,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int testSyscall_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(testSyscall,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	printString("Attempting to transmit syscall", 0);
	syscall_abort(42);

	sl_setp(result, result); //So the compiler doesn't get bored
	sl_setp(abort, abort);
	sl_setp(quiet, quiet);
}
sl_enddef;
