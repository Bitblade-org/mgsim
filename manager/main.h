#ifndef MANAGER_MAIN_H_
#define MANAGER_MAIN_H_

#define PTS_PBASE ((void*)0x420000) //2MiB aligned = 4KiB aligned
#define PTS_VBASE ((void*)0x420000) //2MiB aligned = 4KiB aligned
#define OS_CONTEXT_ID 0
#define MANAGER_CHANNEL 1

#include <stdio.h>
#include <svp/abort.h>
#include <svp/sep.h>

#include "dispatcher.h"
#include "mgsim.h"
#include "PTBuilder.h"
#include "manager.h"
#include "test/memTester.h"
#include "pt_index.h"
#include "IO.h"


sl_decl(tlbEnable,,
		sl_shparm(unsigned, channel),
		sl_shparm(unsigned, managerIOAddr),
		sl_shparm(unsigned, tlbIOAddr)
		);

sl_decl(wait,,
		sl_shparm(short, end)
		);

sl_decl(getRemoteIOAddr,,
		sl_shparm(unsigned, addr)
		);

/*
 * From IOInterface.cpp:
 * 		ASR_IO_PARAMS1 has 32 bits:
 * 			bits 0-7:   number of I/O devices mapped to the AIO
 * 			bits 8-15:  number of notification channels mapped to the PNC
 * 			bits 16-23: device ID of the SMC (enumeration) device on the I/O bus
 * 			bits 24-31: device ID of this core on the I/O bus
 */

union asr_param1{
	uint32_t 	raw;

	struct{
		uint8_t		nr_io_devices;
		uint8_t		nr_notification_channels;
		uint8_t		smc_dev_id;
		uint8_t		core_dev_id;
	};
};


int main(void);
unsigned getIOAddr(void);
int get_DTLB_id(struct mg_io_info* ioInfo, size_t core_id);
void init_manager(void);
void init_memreader(void);



#endif /* MANAGER_MAIN_H_ */
