#include "syscall_gateway.h"

#include <svp/abort.h>
#include <svp/delegate.h>

#include "../clientLib/syscall.h"
#include "../debug.h"

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

/**
 * Method to initiate the syscall subsystem.
 * Run this method on the target core!
 */
sl_def(syscall_init,, sl_shparm(sl_place_t, syscall_gateway)) {
	sl_place_t syscall_gateway = sl_getp(syscall_gateway);
	syscall_target(&syscall_gateway);

//    sl_index(i);
//    unsigned pid = get_current_place();
//    unsigned core_id = get_core_id();
//
//    output_string("MemTester (thread ", 2);
//    output_uint((unsigned int)i, 2);
//    output_string(") now running on core ", 2);
//    output_uint(core_id, 2);
//    output_string(", place_id ", 2);
//    output_hex(pid, 2);
//    output_char('\n', 2);
//    output_char('\n', 2);
//    output_char('\n', 2);
//
//    run(0);

    //Let the compiler know we care
	sl_setp(syscall_gateway, sl_getp(syscall_gateway));
}
sl_enddef
