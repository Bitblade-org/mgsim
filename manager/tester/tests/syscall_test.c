#include "../../clientLib/syscall.h"

#include "tests.h"

sl_def(testSyscall,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	syscall_abort(42);

	sl_setp(result, result); //So the compiler doesn't get bored
	sl_setp(abort, abort);
	sl_setp(quiet, quiet);
}
sl_enddef;
