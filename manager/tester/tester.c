#include "tester.h"

#include <svp/abort.h>
#include <svp/delegate.h>
#include <stddef.h>

#include "../tlbLib/tlbControl.h"
#include "tests/tests.h"
#include "test_lib.h"


//MLDTODO TLB invalidatie van locked lines testen.
void runNewTest(struct test* test, sl_place_t dst, result_t* result, tlbRef_t tlbReference);


void runAll(result_t* result, sl_place_t dst, char abort, tlbRef_t tlbReference){
	printf("Size of result_t=%u\n", sizeof(result_t));
	struct test test;

	test = TEST_REF(trCal, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testClock, 1, 1);
	runNewTest(&test, dst, result, tlbReference);


	int quiet = 1;
	test = TEST_REF(b_4k, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_4k_p, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_4k_t, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_4k_d, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_4k_pt, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_4k_pd, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_4k_td, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_4k_ptd, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);



	quiet = 1;

	test = TEST_REF(b_2m, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_2m_p, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_2m_t, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_2m_d, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_2m_pt, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_2m_pd, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_2m_td, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_2m_ptd, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);






	quiet = 1;

	test = TEST_REF(b_1g, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_1g_p, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_1g_t, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_1g_d, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_1g_pt, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_1g_pd, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_1g_td, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_1g_ptd, quiet, 1);
	runNewTest(&test, dst, result, tlbReference);







	test = TEST_REF(testSmallPageAlignedRW, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testNotLineAlignedR, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testNotLineAlignedRW, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testLineBoundariesR, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testLineBoundariesRW, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testUnalignedR, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testUnalignedRW, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testSmallPageBoundariesR, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testSmallPageBoundariesRW, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testMediumPageBoundariesRW, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testNotSmallPageAligned64RW, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testNotMediumPageAligned4096RW, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testNotLargePageAligned2MRW, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(action, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

//	test = TEST_REF(testAllPageSizes, 1, 1);
//	runNewTest(&test, dst, result, tlbReference);

//	test = TEST_REF(testTLBEntryUnlock, 1, 1);
//	runNewTest(&test, dst, result, tlbReference);

//	test = TEST_REF(testFullSet, 1, 1);
//	runNewTest(&test, dst, result, tlbReference);
//
//	test = TEST_REF(testFullTLB, 1, 1);
//	runNewTest(&test, dst, result, tlbReference);

	svp_abort();

}


void runNewTest(struct test* test, sl_place_t dst, result_t* result, tlbRef_t tlbReference){
	result_t localResult;
	resetResults(&localResult);

	const char* testName = test->nameFunction(); //Note string literals have static storage duration

	printTestStart(test->quiet, testName);

	test->preTestFunction(tlbReference, test->quiet); //Should test return value
	test->runTestFunction(dst, tlbReference, &localResult, test->abort, test->quiet);
	test->postTestFunction(tlbReference, test->quiet); //Should test return value

	printTestEnd(&localResult, test->quiet, (char*)testName	);

	if(localResult.nrFailed > 0 && test->quiet > 0){
//		printString("Repeating test in verbose mode: **Note: Results may vary!**\n", 0);
//		result_t dummyResult;
//		resetResults(&dummyResult);
//		test(&dummyResult, 0, 0);
	}

	addResults(result, &localResult);
}

void flushDCache(uint64_t cpu){
	uint64_t* ptr = (uint64_t*)0x2A0;
	*ptr = cpu;
}
