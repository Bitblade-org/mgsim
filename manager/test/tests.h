#include "memTester.h"

//MLDTODO-DOC: It will be the responsibility of the OS to make sure a physical
//				memory location can only be mapped to a single page size.

#ifndef MANAGER_TEST_TESTS_H_
#define MANAGER_TEST_TESTS_H_

#define LINE_SIZE 		64
#define COUNT8R_START 	0x440000ul
#define COUNT64R_START	0x441000ul
#define S_SANDBOXR		0x500000ul
#define S_SANDBOXW		0x530000ul
#define S0_SANDBOXRW	0x520000ul
#define S0_SANDBOXR		0x510000ul
#define M_SANDBOXR		0x200000000ul
#define M_SANDBOXW		0x210000000ul
#define L_SANDBOXR		0x400000000ul
#define L_SANDBOXW		0x440000000ul
#define SMALL_PAGE_SIZE	0x1000ul
#define MEDIUM_PAGE_SIZE 2097152
#define LARGE_PAGE_SIZE 1073741824ul

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
void pad();

void testQuickPageAlignedRW (result_t *result, char abort, char quiet);
void testNotLineAlignedR(result_t *result, char abort, char quiet);
void testNotLineAlignedRW(result_t *result, char abort, char quiet);
void testLineBoundariesR(result_t *result, char abort, char quiet);
void testLineBoundariesRW(result_t *result, char abort, char quiet);
void testUnalignedR(result_t *result, char abort, char quiet);
void testUnalignedRW(result_t *result, char abort, char quiet);
void testSmallPageBoundariesR(result_t *result, char abort, char quiet);
void testSmallPageBoundariesRW(result_t *result, char abort, char quiet);
void testMediumPageBoundariesRW(result_t *result, char abort, char quiet);
void testNotSmallPageAligned64RW(result_t *result, char abort, char quiet);
void testNotMediumPageAligned4096RW(result_t *result, char abort, char quiet);
void testNotLargePageAligned2MRW(result_t *result, char abort, char quiet);



void testAllPageSizes(result_t *result, char abort, char quiet);
void testTLBEntryUnlock(result_t *result, char abort, char quiet);
void testFullSet(result_t *result, char abort, char quiet);
void testFullTLB(result_t *result, char abort, char quiet);

void addSResult(result_t* destination, char result);
void addResults(result_t* destination, result_t* other);
void resetResults(result_t* result);

#endif /* MANAGER_TEST_TESTS_H_ */
