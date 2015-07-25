#ifndef MANAGER_MANAGER_H_
#define MANAGER_MANAGER_H_

#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <mtconf.h>

#include "defines.h"
#include "pagetable.h"
#include "MgtMsg.h"

/*
 * Exit values:
 * 	-1			General error
 * 	-8...-15	Wrong message type T where T=n*-1-8
 * 	-16			Page walk error
 */

//MLDTODO ProcId -> ContexId?

//MLDTODO Manager parametriseren

//MLDQUESTION Hoe extra instructies definieren?
//MLDQUESTION Hoe oude geheugenmanager checks uitschakelen (voor manager)? (Invalid access by memory....)
//MLDQUESTION Hoe manager draaien naast test binary? ("os" binary die beide initialiseert!)

#define NOTIFICATION_CHANNEL 2

/*
 * Statically stores the pointer to the first PT
 * can be updated by providing a non-null value for ptr
 */
pt_t* first_pt(pt_t* ptr);


/*
 * Entry point for request messages
 *
 * COPIES the message to the address pointed to by the
 * supplied pointer.
 *
 * Returns 1 if a message was received.
 * Returns 0 if no message was received. (Currently impossible)
 * Returns <0 on error. (Currently impossible)
 */
int receive_net_msg(MgtMsg_t* const msg, volatile uint64_t* from);


/*
 * Send responses
 *
 * dst must be at least 1 below UINT64_MAX
 *
 * Returns 1 if request has been sent
 * Returns 0 if req could not been sent due too full buffer. (Currently impossible)
 * Returns <0 on error. (Currently impossible)
 */
int send_net_msg(MgtMsg_t* const msg, volatile uint64_t* dst);


/*//MLDQUESTION Extra instructie nodig? Hoe?
 *
 * We need untranslated access to the memory
 */
int disableDTlb(void);


/*
 * Main function
 */
int main();

int handleMsg(MgtMsg_t* msg);
int handleInvalidation(MgtMsg_t* req);
int handleSetPT(MgtMsg_t* msg);
int handleMiss(MgtMsg_t* req);

int walkPageTable(uint64_t addr, size_t len, pte_t** entry, unsigned* levels);

/*
 * For debugging
 */
void printTable(pt_t* t);

#endif /* MANAGER_MANAGER_H_ */
