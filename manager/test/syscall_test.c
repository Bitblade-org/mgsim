#include "tests.h"

#include "../syscall.h"

void testSyscall(result_t *result, char abort, char quiet){
	syscall_abort(42);
}
