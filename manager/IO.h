/*
 * io.h
 *
 *  Created on: 27 Nov 2015
 *      Author: nijntje
 */

#ifndef ARCH_IO_H_
#define ARCH_IO_H_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <mgsim.h>
#include <mtconf.h>

// If a core is connected to IO, the maximum number of IO devices must be >0.
// Thus, smc_nr_iodevs=0 implies the core is not connected to IO.
struct mg_io_info{
							// aio=async io
							// pnc=notification / interrupt
    size_t smc_max_iodevs;	//Maximum of number of IO devices the [system?/smc?] can handle.
    size_t smc_nr_iodevs;	//Number of IO devices the SMC enumerates.
    size_t n_chans;			//Number of channels [?pnc?]
    size_t aio_dev_sz;		//Size of the aio [?channel?] (Number of bytes reserved per device)
    size_t smc_dev_id;		//Device ID of the SMC
    void* smc;				//Memory location of the SMC mmio mapping
    uint16_t mg_io_dca_devid; //???????????
    char *aio_base;			//Base location of the aio mappings
    long *pnc_base;			//Base location of the pnc mappings
};


size_t find_core_device(struct mg_io_info* ioInfo, size_t core_id, struct mg_device_id* dev_id);
void get_io_info(struct mg_io_info* ioInfo);


#endif /* ARCH_IO_H_ */
