#include "mgt_io.h"


inline void send_net_msg(MgtMsg_t* const msg, volatile uint64_t* dst){
	dst[1] = msg->data.part[1];
	dst[0] = msg->data.part[0];
}
