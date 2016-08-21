#include "dispatcher.h"

#include <svp/abort.h>
#include <mgsim.h>
#include "../ioLib/IO.h"


sl_def(dispatcher_init,, sl_shparm(unsigned, channel)){
    volatile uint64_t* cPtr = (uint64_t*)getNotificationChannelAddress(sl_getp(channel));


	//Tell cpu.io_if.nmux that we want to receive notifications for this channel
	*cPtr = 1;

	// To stop compiler from whining
	sl_setp(channel, sl_getp(channel));
}
sl_enddef;

sl_def(dispatcher,, sl_shparm(unsigned, channelNr), sl_shparm(uint64_t, pt0)) {
	pt_t* 				pt0 		= (pt_t*)sl_getp(pt0);
    volatile uint64_t*  cPtr = (uint64_t*)getNotificationChannelAddress(sl_getp(channelNr));

	//Channel already initialised by manager_dispatcher_init.

	int result;

	while(1){
		MgtMsg_t msgBuffer;
		asm volatile("stq $31, 681($31)");
		msgBuffer.data.part[1] = *cPtr;
		msgBuffer.data.part[0] = *cPtr;
		asm volatile("stq $31, 682($31)");

		if(msgBuffer.type != MISS){
			output_string("Page walker dispatcher received an unrecognised message\n", 2);
			svp_abort();
		}

	    // Place id 1 means: Same core, but size 1
		sl_create(,1,,,,,(sl__exclusive, sl__force_wait), handle_miss,
				sl_sharg(pt_t*, 	pt0, 	pt0),
				sl_sharg(MgtMsg_t*, msg, 	&msgBuffer),
		);
		sl_sync();
	}

	output_string("Page walker dispatcher failed!\n", 2);
	svp_abort();

	// To stop compiler from whining
    sl_setp(channelNr, 0);
    sl_setp(pt0, (uint64_t)pt0);
}
sl_enddef;
