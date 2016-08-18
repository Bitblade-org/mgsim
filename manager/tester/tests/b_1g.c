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
	parameters.target = L_COUNT64R_V+8 * OFFSET; \
	parameters.expect = OFFSET; \
	parameters.iter = 10; \
	doTimedReadTest(&parameters);


const char* b_1g_name(){ return "Bench read, page: 1 GiB, cached: -           "; }
int b_1g_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_1g_post(tlbRef_t tlbReference, char quiet){ return 0; };
void b_1g_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(42, FLUSH_LOCAL_CACHE + FLUSH_LOCAL_TLB + FLUSH_PAGETABLE_CACHE)
}





const char* b_1g_d_name(){ return "Bench read, page: 1 GiB, cached: DATA        "; }
int b_1g_d_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_1g_d_post(tlbRef_t tlbReference, char quiet){ return 0; };
void b_1g_d_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(42, FLUSH_LOCAL_TLB + FLUSH_PAGETABLE_CACHE);
}




const char* b_1g_p_name(){ return "Bench read, page: 1 GiB, cached: PT          "; }
int b_1g_p_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_1g_p_post(tlbRef_t tlbReference, char quiet){ return 0; };
void b_1g_p_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(46, FLUSH_LOCAL_CACHE + FLUSH_LOCAL_TLB);
}




const char* b_1g_pd_name(){ return "Bench read, page: 1 GiB, cached: PT DATA     "; }
int b_1g_pd_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_1g_pd_post(tlbRef_t tlbReference, char quiet){ return 0; };
void b_1g_pd_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(48, FLUSH_LOCAL_TLB);
}




const char* b_1g_pt_name(){ return "Bench read, page: 1 GiB, cached: PT TLB      "; }
int b_1g_pt_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_1g_pt_post(tlbRef_t tlbReference, char quiet){ return 0; };
void b_1g_pt_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(50, FLUSH_LOCAL_CACHE);
}




const char* b_1g_ptd_name(){ return "Bench read, page: 1 GiB, cached: PT TLB DATA "; }
int b_1g_ptd_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_1g_ptd_post(tlbRef_t tlbReference, char quiet){ return 0; };
void b_1g_ptd_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(52, 0);
}




const char* b_1g_t_name(){ return "Bench read, page: 1 GiB, cached: TLB         "; }
int b_1g_t_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_1g_t_post(tlbRef_t tlbReference, char quiet){ return 0; };
void b_1g_t_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(54, FLUSH_PAGETABLE_CACHE + FLUSH_LOCAL_CACHE);
}




const char* b_1g_td_name(){ return "Bench read, page: 1 GiB, cached: TLB DATA    "; }
int b_1g_td_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_1g_td_post(tlbRef_t tlbReference, char quiet){ return 0; };
void b_1g_td_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(56, FLUSH_PAGETABLE_CACHE)
}
