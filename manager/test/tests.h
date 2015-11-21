#include "memTester.h"

#ifndef MANAGER_TEST_TESTS_H_
#define MANAGER_TEST_TESTS_H_

#define LINE_SIZE 		64
#define COUNT8R_START 	0x440000ul
#define SANDBOXR_START	0x500000ul
#define SANDBOXW_START	0x530000ul

typedef struct {
	int nrTests;
	int nrFailed;
} result_t;

typedef void (*test_t)(result_t *result, char abort, char quiet);

void run(char abort);
void runAll(result_t *result, char abort);
void runTest(test_t test, char name[], result_t* result, char abort, char quiet);
void printTestStart(char quiet, char name[]);
void printTestEnd(result_t *result, char quiet, char name[]);
void printString(char text[], char quiet);

void testSmallPageAlignedRW (result_t *result, char abort, char quiet);
void testNotLineAlignedR(result_t *result, char abort, char quiet);
void testNotLineAlignedRW(result_t *result, char abort, char quiet);
void testLineBoundariesR(result_t *result, char abort, char quiet);

void testAllPageSizes(result_t *result, char abort, char quiet);
void testPageBoundaries(result_t *result, char abort, char quiet);
void testTLBEntryUnlock(result_t *result, char abort, char quiet);
void testFullSet(result_t *result, char abort, char quiet);
void testFullTLB(result_t *result, char abort, char quiet);
void testNotPageAligned(result_t *result, char abort, char quiet);

void addSResult(result_t* destination, char result);
void addResults(result_t* destination, result_t* other);
void resetResults(result_t* result);

#endif /* MANAGER_TEST_TESTS_H_ */
