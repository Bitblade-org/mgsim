/*
 * syscall_client.h
 *
 *  Created on: 27 Nov 2015
 *      Author: nijntje
 */

#ifndef MANAGER_SYSCALL_H_
#define MANAGER_SYSCALL_H_

#include <svp/delegate.h>
#include <stdint.h>

#include "../os/syscall_gateway.h"

sl_place_t syscall_target(sl_place_t* newTarget);
void syscall_abort(uint8_t exitCode);

#endif /* MANAGER_SYSCALL_H_ */
