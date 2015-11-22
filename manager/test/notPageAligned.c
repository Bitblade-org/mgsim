#include "tests.h"
#include <svp/abort.h>

void testNotSmallPageAligned64RW(result_t *result, char abort, char quiet){
	int fails = 0;
	int testNr = 0;
	for(testNr=0; testNr<(SMALL_PAGE_SIZE / 8); testNr++){
		write64(S_SANDBOXW + testNr * 8, testNr, 1);
		if(read8(S_SANDBOXR + testNr * 8, testNr, 0, 1)){
			fails++;
			write8(S_SANDBOXW + testNr * 8, testNr, quiet);
			read8(S_SANDBOXR + testNr * 8, testNr, abort, quiet);
		}
		if(fails >= 5){
			printString("Ending test prematurely due to too many failures\n", quiet);
			break;
		}
	}
	result_t localResult;
	localResult.nrFailed = fails;
	localResult.nrTests = testNr + 1;

	addResults(result, &localResult);
}

void testNotMediumPageAligned4096RW(result_t *result, char abort, char quiet){
	int fails = 0;
	int testNr = 0;
	for(testNr=0; testNr<(MEDIUM_PAGE_SIZE / SMALL_PAGE_SIZE); testNr++){
		write64(M_SANDBOXW + testNr * SMALL_PAGE_SIZE, testNr, 1);
		if(read8(M_SANDBOXR + testNr * SMALL_PAGE_SIZE, testNr, 0, 1)){
			fails++;
			write8(M_SANDBOXW + testNr * SMALL_PAGE_SIZE, testNr, quiet);
			read8(M_SANDBOXR + testNr * SMALL_PAGE_SIZE, testNr, abort, quiet);
		}
		if(fails >= 5){
			printString("Ending test prematurely due to too many failures\n", quiet);
			break;
		}
	}
	result_t localResult;
	localResult.nrFailed = fails;
	localResult.nrTests = testNr + 1;

	addResults(result, &localResult);
}

void testNotLargePageAligned2MRW(result_t *result, char abort, char quiet){
	int fails = 0;
	int testNr = 0;

	for(testNr=0; testNr<(LARGE_PAGE_SIZE / MEDIUM_PAGE_SIZE); testNr++){
//		write64(L_SANDBOXW + testNr * MEDIUM_PAGE_SIZE, testNr, 1);
//		if(read8(L_SANDBOXR + testNr * MEDIUM_PAGE_SIZE, testNr, 0, 1)){
//			fails++;
			write8(L_SANDBOXW + testNr * MEDIUM_PAGE_SIZE, testNr, quiet);
			read8(L_SANDBOXR + testNr * MEDIUM_PAGE_SIZE, testNr, abort, quiet);
//		}
		if(fails >= 5){
			printString("Ending test prematurely due to too many failures\n", quiet);
			break;
		}
	}
	result_t localResult;
	localResult.nrFailed = fails;
	localResult.nrTests = testNr + 1;

	addResults(result, &localResult);
}
