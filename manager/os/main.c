#include "main.h"

#include <svp/abort.h>
#include <svp/delegate.h>
#include <mgsim.h>
#include <mtconf.h>
#include <svp/sep.h>
#include <stdio.h>
#include <svp/testoutput.h>

#include "../defines.h"
#include "../MgtMsg.h"
#include "../ptLib/pagetable.h"
#include "../ptLib/PTBuilder.h"
#include "../tester/test_init.h"
#include "../tester/tester.h"
#include "../manager/dispatcher.h"
#include "../tlbLib/tlbControl.h"
#include "../tlbLib/tlbRef.h"



//void enu(void){
//    size_t i, j;
//
//	/* ensure that we are connected to I/O */
//    	struct mg_io_info ioInfo;
//
//	    get_io_info(&ioInfo);
//
//
//	    mg_devinfo.nchannels = ioInfo.n_chans;
//	    mg_devinfo.channels = ioInfo.pnc_base;
//	    mg_io_place_id = get_local_place();
//	    mg_devinfo.ndevices = ioInfo.smc_nr_iodevs;
//
//	    struct mg_device_id _dev_ids[ioInfo.smc_nr_iodevs];
//	    struct mg_device_id *devenum = &_dev_ids[0];
//	    void *_dev_addrs[ioInfo.smc_nr_iodevs];
//
//	    for (i = 0; i < ioInfo.smc_nr_iodevs; ++i)
//	        devenum[i] = (((struct mg_device_id*)ioInfo.smc) + 1)[i];
//
//	    mg_devinfo.enumeration = devenum;
//		output_string("* I/O enumeration data at 0x", 2);
//		output_hex(devenum, 2);
//		output_char('.', 2);
//		output_ts(2);
//		output_char('\n', 2);
//
//	    /* set up the device base addresses */
//
//	    void **addrs = &_dev_addrs[0];
//
//	    for (i = 0; i < ioInfo.smc_nr_iodevs; ++i)
//	    {
//	        addrs[i] = ioInfo.aio_base + ioInfo.aio_dev_sz * i;
//	    }
//	    mg_devinfo.base_addrs = addrs;
//
//	    /* detect the devices */
//
//	    for (i = 0; i < ioInfo.smc_nr_iodevs; ++i)
//	    {
//	        printf("Detected: %d,%d,%d\n", devenum[i].provider, devenum[i].model, devenum[i].revision);
//	    }
//}



int main(void) {
	struct mg_io_info ioInfo;

    get_io_info(&ioInfo);

	printf("Running OS on IO Addr %u\n", getIOAddr());

	pt_t* next_table;
	size_t free;

	//Initialise page tables
	if(!init_pt_set(PTS_PBASE, OS_CONTEXT_ID, PTS_VBASE, &next_table, &free)){
		printf("Could not initialise PTS!\n");
		svp_abort();
	}

	/*
	 * ---===[ Starting manager ]===---
	 */

    sl_place_t place_walker;
    int result = sep_alloc(root_sep, &place_walker, SAL_EXACT, 1);
    if (result == -1){
    	printf("SEP Unable to allocate core for page walker");
    	svp_abort();
    }

    printf("Figuring out IO Address of page walker core\n");
    sl_create(,place_walker,,,,,,getRemoteIOAddr,
    		sl_sharg(unsigned, addr1, 0) // Initialising to stop compiler from complaining
			);
    sl_sync();
    unsigned nocId_walker = sl_geta(addr1);
    printf("IO Address of page walking core is %u\n", nocId_walker);

    // Place id 1 means: Same core, but size 1
    unsigned manager_pid = 1;

    sl_create(,place_walker,,,,,,dispatcher_init,sl_sharg(unsigned, channel0, MANAGER_CHANNEL));
    sl_sync();

    sl_create(,place_walker,,,,,,dispatcher,
    		sl_sharg(unsigned,,MANAGER_CHANNEL),
			sl_sharg(uint64_t,,(uint64_t)PTS_PBASE)
			);
    sl_detach(); // No need for sync on this one. Any manager requests arriving
    			 // before manager is started get buffered or rejected (full buffer).

	for(int i=0; i<2000; i++){
		asm("NOP");
	} // Just so all initialisation printf's from manager have been printed

    sl_place_t place_tester;
    result = sep_alloc(root_sep, &place_tester, SAL_EXACT, 1);
    if (result == -1){
    	printf("SEP Unable to allocate core for testing platform");
    	svp_abort();
    }


    //Figure out it's IO Address
    printf("Figuring out IO Address of victim core\n");
    sl_create(,place_tester,,,,,,getRemoteIOAddr,
    		sl_sharg(unsigned, addr2, 0) // Initialising to stop compiler from complaining
			);
    sl_sync();
    unsigned nocId_tester = sl_geta(addr2);

    tlbRef_t tlbReference = getTlbReference(&ioInfo, nocId_tester, TLB_TYPE_D);
    printf("IO Address of victim core is %u\n", nocId_tester);
    printf("Expecting victim dTLB to be on id %u\n", tlbReference.nocId);

    enableTlb(nocId_walker, MANAGER_CHANNEL, tlbReference);
	//MLDTODO It's a longshot, but maybe there is a better way? Response?
	for(int i=0; i<200; i++){
		asm("NOP");
	} // Give the remote TLB some time to mess up the core nice and good.


	printf("Done initialising victim core!\nCreating testing environment...\n");

	tester_init(OS_CONTEXT_ID, PTS_PBASE, &next_table, &free);


	printf("Starting memory test suite...\n");

	/*
	 * ---===[ Run memreader ]===---
	 */

	runAll(NULL, place_tester, 0, tlbReference);

//	svp_abort();

	while(1){
		asm("NOP");
	}
}



sl_def(getRemoteIOAddr,,
		sl_shparm(unsigned, addr)
		)
{
	(void)sl_getp(addr); // To stop compiler from whining
	sl_setp(addr, getIOAddr());
}sl_enddef;



