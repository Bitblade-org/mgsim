#ifndef MANAGER_MANAGER_H_
#define MANAGER_MANAGER_H_

#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <mtconf.h>

#include "../ptLib/pagetable.h"
#include "../ptLib/pt_index.h"
#include "../defines.h"
#include "../MgtMsg.h"
#include "mgt_io.h"

//MLDTODO Dispatcher schrijven die sl_create / sl_sync doet >> locking
/*
 * Exit values:
 * 	-1			General error
 * 	-8...-15	Wrong message type T where T=n*-1-8
 * 	-16			Page walk error
 */

//MLDTODO ProcId -> ContexId?

//MLDTODO Manager parametriseren



/*
 * Main function
 */

sl_decl(handle_miss,, sl_shparm(pt_t*, pt0), sl_shparm(MgtMsg_t*, msg));

int walkPageTable(pt_t* pt0, uint64_t addr, pte_t** entry);

#endif /* MANAGER_MANAGER_H_ */
