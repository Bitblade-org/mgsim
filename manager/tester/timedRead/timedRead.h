#ifndef MANAGER_TESTER_TIMEDREAD_TIMEDREAD_H_
#define MANAGER_TESTER_TIMEDREAD_TIMEDREAD_H_

#include <stdint.h>
#include "../../tlbLib/tlbRef.h"
#include "../test_lib.h"
#include <svp/abort.h>
#include <svp/delegate.h>
#include <svp/sep.h>

struct testParameters {
	sl_place_t destination;
	tlbRef_t tlbReference;
	uint64_t target;
	uint64_t expect;
	result_t* result;
	char doPreRead;
	char flush;
	char quiet;
	char abort;
	char iter;
};

struct input {
	uint64_t address;
	uint64_t expectation;
};

struct output{
	uint64_t read_result;
	uint64_t xor_result;
	uint64_t clock_result;
};

typedef union {
	struct input input;
	struct output output;
} timedReadData_t;

#define TIMEDREAD_CALIBRATION_VALUE -5

#define FLUSH_LOCAL_TLB			1
#define FLUSH_LOCAL_CACHE		2
#define FLUSH_PAGETABLE_CACHE	4

#define CPUID_PAGEWALKER 4

void __attribute__((optimize("O0"))) timedRead_asm(timedReadData_t* data);
void __attribute__((optimize("O0"))) timedRead_asm_calibrationRun(timedReadData_t* data);
void flush(unsigned char flags, tlbRef_t tlbReference);
void preRead(sl_place_t destination, uint64_t target, uint64_t expect, result_t* result, char abort, char quiet);
void timedRead(sl_place_t destination, uint64_t target, uint64_t expect, result_t* result, char abort, char quiet);
void testCalibration(sl_place_t destination, result_t* result, char abort, char quiet);
void doTimedReadTest(struct testParameters* p);

sl_decl(sl_preRead,,        sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet), sl_shparm(uint64_t, target), sl_shparm(uint64_t, expect));
sl_decl(sl_timedRead,,      sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet), sl_shparm(uint64_t, target), sl_shparm(uint64_t, expect));
sl_decl(sl_testCalibration,,sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));

#endif /* MANAGER_TESTER_TIMEDREAD_TIMEDREAD_H_ */
