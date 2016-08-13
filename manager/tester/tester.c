#include "tester.h"

#include <svp/abort.h>
#include <svp/delegate.h>
#include <stddef.h>

#include "tests/tests.h"
#include "test_lib.h"

//void run(char abort){
//	runAll(NULL, abort);
//}

void runAll(result_t* result, sl_place_t dst, char abort){
	svp_abort();
	runTest(p_testSmallPageAlignedRW, dst, "Small Page Aligned RW", result, abort, 1);
	svp_abort();

	runTest(p_testNotLineAlignedR, dst, "Not line aligned R", result, abort, 1);
	runTest(p_testNotLineAlignedRW, dst, "Not line aligned RW", result, abort, 1);
	runTest(p_testLineBoundariesR, dst, "Line boundaries R", result, abort, 1);
	runTest(p_testLineBoundariesRW, dst, "Line boundaries RW", result, abort, 1);
	runTest(p_testUnalignedR, dst, "Unaligned R", result, abort, 1);
	runTest(p_testUnalignedRW, dst, "Unaligned RW", result, abort, 1);
	runTest(p_testSmallPageBoundariesR, dst, "Page boundaries of smallest page R", result, abort, 1);
	runTest(p_testSmallPageBoundariesRW, dst, "Page boundaries of smallest page RW", result, abort, 1);
	runTest(p_testMediumPageBoundariesRW, dst, "Page boundaries of medium page RW", result, abort, 1);
	runTest(p_testNotSmallPageAligned64RW, dst, "Not page aligned RW (small page, 64 bits)", result, abort, 1);
	runTest(p_testNotMediumPageAligned4096RW, dst, "Not page aligned RW (medium page, 64 bits per 4096 bytes)", result, abort, 1);
	runTest(p_testNotLargePageAligned2MRW, dst, "Not page aligned RW (large page, 64 bits per 2MiB)", result, abort, 1);
	runTest(p_testSyscall, dst, "Syscall test", result, abort, 1);
}



void runTest(testPtr_t test, sl_place_t dst, char name[], result_t* result, char abort, char quiet){
	result_t localResult;
	resetResults(&localResult);

	printTestStart(quiet, name);

	sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
			&test,
			sl_sharg(result_t*, result0, &localResult),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
//	switch(test){
//		case 0:
//
//		break;
//		case 1:
//			sl_create(,dst,,,,, (sl__exclusive, sl__force_wait),
//					testNotLineAlignedR,
//					sl_sharg(result_t*, result1, &localResult),
//					sl_sharg(char,, abort),
//					sl_sharg(char,, quiet));
//			sl_sync();
//			break;
//		case 2:
//			RUNTEST(2, testNotLineAlignedRW);
//			break;
//		case 3:
//			RUNTEST(3, testLineBoundariesR);
//			break;
//		case 4:
//			RUNTEST(4, testLineBoundariesRW);
//			break;
//		case 5:
//			RUNTEST(5, testUnalignedR);
//			break;
//		case 6:
//			RUNTEST(6, testUnalignedRW);
//			break;
//		case 7:
//			RUNTEST(7, testSmallPageBoundariesR);
//			break;
//		case 8:
//			RUNTEST(8, testSmallPageBoundariesRW);
//			break;
//		case 9:
//			RUNTEST(9, testMediumPageBoundariesRW);
//			break;
//		case 10:
//			RUNTEST(10, testNotSmallPageAligned64RW);
//			break;
//		case 11:
//			RUNTEST(11, testNotMediumPageAligned4096RW);
//			break;
//		case 12:
//			RUNTEST(12, testNotLargePageAligned2MRW);
//			break;
//		case 13:
//			RUNTEST(13, testSyscall);
//			break;
//	}



	printTestEnd(&localResult, quiet, name);

//	if(localResult.nrFailed > 0 && quiet > 0){
//		printString("Repeating test in verbose mode: **Note: Results may vary!**\n", 0);
//		result_t dummyResult;
//		resetResults(&dummyResult);
//		test(&dummyResult, 0, 0);
//	}

	addResults(result, &localResult);
}
