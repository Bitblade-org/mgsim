#include "memreader.h"

sl_def(memreader, , ) {
    sl_index(i);
    unsigned pid = get_current_place();
    unsigned core_id = get_core_id();

    printf("Memreader (thread %d) now running on core %d, place_id %x\n", (int)i, core_id, pid);

	uint64_t* loc;
	uint64_t data;

	loc = (uint64_t*)0x440000;
	printf("Memreader reading from %p\n", loc);
	data = *loc;
	if(data == 0x12345){
		printf("RESULT: OK\n");
	}else{
		printf("RESULT: FAIL: %llX\n", data);
		svp_abort();
	}

    loc = (uint64_t*)0x500000;
	printf("\nMemreader reading from %p\n", loc);
	data = *loc;
	if(data == 0x12345){
		printf("RESULT: OK\n");
	}else{
		printf("RESULT: FAIL: %llX\n", data);
		svp_abort();
	}

    loc = (uint64_t*)0x510000;
	printf("\nMemreader reading from %p\n", loc);
	data = *loc;
	if(data == 0x12345){
		printf("RESULT: OK\n");
	}else{
		printf("RESULT: FAIL: %llX\n", data);
		svp_abort();
	}

    loc = (uint64_t*)0x520000;
	printf("\nMemreader reading from %p\n", loc);
	data = *loc;
	if(data == 0x12345){
		printf("RESULT: OK\n");
	}else{
		printf("RESULT: FAIL: %llX\n", data);
		svp_abort();
	}

    printf("\nMemreader has undergone an identity crisis and assumes the role of a memreader/writer\n");

    loc = (uint64_t*)0x500000;
	printf("\nMemreader WRITING to %p\n", loc);
	*loc = 0x23456;

    loc = (uint64_t*)0x500000;
	printf("\nMemreader reading from %p\n", loc);
	data = *loc;
	if(data == 0x23456){
		printf("RESULT: OK\n");
	}else{
		printf("RESULT: FAIL: %llX\n", data);
		svp_abort();
	}

    loc = (uint64_t*)0x530000;
	printf("\nMemreader WRITING to %p\n", loc);
	*loc = 0x34567;

    loc = (uint64_t*)0x500000;
	printf("\nMemreader reading from %p\n", loc);
	data = *loc;
	if(data == 0x34567){
		printf("RESULT: OK\n");
	}else{
		printf("RESULT: FAIL: %llX\n", data);
		svp_abort();
	}


	svp_abort();
}
sl_enddef
