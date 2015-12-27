#ifndef MANAGER_SYSCALL_LISTENER_H_
#define MANAGER_SYSCALL_LISTENER_H_

#include <mtconf.h>
#include <stddef.h>
#include <stdint.h>
#include <svp/abort.h>

#include "SysCallMsg.h"
#include "syscall.h"
#include "debug.h"


sl_decl(syscall_listener_init);

sl_decl(syscall_listener_start,,	sl_shparm(size_t, managerCore),
									sl_shparm(size_t, managerChannel)
);


#endif /* MANAGER_SYSCALL_LISTENER_H_ */
