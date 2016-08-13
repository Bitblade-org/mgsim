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

/*//MLDQUESTION Extra instructie nodig? Hoe?
 *
 * We need untranslated access to the memory
 */
int disableDTlb(void);


/*
 * Main function
 */

sl_decl(manager_handle,, sl_shparm(pt_t*, pt0), sl_shparm(MgtMsg_t*, msg), sl_shparm(int, result));


int handleMsg(pt_t* pt0, MgtMsg_t* msg);
int handleInvalidation(pt_t* pt0, MgtMsg_t* req);
int handleMiss(pt_t* pt0, MgtMsg_t* req);

int walkPageTable(pt_t* pt0, uint64_t addr, size_t len, pte_t** entry, unsigned* levels);

/*
 * For debugging
 */
void printTable(pt_t* t);

#endif /* MANAGER_MANAGER_H_ */
