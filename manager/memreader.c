#include "memreader.h"

sl_def(memreader, , ) {
    sl_index(i);

//    printf("Memreader io addr: %u\n", getIOAddr());
//    uint64_t* dst = (uint64_t*)TRANSMIT_ADDR(getIOAddr() + 2);
//    MgtMsg_t msg;
//    msg.type = SET;
//
//    msg.set.property = SET_MGT_ADDR_ON_TLB;
//    msg.set.val1 = 0;
//    msg.set.val2 = 1;
//	dst[2] = msg.data.part[0];
//
//	printf("Sync...");
//
//    msg.set.property = SET_STATE_ON_TLB;
//    msg.set.val1 = 1;
//	svp_abort();
//	dst[2] = msg.data.part[0];
//
	uint64_t* loc = (uint64_t*)0x100000;

	//*loc = 0x123456789abcdef;




    unsigned pid = get_current_place();
    unsigned core_id = get_core_id();
    printf("\n");

    printf("Memreader (thread %d) now running on core %d, place_id %x\n", (int)i, core_id, pid);
    printf("Location %p contains %u\n", loc, *loc);
    loc = (uint64_t*)0x420000; printf("location %p contains %u\n", loc, *loc);
    loc = (uint64_t*)0x212345; printf("Location %p contains %u\n", loc, *loc);
    loc = (uint64_t*)0x312345; printf("Location %p contains %u\n", loc, *loc);
    loc = (uint64_t*)0x412345; printf("Location %p contains %u\n", loc, *loc);
    loc = (uint64_t*)0x512345; printf("Location %p contains %u\n", loc, *loc);
}
sl_enddef
