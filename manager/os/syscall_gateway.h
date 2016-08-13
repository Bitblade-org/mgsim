#ifndef OS_SYSCALL_GATEWAY_H_
#define OS_SYSCALL_GATEWAY_H_

#include <svp/delegate.h>

sl_decl(gateway_syscall_abort,,	sl_shparm(unsigned char, exitCode));
sl_decl(syscall_init,, sl_shparm(sl_place_t, syscall_gateway));


#endif /* OS_SYSCALL_GATEWAY_H_ */
