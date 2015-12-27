#ifndef SYSCALL_GATEWAY_H_
#define SYSCALL_GATEWAY_H_

#include <svp/delegate.h>
#include <svp/abort.h>
#include "debug.h"

sl_decl(gateway_syscall_abort,,	sl_shparm(unsigned char, exitCode));

#endif /* SYSCALL_GATEWAY_H_ */
