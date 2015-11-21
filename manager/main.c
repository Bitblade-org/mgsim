#include "main.h"

//MLDTODO "slc -b mta - -I/home/nijntje/mgsim_install/share/mgsim/ main.o manager.o memreader.o pagetable.o PTBuilder.o pt_index.o -o boot.bin" does not return or only after a very long time. Note the extra "-"

int main(void) {
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

    sl_create(,manager_pid,,,,,,manager_init,
    		sl_sharg(unsigned, channel0, MANAGER_CHANNEL)
			);
    sl_sync();

    sl_create(,manager_pid,,,,,,manager,
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
    printf("IO Address of victim core is %u\n", p_io);

    //Do all we can to warn (Technically not needed in this specific case)
    sl_create(,p,,,,,sl__exclusive, wait,
    		sl_sharg(short, end, 0) // Initialising to stop compiler from complaining
			);

		//Switch on the dTLB _remotely_.
		//(One DOES NOT want to do this locally if one wishes to remain sane)
		sl_create(,1,,,,,,tlbEnable,
				sl_sharg(unsigned,, MANAGER_CHANNEL),
				sl_sharg(unsigned,, getIOAddr()), //p=1, so this runs on the same core as main
				sl_sharg(unsigned,, p_io + 2) //if the core has io address 5, the iTLB will be 6 and the dTLB 7
				);						//MLDTODO Raphael is going to claim I can't count on that
		sl_sync();

		//MLDTODO It's a longshot, but maybe there is a better way? Response?
		for(int i=0; i<200; i++){
			asm("NOP");
		} // Give the remote TLB some time to mess up the core nice and good.

		sl_seta(end, 1);
	sl_sync();

	printf("Done initialising victim core!\nCreating testing environment...\n");

	createPageEntries(OS_CONTEXT_ID, PTS_PBASE, &next_table, &free);
	writeStartingData();


	printf("Starting memory test suite...\n");

	/*
	 * ---===[ Run memreader ]===---
	 */

	sl_create(,p,,,,,,memTester);
	sl_sync();

	svp_abort();

	while(1){
		asm("NOP");
	}
}


unsigned getIOAddr(void){
	union asr_param1 io_config;

	mgsim_read_asr(io_config.raw, ASR_IO_PARAMS1);

	return io_config.core_dev_id;
}

sl_def(getRemoteIOAddr,,
		sl_shparm(unsigned, addr)
		)
{
	(void)sl_getp(addr); // To stop compiler from whining
	sl_setp(addr, getIOAddr());
}sl_enddef;

sl_def(tlbEnable,,
		sl_shparm(unsigned, channel2), // Numbered to stop compiler from whining
		sl_shparm(unsigned, managerIOAddr),
		sl_shparm(unsigned, tlbIOAddr)
		)
{
    uint64_t* tlbIO = (uint64_t*)TRANSMIT_ADDR(sl_getp(tlbIOAddr));
    MgtMsg_t msg;
    msg.type = SET;

    msg.set.property = SET_MGT_ADDR_ON_TLB;
    msg.set.val1 = sl_getp(managerIOAddr);
    msg.set.val2 = sl_getp(channel2);
    tlbIO[2] = msg.data.part[0];

	printf("Sync...\n"); //WHY!?!

    msg.set.property = SET_STATE_ON_TLB;
    msg.set.val1 = 1;
	tlbIO[2] = msg.data.part[0];

	// Our prayers be with those threads who remain on the core during the
	// imminent all-fubarring apocalypse.

	// To stop compiler from whining
	sl_setp(channel2, sl_getp(channel2));
	sl_setp(managerIOAddr, sl_getp(managerIOAddr));
	sl_setp(tlbIOAddr, sl_getp(tlbIOAddr));
}sl_enddef;

sl_def(wait,,
		sl_shparm(short, end)
		)
{
	while(!sl_getp(end)){
		//Do nothing
	}
	sl_setp(end, 2); // To stop compiler from whining
}sl_enddef;
