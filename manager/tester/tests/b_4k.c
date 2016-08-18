#include <stdint.h>

#include "../../tlbLib/tlbRef.h"
#include "../test_def.h"
#include "../test_lib.h"
#include "../timedRead/timedRead.h"

#define SIMPLETEST(OFFSET, FLUSH)	\
	struct testParameters parameters; \
	parameters.destination = destination; \
	parameters.tlbReference = tlbReference; \
	parameters.abort = abort; \
	parameters.quiet = quiet; \
	parameters.result = result; \
	parameters.doPreRead = 1; \
	parameters.flush = FLUSH; \
	parameters.target = SMALL_ICOUNT64_START+8 * OFFSET; \
	parameters.expect = 255 - OFFSET; \
	parameters.iter = 10; \
	doTimedReadTest(&parameters);

const char* b_4k_name(){ return "Bench read, page: 4 KiB, cached: -           "; }
int b_4k_pre(tlbRef_t tlbReference, char quiet){ return 0;; }
int b_4k_post(tlbRef_t tlbReference, char quiet){ return 0;; };

void b_4k_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(10, FLUSH_LOCAL_CACHE + FLUSH_LOCAL_TLB + FLUSH_PAGETABLE_CACHE);
}





const char* b_4k_d_name(){ return "Bench read, page: 4 KiB, cached: DATA        "; }
int b_4k_d_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_d_post(tlbRef_t tlbReference, char quiet){ return 0; }

void b_4k_d_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(11, FLUSH_LOCAL_TLB + FLUSH_PAGETABLE_CACHE);
}




const char* b_4k_p_name(){ return "Bench read, page: 4 KiB, cached: PT          "; }
int b_4k_p_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_p_post(tlbRef_t tlbReference, char quiet){ return 0; }

void b_4k_p_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(12, FLUSH_LOCAL_CACHE + FLUSH_LOCAL_TLB);
}




const char* b_4k_pd_name(){ return "Bench read, page: 4 KiB, cached: PT DATA     "; }
int b_4k_pd_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_pd_post(tlbRef_t tlbReference, char quiet){ return 0; }

void b_4k_pd_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(13, FLUSH_LOCAL_TLB);
}




const char* b_4k_pt_name(){ return "Bench read, page: 4 KiB, cached: PT TLB      "; }
int b_4k_pt_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_pt_post(tlbRef_t tlbReference, char quiet){ return 0; }

void b_4k_pt_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(14, FLUSH_LOCAL_CACHE);
}




const char* b_4k_ptd_name(){ return "Bench read, page: 4 KiB, cached: PT TLB DATA "; }
int b_4k_ptd_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_ptd_post(tlbRef_t tlbReference, char quiet){ return 0; }

void b_4k_ptd_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(15, 0);
}




const char* b_4k_t_name(){ return "Bench read, page: 4 KiB, cached: TLB         "; }
int b_4k_t_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_t_post(tlbRef_t tlbReference, char quiet){ return 0; }

void b_4k_t_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(16, FLUSH_PAGETABLE_CACHE + FLUSH_LOCAL_CACHE);

}




const char* b_4k_td_name(){ return "Bench read, page: 4 KiB, cached: TLB DATA    "; }
int b_4k_td_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_td_post(tlbRef_t tlbReference, char quiet){ return 0; }

void b_4k_td_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(17, FLUSH_PAGETABLE_CACHE);

}
