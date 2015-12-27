#include "mgt_io.h"

int receive_net_msg(MgtMsg_t* const msg, volatile uint64_t* from){
	msg->data.part[1] = *from;
	msg->data.part[0] = *from;

	return 1;
}

int send_net_msg(MgtMsg_t* const msg, volatile uint64_t* dst){
	dst[1] = msg->data.part[1];
	dst[0] = msg->data.part[0];
	return 1;
}
