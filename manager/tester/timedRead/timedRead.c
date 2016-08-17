#include "timedRead.h"
#include <stdio.h>
#include "../tester.h"
#include <svp/abort.h>
#include <svp/delegate.h>
#include <svp/sep.h>

void flush(unsigned char flags, tlbRef_t tlbReference){
	if(flags & FLUSH_PAGETABLE_CACHE){
		flushDCache(CPUID_PAGEWALKER);
	}

	if(flags & FLUSH_LOCAL_CACHE){
		flushDCache(LOCAL_CPUID);
	}

	if(flags & FLUSH_LOCAL_TLB){
		invalidateTlb(tlbReference);
		// Callback not implemented, so this will have to do for now.
		for(int i=0; i<250; i++){
			asm volatile("nop;");
		}
	}
}

void preRead(sl_place_t destination, uint64_t target, uint64_t expect, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			sl_preRead,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet),
			sl_sharg(uint64_t,, target),
			sl_sharg(uint64_t,, expect));
	sl_sync();
}

void timedRead(sl_place_t destination, uint64_t target, uint64_t expect, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			sl_timedRead,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet),
			sl_sharg(uint64_t,, target),
			sl_sharg(uint64_t,, expect));
	sl_sync();
}

void testCalibration(sl_place_t destination, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			sl_testCalibration,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

sl_def(sl_preRead,, sl_shparm(result_t*, result), sl_shparm(char, abort),
		sl_shparm(char, quiet), sl_shparm(uint64_t, target), sl_shparm(uint64_t, expect)){

	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);
	uint64_t target = sl_getp(target);
	uint64_t expect = sl_getp(expect);

	timedReadData_t data;
	data.input.address = target;
	data.input.expectation = expect;

	timedRead_asm(&data);

//	printf("TimedRead responded with: data: 0x%X, xor: 0x%X, clock: %u", data.output.read_result, data.output.xor_result, data.output.clock_result);

	if(data.output.xor_result){
		printString("TimedRead setup failed: Read did not return expected value", quiet);
	}

	addSResult(result, read64((uint64_t)&data.output.read_result, expect, abort, quiet));
	printString("TimedRead preread completed in ", quiet);
	printUint(data.output.clock_result, quiet);
	printString(" core cycles (not adjusted for calibration)\n", quiet);

	sl_setp(result, result); //Show the compiler some love
	sl_setp(abort, abort);
	sl_setp(quiet, quiet);
	sl_setp(target, target);
	sl_setp(expect, expect);
}
sl_enddef;

sl_def(sl_timedRead,, sl_shparm(result_t*, result), sl_shparm(char, abort),
		sl_shparm(char, quiet), sl_shparm(uint64_t, target), sl_shparm(uint64_t, expect)){

	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);
	uint64_t target = sl_getp(target);
	uint64_t expect = sl_getp(expect);

	timedReadData_t data;
	data.input.address = target;
	data.input.expectation = expect;

	timedRead_asm(&data);

	if(data.output.xor_result){
		printString("TimedRead timing read failed: Read did not return expected value", quiet);
	}

	addSResult(result, read64((uint64_t)&data.output.read_result, expect, abort, quiet));
	printString("TimedRead timing read completed in ", quiet);
	printUint(data.output.clock_result, quiet);
	printString(" core cycles (not adjusted for calibration)\n", quiet);

	snprintf(result->resultText, 80, "CYCLES=%d", data.output.clock_result + TIMEDREAD_CALIBRATION_VALUE);

	sl_setp(result, result); //Show the compiler some love
	sl_setp(abort, abort);
	sl_setp(quiet, quiet);
	sl_setp(target, target);
	sl_setp(expect, expect);
}
sl_enddef;

sl_def(sl_testCalibration,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){

	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	uint64_t randomData = 4; //As per http://xkcd.com/221/

	timedReadData_t data;
	data.input.address = (uint64_t)&randomData;
	data.input.expectation = 4;

	timedRead_asm(&data);

	if(data.output.xor_result){
		printString("TimedRead calibration read failed: Read did not return expected value", quiet);
	}

	addSResult(result, read64((uint64_t)&data.output.read_result, 4, abort, quiet));

	if(data.output.clock_result + TIMEDREAD_CALIBRATION_VALUE){
		addSResult(result, 1);
		snprintf(result->resultText, 80, "TimedRead calibration is off! %d + %d != 0", data.output.clock_result, TIMEDREAD_CALIBRATION_VALUE);
		printString(result->resultText, quiet);
		printChar('\n', quiet);
		if(abort){ svp_abort(); }
		sl_setp(result, result);
		return;
	}

	snprintf(result->resultText, 80, "Calibration OK");


	printString("TimedRead calibration read completed in ", quiet);
	printUint(data.output.clock_result, quiet);
	printString(" core cycles. Calibration is OK\n", quiet);


	sl_setp(result, result); //Show the compiler some love
	sl_setp(abort, abort);
	sl_setp(quiet, quiet);
}
sl_enddef;
