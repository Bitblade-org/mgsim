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
    printf("Thread %d now running on core %d, place_id %x\n", (int)i, core_id, pid);
    printf("Memory Address %p, read from the thread, contains %u\n", n, *n);
    *n = core_id;
    printf("Memory Address %p, read from the thread after writing core_id, contains %u\n", n, *n);

    int asrval;


}
sl_enddef

int main(void) {
    printf("Memory Address %p, read from Main before detatch, contains %u\n", ptr, *ptr);
    sl_place_t p;
    int r = sep_alloc(root_sep, &p, SAL_EXACT, 1);
    if (r == -1){
    	printf("SEP Unable to allocate");
        return 100;
    }

    //        ,pl,st,en,li,bl,,fe,ar
    sl_create(, p, 0, 1, 1,  ,,t1, sl_sharg(uint64_t, c, (uint64_t)ptr) );
    sl_detach(); // 1 thread at place p
    printf("Memory Address %p, read from Main immediately after detatch, contains %u\n", ptr, *ptr);

    unsigned pid = get_current_place();
    unsigned core_id = get_core_id();
    printf("Main       running on core %d, place_id %d\n", core_id, pid);

    while(*ptr == 666){ }

    printf("Memory Address %p, read from Main after loop, contains %u\n", ptr, *ptr);

    // wait 5 seconds
    time_t t = time(0);
    while (time(0) < t + 5) { }

    return 0;
}
