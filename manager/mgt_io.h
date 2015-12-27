#ifndef MGT_IO_H_
#define MGT_IO_H_

#include <stdint.h>
#include "MgtMsg.h"



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

#endif /* MGT_IO_H_ */
