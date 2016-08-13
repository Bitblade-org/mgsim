#include "dispatcher.h"

#include <svp/abort.h>


sl_def(dispatcher_init,, sl_shparm(unsigned, channel)){
	volatile uint64_t* cPtr = &(mg_devinfo.channels[sl_getp(channel)]);

	//Tell cpu.io_if.nmux that we want to receive notifications for this channel
	*cPtr = 1;

	// To stop compiler from whining
	sl_setp(channel, sl_getp(channel));
}
sl_enddef;

sl_def(dispatcher,, sl_shparm(unsigned, channelNr), sl_shparm(uint64_t, pt0)) {
	pt_t* 				pt0 		= (pt_t*)sl_getp(pt0);
	unsigned 			channelNr 	= sl_getp(channelNr);
	volatile uint64_t*	channel 	= &(mg_devinfo.channels[channelNr]);
	//Channel already initialised by manager_dispatcher_init.

	PRINT_STRING("Manager dispatcher received a notification!\n");

	int result;

	while(1){
		MgtMsg_t msgBuffer;

		result = receive_net_msg(&msgBuffer, channel);
		if(result == 0)	{ continue; }
		if(result < 0)  { break; } //A whoopsie occurred

	    // Place id 1 means: Same core, but size 1
		sl_create(,1,,,,,(sl__exclusive, sl__force_wait), manager_handle,
				sl_sharg(pt_t*, 	pt0, 	pt0),
				sl_sharg(MgtMsg_t*, msg, 	&msgBuffer),
				sl_sharg(int, 		result, 0)
		);
		sl_sync();

		result = sl_geta(result);
		if(result < 0)	{ break; }
	}

    PRINT_STRING("Manager failed with error code ");
    PRINT_INT(result);
    svp_abort();

	// To stop compiler from whining
    sl_setp(channelNr, channelNr);
    sl_setp(pt0, (uint64_t)pt0);
}
sl_enddef;


//int dispatcher_main(pt_t* pt0, unsigned channelNr){
//	volatile uint64_t *channel = &(mg_devinfo.channels[channelNr]);
//	//Channel already initialised by manager_dispatcher_init.
//
//	MgtMsg_t msgBuffer;
//	int result;
//
//	PRINT_STRING("Manager dispatcher received a notification!\n");
//
//	while(1){
//		result = receive_net_msg(&msgBuffer, channel);
//		if(result == 0)	{ continue; }
//		if(result < 0)  { return result; } //A whoopsie occurred
//
//	    // Place id 1 means: Same core, but size 1
//		sl_create(,1,,,,,(sl__exclusive, sl__force_wait),
//				manager_handle, sl_sharg(pt_t*, pt0, pt0), sl_sharg(MgtMsg_t*, msg, &msgBuffer), sl_sharg(int, result, 0));
//		sl_sync();
//
//		result = sl_geta(result);
//		if(result < 0)	{ return result; } //A whoopsie occurred
//	}
//}

