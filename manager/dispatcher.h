#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <mtconf.h>

#include "mgt_io.h"
#include "manager.h"
#include "defines.h"
#include "pagetable.h"
#include "MgtMsg.h"
#include "pt_index.h"

sl_decl(dispatcher_init,,sl_shparm(unsigned, channelNr));
sl_decl(dispatcher, , sl_shparm(unsigned, channel), sl_shparm(uint64_t, pt0));



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
//int dispatcher_main(pt_t* pt0, unsigned channelNr);


#endif /* DISPATCHER_H_ */
