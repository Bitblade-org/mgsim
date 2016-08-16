
//MLDTODO-DOC: It will be the responsibility of the OS to make sure a physical
//				memory location can only be mapped to a single page size.

#ifndef MANAGER_TEST_TESTS_H_
#define MANAGER_TEST_TESTS_H_

#include <svp/delegate.h>

#include "test_lib.h"
#include "tests/tests.h"
#include "../tlbLib/tlbRef.h"

#define TEST_REF(NAME, QUIET, ABORT) (struct test){ .preTestFunction = & NAME ## _pre, .runTestFunction = & NAME ## _run, .postTestFunction = & NAME ## _post, .quiet = QUIET, .abort = ABORT, .nameFunction = & NAME ## _name}



typedef int (*preTest_ft)(tlbRef_t tlbReference, char quiet);
typedef void (*runTest_ft)(sl_place_t destination, result_t* result, char abort, char quiet);
typedef int (*postTest_ft)(tlbRef_t tlbReference, char quiet);
typedef const char* (*testName_ft)();

struct test {
	preTest_ft preTestFunction;
	runTest_ft runTestFunction;
	postTest_ft postTestFunction;
	testName_ft nameFunction;
	char quiet;
	char abort;
};

void run(char abort);
void runAll(result_t *result, sl_place_t dst, char abort, tlbRef_t tlbReference);
void runTest(char test, sl_place_t dst, char name[], result_t* result, char abort, char quiet, tlbRef_t tlbReference);
void flushDCache(uint64_t cpu);

#endif /* MANAGER_TEST_TESTS_H_ */
