#include <stdint.h>
#include "tests.h"
#include "../../tlbLib/tlbRef.h"
#include "../test_lib.h"
#include "../timedRead/timedRead.h"

const char* trCal_name(){ return "Timed reader calibration test"; }

int trCal_pre(tlbRef_t tlbReference, char quiet){ return 0; }
int trCal_post(tlbRef_t tlbReference, char quiet){ return 0; };

void trCal_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	uint64_t loc = SMALL_ICOUNT64_START+8*10;
	uint64_t expect = 245;

	testCalibration(destination, result, abort, quiet);
}
