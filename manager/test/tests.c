#include "tests.h"

#include <svp/abort.h>

void run(char abort){
	runAll(NULL, abort);
}

void runAll(result_t* result, char abort){
	svp_abort();
	runTest(&testQuickPageAlignedRW, "Small Page Aligned RW", result, abort, 1);
	svp_abort();
	runTest(&testNotLineAlignedR, "Not line aligned R", result, abort, 1);
	runTest(&testNotLineAlignedRW, "Not line aligned RW", result, abort, 1);
	runTest(&testLineBoundariesR, "Line boundaries R", result, abort, 1);
	runTest(&testLineBoundariesRW, "Line boundaries RW", result, abort, 1);
	runTest(&testUnalignedR, "Unaligned R", result, abort, 1);
	runTest(&testUnalignedRW, "Unaligned RW", result, abort, 1);
	runTest(&testSmallPageBoundariesR, "Page boundaries of smallest page R", result, abort, 1);
	runTest(&testSmallPageBoundariesRW, "Page boundaries of smallest page RW", result, abort, 1);
	runTest(&testMediumPageBoundariesRW, "Page boundaries of medium page RW", result, abort, 1);
	runTest(&testNotSmallPageAligned64RW, "Not page aligned RW (small page, 64 bits)", result, abort, 1);
	runTest(&testNotMediumPageAligned4096RW, "Not page aligned RW (medium page, 64 bits per 4096 bytes)", result, abort, 1);
	runTest(&testNotLargePageAligned2MRW, "Not page aligned RW (large page, 64 bits per 2MiB)", result, abort, 1);
	runTest(&testSyscall, "Syscall test", result, abort, 1);
}

void runTest(test_t test, char name[], result_t* result, char abort, char quiet){
	result_t localResult;
	resetResults(&localResult);

	printTestStart(quiet, name);

	test(&localResult, abort, quiet);

	printTestEnd(&localResult, quiet, name);

	if(localResult.nrFailed > 0 && quiet > 0){
		printString("Repeating test in verbose mode: **Note: Results may vary!**\n", 0);
		result_t dummyResult;
		resetResults(&dummyResult);
		test(&dummyResult, 0, 0);
	}

	addResults(result, &localResult);
}

void resetResults(result_t* result){
	result->nrFailed = 0;
	result->nrTests = 0;
}

void addSResult(result_t* destination, char result){
	if(destination != NULL){
		destination->nrTests += 1;
		destination->nrFailed += result;
	}
}

void addResults(result_t* destination, result_t* other){
	if(destination != NULL){
		destination->nrTests += other->nrTests;
		destination->nrFailed += other->nrFailed;
	}
}

void printTestStart(char quiet, char name[]){
	if(quiet == 0){
		output_string("Starting test \"", 2);
		output_string(name, 2);
		output_string("\"\n", 2);
	}else{
		output_string("Running test \"", 2);
		output_string(name, 2);
		output_string("\"...\n", 2);
	}
}

void printTestEnd(result_t *result, char quiet, char name[]){
	if(quiet == 0){
		pad();
		output_string("End of test \"", 2);
		output_string(name, 2);
		output_string("\": ", 2);
		output_uint(result->nrFailed, 2);
		output_string(" out of ", 2);
		output_uint(result->nrTests, 2);
		output_string(" failed\n", 2);
	}else{
		output_string("\033[1A", 2);
		if(result->nrFailed == 0){
			output_string(" [ ", 2);
			output_string("\033[1;32m", 2);
			output_string("SUCCESS", 2);
			output_string("\033[0m", 2);
			output_string(" ]  \"", 2);
		}else{
			output_string(" [ ", 2);
			output_string("\033[1;31m", 2);
			output_string("FAILURE", 2);
			output_string("\033[0m", 2);
			output_string(" ]  \"", 2);
		}
		output_string(name, 2);
		output_string("\": ", 2);
		output_uint(result->nrTests - result->nrFailed, 2);
		output_string(" / ", 2);
		output_uint(result->nrTests, 2);
		output_char('\n', 2);
	}
}

void pad(){
	output_string("              ", 2);
}

void printString(char text[], char quiet){
	if(quiet == 0){
		pad(0);
		output_string(text, 2);
	}
}


