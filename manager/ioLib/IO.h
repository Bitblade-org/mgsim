
#ifndef ARCH_IO_H_
#define ARCH_IO_H_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <mgsim.h>
#include <mtconf.h>

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
unsigned getIOAddr(void);
void* getPncBase(void);
void* getPncBase(void);
void* getNotificationChannelAddress(unsigned int channel);


#endif /* ARCH_IO_H_ */
