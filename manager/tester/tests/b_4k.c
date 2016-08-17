#include <stdint.h>

#include "../../tlbLib/tlbRef.h"
#include "../test_def.h"
#include "../test_lib.h"
#include "../timedRead/timedRead.h"

const char* b_4k_name(){ return "Bench read, page: 4k, cached: -           "; }
int b_4k_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_4k_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	uint64_t loc = ADDR64_START+8*10;
	uint64_t expect = 245;

	preRead(destination, loc, expect, result, abort, quiet);
	flush(FLUSH_LOCAL_CACHE + FLUSH_LOCAL_TLB + FLUSH_PAGETABLE_CACHE, tlbReference);
	timedRead(destination, loc, expect, result, abort, quiet);
}





const char* b_4k_d_name(){ return "Bench read, page: 4k, cached: DATA        "; }
int b_4k_d_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_d_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_4k_d_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	uint64_t loc = ADDR64_START+8*11;
	uint64_t expect = 244;

	preRead(destination, loc, expect, result, abort, quiet);
	flush(FLUSH_LOCAL_TLB + FLUSH_PAGETABLE_CACHE, tlbReference);
	timedRead(destination, loc, expect, result, abort, quiet);
}




const char* b_4k_p_name(){ return "Bench read, page: 4k, cached: PT          "; }
int b_4k_p_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_p_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_4k_p_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	uint64_t loc = ADDR64_START+8*12;
	uint64_t expect = 243;

	preRead(destination, loc, expect, result, abort, quiet);
	flush(FLUSH_LOCAL_CACHE + FLUSH_LOCAL_TLB, tlbReference);
	timedRead(destination, loc, expect, result, abort, quiet);
}




const char* b_4k_pd_name(){ return "Bench read, page: 4k, cached: PT DATA     "; }
int b_4k_pd_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_pd_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_4k_pd_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	uint64_t loc = ADDR64_START+8*13;
	uint64_t expect = 242;

	preRead(destination, loc, expect, result, abort, quiet);
	flush(FLUSH_LOCAL_TLB, tlbReference);
	timedRead(destination, loc, expect, result, abort, quiet);
}




const char* b_4k_pt_name(){ return "Bench read, page: 4k, cached: PT TLB      "; }
int b_4k_pt_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_pt_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_4k_pt_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	uint64_t loc = ADDR64_START+8*14;
	uint64_t expect = 241;

	preRead(destination, loc, expect, result, abort, quiet);
	flush(FLUSH_LOCAL_CACHE, tlbReference);
	timedRead(destination, loc, expect, result, abort, quiet);
}




const char* b_4k_ptd_name(){ return "Bench read, page: 4k, cached: PT TLB DATA "; }
int b_4k_ptd_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_ptd_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_4k_ptd_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	uint64_t loc = ADDR64_START+8*15;
	uint64_t expect = 240;

	preRead(destination, loc, expect, result, abort, quiet);
	timedRead(destination, loc, expect, result, abort, quiet);
}




const char* b_4k_t_name(){ return "Bench read, page: 4k, cached: TLB         "; }
int b_4k_t_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_t_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_4k_t_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	uint64_t loc = ADDR64_START+8*16;
	uint64_t expect = 239;

	preRead(destination, loc, expect, result, abort, quiet);
	flush(FLUSH_PAGETABLE_CACHE + FLUSH_LOCAL_CACHE, tlbReference);
	timedRead(destination, loc, expect, result, abort, quiet);
}




const char* b_4k_td_name(){ return "Bench read, page: 4k, cached: TLB DATA    "; }
int b_4k_td_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int b_4k_td_post(tlbRef_t tlbReference, char quiet){ return 0; };

void b_4k_td_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	uint64_t loc = ADDR64_START+8*17;
	uint64_t expect = 238;

	preRead(destination, loc, expect, result, abort, quiet);
	flush(FLUSH_PAGETABLE_CACHE, tlbReference);
	timedRead(destination, loc, expect, result, abort, quiet);
}
