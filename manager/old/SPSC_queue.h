#ifndef MANAGER_SPSC_QUEUE_H_
#define MANAGER_SPSC_QUEUE_H_

#include <stdint.h>
#include <string.h>

#include "managerReq.h"

/*
 * Implementation of Lamport's circular buffer, modified to work with
 * a weakly ordered memory model by Higham and Kawash
 *
 * L. Higham and J. Kawash. Critical sections and producer/consumer queues
 * in weak memory systems. In ISPAN ’97: Proceedings of the 1997 International
 * Symposium on Parallel Architectures, Algorithms and Networks,
 * page 56, Washington, DC, USA, 1997. IEEE Computer Society.
 *
 * Peek function implemented by me. Does not affect thread-safety since it will only
 * return objects which are already present and will be called from the consumer.
 */

typedef struct{
	uint8_t 	 head : 4;
	uint8_t 	 tail : 4;
	managerReq_t buffer[16];
} reqQueue_t;
#define R_QUEUE_SIZE 16

int push(reqQueue_t *q, managerReq_t* req);
int pop(reqQueue_t *q);
int peek(reqQueue_t *q, managerReq_t** req);


int push(reqQueue_t *q, managerReq_t* req){
	if(q->buffer[q->tail].base.free == 1){
		memcpy((void*)&(q->buffer[q->tail]), req, sizeof(*req));
		q->buffer[q->tail].base.free = 0;
		q->tail = q->tail + 1 % R_QUEUE_SIZE;
		return 1;
	}
	return 0;
}

int pop(reqQueue_t *q){
	if(q->buffer[q->head].base.free != 1){
		q->buffer[q->head].base.free = 1;
		q->head = q->head + 1 % R_QUEUE_SIZE;
		return 1;
	}
	return 0;
}

int peek(reqQueue_t *q, managerReq_t** req){
	if(q->buffer[q->head].base.free != 1){
		*req = &(q->buffer[q->head]);
		return 1;
	}
	return 0;
}


#endif /* MANAGER_SPSC_QUEUE_H_ */
