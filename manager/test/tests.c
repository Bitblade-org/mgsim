#include "tests.h"

void run(char abort){
	runAll(NULL, abort);
}

void runAll(result_t* result, char abort){

	runTest(&testSmallPageAlignedRW, "Small Page Aligned RW", result, abort, 1);
	runTest(&testNotLineAlignedR, "Not line aligned R", result, abort, 1);
	runTest(&testNotLineAlignedRW, "Not line aligned RW", result, abort, 0);
	runTest(&testLineBoundariesR, "Line boundaries R", result, abort, 1);
}

void runTest(test_t test, char name[], result_t* result, char abort, char quiet){
	result_t localResult;
	resetResults(&localResult);

	printTestStart(quiet, name);

	test(&localResult, abort, quiet);

	printTestEnd(&localResult, quiet, name);

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
		output_string("\n\nStarting test \"", 2);
		output_string(name, 2);
		output_string("\"\n\n", 2);
	}else{
		output_string(name, 2);
		output_string(": ", 2);
	}
}

void printTestEnd(result_t *result, char quiet, char name[]){
	if(quiet == 0){
		output_string("\n\nEnd of test \"", 2);
		output_string(name, 2);
		output_string("\": ", 2);
		output_uint(result->nrFailed, 2);
		output_string(" out of ", 2);
		output_uint(result->nrTests, 2);
		output_string(" failed\n", 2);
	}else{
		output_uint(result->nrTests - result->nrFailed, 2);
		output_string(" / ", 2);
		output_uint(result->nrTests, 2);
		if(result->nrFailed > 0){
			output_string(" !!!!! !!!!! !!!!! !!!!! !!!!! \n", 2);
		}else{
			output_string(" OK \n", 2);
		}
	}
}

void printString(char text[], char quiet){
	if(quiet == 0){
		output_string(text, 2);
	}
}


