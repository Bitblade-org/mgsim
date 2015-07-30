#include <mgsim.h>
#include <mtconf.h>
#include <stdio.h>
#include <time.h>
#include <svp/sep.h>
#include <svp/delegate.h>  // voor get_current_place()

sl_def(t1,,sl_shparm(uint64_t, c)) {
    sl_index(i);
    uint64_t* n = (uint64_t*)sl_getp(c);

    unsigned pid = get_current_place();
    unsigned core_id = get_core_id();
    printf("    T%d: now running on core %d, place_id %x\n", (int)i, core_id, pid);
    printf("    T%d: Memory Address %p, read from the thread, before writing contains %u\n", (int)i, n, *n);
    *n = 42;
    printf("    T%d: Memory Address %p, read from the thread after writing, contains %u\n", (int)i, n, *n);

    int asrval;


}
sl_enddef

int main(void) {
    unsigned pid = get_current_place();
    unsigned core_id = get_core_id();
    printf("Main method now running on core %d, place_id %d\n", core_id, pid);

    uint64_t* ptr = (uint64_t*)0x4200000;
    *ptr = 1337;

    sl_place_t p;
    int r = sep_alloc(root_sep, &p, SAL_EXACT, 1);
    if (r == -1){
    	printf("SEP Unable to allocate");
        return 100;
    }

    printf("Memory Address %p, read from Main before detatch, contains %u\n", ptr, *ptr);
    printf("Main will now spawn thread and wait 100.000 cycles for the contents of memory address %p to change\n", ptr);

    //        ,pl,st,en,li,bl,,fe,ar
    sl_create(, p, 0, 1, 1,  ,,t1, sl_sharg(uint64_t, c, (uint64_t)ptr) );
    sl_detach(); // 1 thread at place p
    int count = 0;


//    time_t t = time(0);
//    while (time(0) < t + 1) { }
    while(*ptr == 1337 && count < 100000){// && time(0) < t + 10){
//    	for(int i=0; i<10000; i++){
//    		if(*ptr != 1337){
//    			break;
//    		}
    		asm("NOP");
    		count++;
//    	}
//		printf("Main is still waiting for the contents of memory address %p to change\n", ptr);
    }
    printf("\nMain got bored and is done waiting for the contents to change from %u to 42...\n", *ptr);
    printf("Lets try reading it one last try... Who knows... Might get lucky...\n");

    if(*ptr == 42){
    	printf("\nSee? Speaking up helps :P. Memory Address %p, read from Main after loop, now contains %u\n\n\n", ptr, *ptr);
    }else{
    	printf("\nNo change? Weird... When I tried this, the memory synchronised after/during a call to printf\n\n\n");
    }

    // wait 1 seconds
    time_t t = time(0);
    while (time(0) < t + 1) { }

    return 0;
}
