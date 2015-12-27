#include "old/syscall_listener.h"

static int syscall_listener_main(size_t managerCore, size_t managerChannel);
static int handleMsg(SysCallMsg_t* msg);
static int receive_net_msg(SysCallMsg_t* const msg, volatile uint64_t* from);
static int send_net_msg(SysCallMsg_t* const msg, volatile uint64_t* dst);

sl_def(syscall_listener_init){
	volatile uint64_t* cPtr = &(mg_devinfo.channels[SYSCALL_CHANNEL]);

	//Tell cpu.io_if.nmux that we want to receive notifications for this channel
	*cPtr = 1;
}sl_enddef;

sl_def(syscall_listener_start,,
		sl_shparm(size_t,   managerCore   ),
		sl_shparm(size_t,   managerChannel)
){
	int res = syscall_listener_main(sl_getp(managerCore), sl_getp(managerChannel));
	PRINT_STRING("SysCall listener failed with error code ");
	PRINT_INT(res);
	svp_abort();

	// Stop compiler from whining
	sl_setp(managerCore, sl_getp(managerCore));
	sl_setp(managerChannel, sl_getp(managerChannel));
}sl_enddef;

static int syscall_listener_main(size_t managerCore, size_t managerChannel){

	volatile uint64_t *channel = &(mg_devinfo.channels[SYSCALL_CHANNEL]);
	//Channel already initialised by init.

	SysCallMsg_t msgBuffer;
	int result;

	while(1){
		result = receive_net_msg(&msgBuffer, channel);
		PRINT_STRING("Syscall Listener received something!!!");
		if(result > 0)	{ continue; }
		if(result <= 0) { return result; } //A whoopsie occurred

		result = handleMsg(&msgBuffer);
		if(result < 0)	{ return result; } //A whoopsie occurred
	}
}

static int handleMsg(SysCallMsg_t* msg){
	if(msg->type == SC_ABORT){
		PRINT_STRING("ABORTING due to SysRq message!");
		svp_abort();
	}
	return 1;
}

static int receive_net_msg(SysCallMsg_t* const msg, volatile uint64_t* from){
	msg->data.part[0] = *from;

	int i = 1;
	for(; i<msg->header.parts; i++){
		msg->data.part[i] = *from;
	}

	return i;
}

static int send_net_msg(SysCallMsg_t* const msg, volatile uint64_t* dst){
	int i = 0;
	for(; i<msg->header.parts; i++){
		*dst = msg->data.part[i];
	}

	return i;
}
