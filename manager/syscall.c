#include "syscall.h"

sl_place_t syscall_target(sl_place_t* newTarget){
	static sl_place_t target;
	if(newTarget != NULL){
		target = *newTarget;
	}
	return target;
}

void syscall_abort(uint8_t exitCode){
	sl_create(,syscall_target(NULL),,,,,(sl__exclusive, sl__force_wait),
			gateway_syscall_abort, sl_sharg(uint8_t, exitCode, exitCode));
	sl_sync();
}


