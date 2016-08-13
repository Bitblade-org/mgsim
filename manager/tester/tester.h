#include <svp/delegate.h>

#include "test_lib.h"
#include "tests/tests.h"

//MLDTODO-DOC: It will be the responsibility of the OS to make sure a physical
//				memory location can only be mapped to a single page size.

#ifndef MANAGER_TEST_TESTS_H_
#define MANAGER_TEST_TESTS_H_


void run(char abort);
void runAll(result_t *result, sl_place_t dst, char abort);
void runTest(testPtr_t test, sl_place_t dst, char name[], result_t* result, char abort, char quiet);

#endif /* MANAGER_TEST_TESTS_H_ */
