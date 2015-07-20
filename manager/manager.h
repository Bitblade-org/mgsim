#ifndef MANAGER_MANAGER_H_
#define MANAGER_MANAGER_H_

#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "defines.h"
#include "managerReq.h"
#include "pagetable.h"
#include "SPSC_queue.h"
#include "managerResp.h"

#define TABLE_OFFSET 4

void out_RefillMessage(const tlbRefillMsg *msg);


/*
 * Simple implementation for the manager.
 *
 * The implementation will handle requests in-order.
 * Not only does this imply laziness on the programmers part, it also
 * guarantees FIFO request handling.
 *
 * The queue (single producer, single consumer) will be of a fixed size,
 * making thread-safety almost trivial, even using a lockless restriction.
 * At least, that's what I'm going with. Assuming there is a much better way
 * in SL, I'm not putting much effort in the queue.
 */

/*
 * Statically stores the queue
 */
reqQueue_t* getQueue(void);

/*
 * Statically stores the pointer to the first PT
 * can be updated by providing a non-null value for ptr
 */
pt_t* firstPt(pt_t* ptr);

/*
 * Temporary entry points for Manager requests.
 * These functions construct request messages and pass them on to in_netMsg.
 *
 * Return value equals that of in_netMsg
 */
int in_TlbMiss(uint8_t tlbType, uint8_t lineIndex, uint16_t processId, uint64_t vAddr, uint16_t dest );
int in_Invalidate(uint8_t reqType, uint8_t filterPid, uint8_t filterVAddr, uint16_t processId, uint64_t vAddr);


/*
 * Entry point for request messages
 *
 * COPIES the message to the queue,
 * so the msg pointer only needs to be valid during the call.
 *
 * Returns 1 if request has been queued.
 * Returns 0 if req has not been queued due too full queue.
 * Returns <0 on error.
 */
int in_netMsg(managerReq_t* const msg);


/*
 * Send responses
 *
 *
 * Returns 1 if request has been sent/buffered
 * Returns 0 if req has not been sent/buffered due too full buffer.
 * Returns <0 on error.
 */
int out_TlbRefill(void);
int out_invalidate(void);



/*
 * We need untranslated access to the memory
 */
int disableDTlb(void);


/*
 * main loop
 */
void loop(void);
int handleReq();
int handleInvalidation(managerReq_t* req);
int handleMiss(managerReq_t* req);

int walkPageTable(uint64_t addr, size_t len, pte_t** entry, unsigned* levels);
void printTable(pt_t* t);

#endif /* MANAGER_MANAGER_H_ */
