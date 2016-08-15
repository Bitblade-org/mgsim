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

    // Place id 1 means: Same core, but size 1
    unsigned manager_pid = 1;

    sl_create(,manager_pid,,,,,,dispatcher_init,sl_sharg(unsigned, channel0, MANAGER_CHANNEL));
    sl_sync();

    sl_create(,manager_pid,,,,,,dispatcher,
    		sl_sharg(unsigned,,MANAGER_CHANNEL),
			sl_sharg(uint64_t,,(uint64_t)PTS_PBASE)
			);
    sl_detach(); // No need for sync on this one. Any manager requests arriving
    			 // before manager is started get buffered or rejected (full buffer).

	for(int i=0; i<2000; i++){
		asm("NOP");
	} // Just so all initialisation printf's from manager have been printed

	/*
	 * ---===[ Reserve and initialise a nice cosy core ]===---
	 */

    sl_place_t p;
    int r = sep_alloc(root_sep, &p, SAL_EXACT, 1);
    if (r == -1){
    	printf("SEP Unable to allocate core for memreader");
    	svp_abort();
    }


    //Figure out it's IO Address
    printf("Figuring out IO Address of victim core\n");
    sl_create(,p,,,,,,getRemoteIOAddr,
    		sl_sharg(unsigned, addr, 0) // Initialising to stop compiler from complaining
			);
    sl_sync();
    unsigned p_io = sl_geta(addr);

    tlbRef_t tlbReference = getTlbReference(&ioInfo, p_io, TLB_TYPE_D);
    printf("IO Address of victim core is %u\n", p_io);
    printf("Expecting victim dTLB to be on id %u\n", tlbReference.nocId);

    enableTlb(getIOAddr(), MANAGER_CHANNEL, tlbReference);

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

	runAll(NULL, p, 0, tlbReference);

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



