#include "IO.h"


unsigned getIOAddr(void){
	union asr_param1 io_config;

	mgsim_read_asr(io_config.raw, ASR_IO_PARAMS1);

	return io_config.core_dev_id;
}

//This method works on the assumption that the ID of the core and the ID's of
//the devices linked to that core will form a contiguous set.
//This also means 0 is an invalid response.
//MLDTODO Figure out a better way
size_t find_core_device(struct mg_io_info* ioInfo, size_t core_id, struct mg_device_id* needle){
	struct mg_device_id* id = &(((struct mg_device_id*)ioInfo->smc) + 1)[core_id];
	if(id->provider != 1 || id->model != 0 || id->revision != 1){
		// Since the device is expected to have an ID higher than the core and
		// no core can have an ID lower than 0, 0 cannot be valid output.
		return 0;
	}

    for (int i = core_id + 1; i < ioInfo->smc_nr_iodevs; ++i)
    {
    	struct mg_device_id* id = &(((struct mg_device_id*)ioInfo->smc) + 1)[i];
    	if(
    		id->provider == needle->provider &&
    		id->model == needle->model &&
			id->revision == needle->revision
		){
    		return i;
    	}else if(id->provider == 1 && id->model == 0 && id->revision == 1){
    		break;
    	}
    }

    return 0;
}


void get_io_info(struct mg_io_info* ioInfo){
	uint32_t io_params;

    mgsim_read_asr(io_params, ASR_IO_PARAMS1);
    if (io_params == 0){
    	ioInfo->smc_max_iodevs = 0;
    	ioInfo->n_chans = 0;
    	return;
    }

    ioInfo->smc_max_iodevs = (io_params & 0xff);
    ioInfo->n_chans = ((io_params >> 8) & 0xff);

    ioInfo->mg_io_dca_devid = (io_params >> 24) & 0xff;
    ioInfo->smc_dev_id = (io_params >> 16) & 0xff;

    mgsim_read_asr(io_params, ASR_IO_PARAMS2);
    ioInfo->aio_dev_sz = 1UL << (io_params & 0xff);

    mgsim_read_asr(ioInfo->aio_base, ASR_AIO_BASE);

    /* save the notification address */
    mgsim_read_asr(ioInfo->pnc_base, ASR_PNC_BASE);

    ioInfo->smc = ioInfo->aio_base + ioInfo->aio_dev_sz * ioInfo->smc_dev_id;
    ioInfo->smc_nr_iodevs = *(uint64_t*)ioInfo->smc;

//	Can only catch part of the times not all devices are visible.
//    if (ioInfo->nr_iodevs > ioInfo->max_iodevs)
//    {
//        output_string("# warning: not all devices are visible from the I/O core.", 2);
//        ioInfo->nr_iodevs = ioInfo->max_iodevs;
//    }

}
