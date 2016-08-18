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
	parameters.target = M_COUNT64R_V+8 * OFFSET; \
	parameters.expect = OFFSET; \
	parameters.iter = 10; \
	doTimedReadTest(&parameters);

const char* b_2m_name(){ return "Bench read, page: 2 MiB, cached: -           "; }
int b_2m_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_2m_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_2m_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(42, FLUSH_LOCAL_CACHE + FLUSH_LOCAL_TLB + FLUSH_PAGETABLE_CACHE);
}





const char* b_2m_d_name(){ return "Bench read, page: 2 MiB, cached: DATA        "; }
int b_2m_d_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_2m_d_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_2m_d_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(44, FLUSH_LOCAL_TLB + FLUSH_PAGETABLE_CACHE);
}




const char* b_2m_p_name(){ return "Bench read, page: 2 MiB, cached: PT          "; }
int b_2m_p_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_2m_p_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_2m_p_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(46, FLUSH_LOCAL_CACHE + FLUSH_LOCAL_TLB);
}




const char* b_2m_pd_name(){ return "Bench read, page: 2 MiB, cached: PT DATA     "; }
int b_2m_pd_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_2m_pd_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_2m_pd_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(48, FLUSH_LOCAL_TLB);
}




const char* b_2m_pt_name(){ return "Bench read, page: 2 MiB, cached: PT TLB      "; }
int b_2m_pt_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_2m_pt_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_2m_pt_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(50, FLUSH_LOCAL_CACHE);
}




const char* b_2m_ptd_name(){ return "Bench read, page: 2 MiB, cached: PT TLB DATA "; }
int b_2m_ptd_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_2m_ptd_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_2m_ptd_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(52, 0);
}




const char* b_2m_t_name(){ return "Bench read, page: 2 MiB, cached: TLB         "; }
int b_2m_t_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_2m_t_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_2m_t_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(54, FLUSH_PAGETABLE_CACHE + FLUSH_LOCAL_CACHE);
}




const char* b_2m_td_name(){ return "Bench read, page: 2 MiB, cached: TLB DATA    "; }
int b_2m_td_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_2m_td_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_2m_td_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	SIMPLETEST(56, FLUSH_PAGETABLE_CACHE);
}
