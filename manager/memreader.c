#include "memreader.h"

sl_def(memreader, , ) {
    sl_index(i);
    unsigned pid = get_current_place();
    unsigned core_id = get_core_id();

    printf("Memreader (thread %d) now running on core %d, place_id %x\n", (int)i, core_id, pid);

	uint64_t* loc;
	uint64_t data;

	loc = (uint64_t*)0x440000;
	printf("Memreader will start request for location %p after the break\n", loc);
	svp_abort();
	data = *loc;
    printf("location %p contains 0x%X (expecting 0x12345)\n", loc, data);

    loc = (uint64_t*)0x500000;
	printf("Memreader will start request for location %p after the break\n", loc);
	svp_abort();
	data = *loc;
    printf("location %p contains 0x%X\n", loc, data);

    loc = (uint64_t*)0x510000;
	printf("Memreader will start request for location %p after the break\n", loc);
	svp_abort();
	data = *loc;
    printf("location %p contains 0x%X\n", loc, data);

    loc = (uint64_t*)0x520000;
	printf("Memreader will start request for location %p after the break\n", loc);
	svp_abort();
	data = *loc;
    printf("location %p contains 0x%X\n", loc, data);

    loc = (uint64_t*)0x530000;
	printf("Memreader will start request for location %p after the break\n", loc);
	svp_abort();
	data = *loc;
    printf("location %p contains 0x%X\n", loc, data);
}
sl_enddef
