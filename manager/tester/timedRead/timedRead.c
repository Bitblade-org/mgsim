#include "timedRead.h"
#include <stdio.h>
#include "../tester.h"
#include <svp/abort.h>
#include <svp/delegate.h>
#include <svp/sep.h>


void doTimedReadTest(struct testParameters* p){
	// Make sure every other cache is filled
	p->iter++;

	for(int i=0; i<p->iter; i++){
		preRead(p->destination, p->target, p->expect, p->result, p->abort, p->quiet);
		flush(p->flush, p->tlbReference);
		timedRead(p->destination, p->target, p->expect, p->result, p->abort, p->quiet);
	}

	snprintf(p->result->resultText, 80, "CYCLES: |(%-4d)| %-4d | %-4d | %-4d | %-4d | %-4d | %-4d | %-4d | %-4d | %-4d | %-4d |",
			p->result->metrics[0], p->result->metrics[1], p->result->metrics[2], p->result->metrics[3],
			p->result->metrics[4], p->result->metrics[5], p->result->metrics[6], p->result->metrics[7],
			p->result->metrics[8], p->result->metrics[9], p->result->metrics[10]
	);

}


void flush(unsigned char flags, tlbRef_t tlbReference){
	uint64_t* ptr;
	uint64_t localFlags = flags & (FLUSH_LOCAL_CACHE + FLUSH_LOCAL_TLB);

	if(localFlags){
		ptr = (uint64_t*)(localFlags + 0x2A0);
		*ptr = getCurrentCore();
	}


	if(flags & FLUSH_PAGETABLE_CACHE){
		ptr = (uint64_t*)0x2A2;
		*ptr = CPUID_PAGEWALKER;
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

	addSResult(result, read64(target, expect, abort, quiet));

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
	if(!quiet){
		printf("TimedRead timing read (iter %d) completed in %d core cycles (%d adjusted for calibration)\n",
				result->nextMetric, data.output.clock_result, data.output.clock_result + TIMEDREAD_CALIBRATION_VALUE);
	}

	result->metrics[result->nextMetric++] = data.output.clock_result + TIMEDREAD_CALIBRATION_VALUE;

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
