#include "syscall_gateway.h"

sl_def(gateway_syscall_abort,,	sl_shparm(unsigned char, exitCode)){
	PRINT_STRING("Syscall gateway on core ");
	PRINT_UINT(get_core_id());
	PRINT_STRING(" handling syscall_abort with exit code ");
	PRINT_INT(sl_getp(exitCode));
	PRINT_CHAR('\n');

	svp_abort();

	// Stop compiler from whining
	sl_setp(exitCode, sl_getp(exitCode));
}
sl_enddef
