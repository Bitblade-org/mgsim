#include "main.h"

sl_def(memreader, , ) {
    sl_index(i);

    unsigned pid = get_current_place();
    unsigned core_id = get_core_id();
    printf("\n");

    printf("Memreader (thread %d) now running on core %d, place_id %x\n", (int)i, core_id, pid);
    uint64_t* loc;
    loc = (uint64_t*)0x100000; printf("Location %p contains %u\n", loc, *loc);
    loc = (uint64_t*)0x200000; printf("Location %p contains %u\n", loc, *loc);
    loc = (uint64_t*)0x300000; printf("Location %p contains %u\n", loc, *loc);
    loc = (uint64_t*)0x400000; printf("Location %p contains %u\n", loc, *loc);
    loc = (uint64_t*)0x500000; printf("Location %p contains %u\n", loc, *loc);
}
sl_enddef

sl_def(manager, , sl_shparm(unsigned, c), sl_shparm(uint64_t, p)) {
    sl_index(i);

    unsigned pid = get_current_place();
    unsigned core_id = get_core_id();
    printf("Will run manager thread %d on core %d, place_id %x\n", (int)i, core_id, pid);

	first_pt((pt_t*)sl_getp(p));
    manager_loop(sl_getp(c));
}
sl_enddef

int main(void) {
	printf("Running OS on core %u\n", getIOAddr());

	pt_t* next_table;
	size_t free;

	//Initialise page tables
	if(!init_pt_set(PTS_PBASE, OS_CONTEXT_ID, PTS_VBASE, &next_table, &free)){
		printf("Could not initialise PTS!\n");
		svp_abort();
	}

	init_manager();

	init_memreader();

	while(1){}
}


unsigned getIOAddr(void){
	union asr_param1 io_config;

	mgsim_read_asr(io_config.raw, ASR_IO_PARAMS1);

	return io_config.core_dev_id;
}

//sl_def(manager, , sl_shparm(unsigned, c)) {
//	manager_loop(sl_getp(c));
//}

void init_manager(){
    unsigned pid = get_current_place();

    sl_create(, pid, 0, 1, 1,  ,,manager, sl_sharg(unsigned, c, MANAGER_CHANNEL), sl_sharg(uint64_t, p, (uint64_t)PTS_PBASE));
    sl_detach(); // 1 thread at place p

    //        ,pl,st,en,li,bl,,fe,ar
    //sl_create(, get_current_place(), 0, 1, 1, , , manager, sl_sharg(unsigned, c, MANAGER_CHANNEL) );
    //sl_detach(); // 1 thread at place p
}

void init_memreader(){
	for(int i=0; i<2000; i++){
		asm("NOP");
	}

    sl_place_t p;
    int r = sep_alloc(root_sep, &p, SAL_EXACT, 1);
    if (r == -1){
    	printf("SEP Unable to allocate core for memreader");
    }

    sl_create(, p, 0, 1, 1,  ,,memreader, );
    sl_detach(); // 1 thread at place p
}
