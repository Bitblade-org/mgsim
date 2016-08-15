#include "tester.h"

#include <svp/abort.h>
#include <svp/delegate.h>
#include <stddef.h>

#include "../tlbLib/tlbControl.h"
#include "tests/tests.h"
#include "test_lib.h"

//void run(char abort){
//	runAll(NULL, abort);
//}

//MLDTODO TLB invalidatie van locked lines testen.
void runNewTest(struct test* test, sl_place_t dst, result_t* result, tlbRef_t tlbReference);


void runAll(result_t* result, sl_place_t dst, char abort, tlbRef_t tlbReference){
	struct test test;

	test = TEST_REF(testClock, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(b_4k_t_p, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testClock, 1, 1);
	runNewTest(&test, dst, result, tlbReference);

	test = TEST_REF(testSyscall, 1, 1);

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

//	test = TEST_REF(testAllPageSizes, 1, 1);
//	runNewTest(&test, dst, result, tlbReference);

//	test = TEST_REF(testTLBEntryUnlock, 1, 1);
//	runNewTest(&test, dst, result, tlbReference);

//	test = TEST_REF(testFullSet, 1, 1);
//	runNewTest(&test, dst, result, tlbReference);
//
//	test = TEST_REF(testFullTLB, 1, 1);
//	runNewTest(&test, dst, result, tlbReference);


//
//	runTest(14, dst, "Clock test", result, abort, 1, tlbReference);
//	runTest(15, dst, "read timing test", result, abort, 1, tlbReference);

//	invalidateTlb(tlbReference);	runNewTest(&test, dst, result, tlbReference);
//
//	runTest(3, dst, "Line boundaries R", result, abort, 1, tlbReference);
//	invalidateTlb(tlbReference);
//	runTest(4, dst, "Line boundaries RW", result, abort, 1, tlbReference);
//	invalidateTlb(tlbReference);
//	runTest(5, dst, "Unaligned R", result, abort, 1, tlbReference);
//	invalidateTlb(tlbReference);
//	runTest(6, dst, "Unaligned RW", result, abort, 1, tlbReference);
//	invalidateTlb(tlbReference);
//	runTest(7, dst, "Page boundaries of smallest page R", result, abort, 1, tlbReference);
//	invalidateTlb(tlbReference);
//	runTest(8, dst, "Page boundaries of smallest page RW", result, abort, 1, tlbReference);
//	invalidateTlb(tlbReference);
//	runTest(9, dst, "Page boundaries of medium page RW", result, abort, 1, tlbReference);
//	invalidateTlb(tlbReference);
//	runTest(10, dst, "Not page aligned RW (small page, 64 bits)", result, abort, 1, tlbReference);
//	invalidateTlb(tlbReference);
//	runTest(11, dst, "Not page aligned RW (medium page, 64 bits per 4096 bytes)", result, abort, 1, tlbReference);
//	invalidateTlb(tlbReference);
//	runTest(12, dst, "Not page aligned RW (large page, 64 bits per 2MiB)", result, abort, 1, tlbReference);
//	invalidateTlb(tlbReference);
	svp_abort();

}


void runNewTest(struct test* test, sl_place_t dst, result_t* result, tlbRef_t tlbReference){
	result_t localResult;
	resetResults(&localResult);

	const char* testName = test->nameFunction(); //Note string literals have static storage duration

	printTestStart(test->quiet, testName);

	test->preTestFunction(tlbReference, test->quiet); //Should test return value
	test->runTestFunction(dst, &localResult, test->abort, test->quiet);
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


void runTest(char test, sl_place_t dst, char name[], result_t* result, char abort, char quiet, tlbRef_t tlbReference){
	result_t localResult;
	resetResults(&localResult);

	printTestStart(quiet, name);

	switch(test){

		case 2:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testNotLineAlignedRW,
					sl_sharg(result_t*, result2, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 3:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testLineBoundariesR,
					sl_sharg(result_t*, result3, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 4:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testLineBoundariesRW,
					sl_sharg(result_t*, result4, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 5:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testUnalignedR,
					sl_sharg(result_t*, result5, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 6:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testUnalignedRW,
					sl_sharg(result_t*, result6, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 7:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testSmallPageBoundariesR,
					sl_sharg(result_t*, result7, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 8:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testSmallPageBoundariesRW,
					sl_sharg(result_t*, result8, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 9:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testMediumPageBoundariesRW,
					sl_sharg(result_t*, result9, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 10:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testNotSmallPageAligned64RW,
					sl_sharg(result_t*, result10, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 11:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testNotMediumPageAligned4096RW,
					sl_sharg(result_t*, result11, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 12:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testNotLargePageAligned2MRW,
					sl_sharg(result_t*, result12, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 13:
			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
					testSyscall,
					sl_sharg(result_t*, result13, &localResult),
					sl_sharg(char,, abort),
					sl_sharg(char,, quiet));
			sl_sync();
			break;
		case 14:
			testClock_pre(tlbReference, quiet); //Should test return value
			testClock_run(dst, &localResult, abort, quiet);
			testClock_post(tlbReference, quiet); //Should test return value
			break;
		case 15:
			b_4k_t_p_pre(tlbReference, quiet); //Should test return value
			b_4k_t_p_run(dst, &localResult, abort, quiet);
			b_4k_t_p_post(tlbReference, quiet); //Should test return value
			break;
	}



	printTestEnd(&localResult, quiet, name);

	if(localResult.nrFailed > 0 && quiet > 0){
//		printString("Repeating test in verbose mode: **Note: Results may vary!**\n", 0);
//		result_t dummyResult;
//		resetResults(&dummyResult);
//		test(&dummyResult, 0, 0);
	}

	addResults(result, &localResult);
}
