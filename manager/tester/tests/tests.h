#ifndef TESTER_TESTS_TESTS_H_
#define TESTER_TESTS_TESTS_H_

#include "../test_def.h"
#include "../test_lib.h"

//MLDTODO Aanpassingen voor gemak (dbg makefile, logging filter enz) ook in scriptie.

// make a type (http://dare.uva.nl/document/2/109511, pg80)
sl_typedef_fptr(testPtr_t,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));

sl_decl(testSyscall,, 					sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testSmallPageAlignedRW,, 		sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testNotLineAlignedR,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testNotLineAlignedRW,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testLineBoundariesR,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testLineBoundariesRW,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testUnalignedR,, 				sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testUnalignedRW,, 				sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testSmallPageBoundariesR,, 		sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testSmallPageBoundariesRW,, 	sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testMediumPageBoundariesRW,, 	sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testNotSmallPageAligned64RW,, 	sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testNotMediumPageAligned4096RW,,sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testNotLargePageAligned2MRW,, 	sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testAllPageSizes,, 				sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testTLBEntryUnlock,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testFullSet,, 					sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
sl_decl(testFullTLB,, 					sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));

testPtr_t p_testSyscall = (testPtr_t)testSyscall;
testPtr_t p_testSmallPageAlignedRW = (testPtr_t)testSmallPageAlignedRW;
testPtr_t p_testNotLineAlignedR = (testPtr_t)testNotLineAlignedR;
testPtr_t p_testNotLineAlignedRW = (testPtr_t)testNotLineAlignedRW;
testPtr_t p_testLineBoundariesR = (testPtr_t)testLineBoundariesR;
testPtr_t p_testLineBoundariesRW = (testPtr_t)testLineBoundariesRW;
testPtr_t p_testUnalignedR = (testPtr_t)testUnalignedR;
testPtr_t p_testUnalignedRW = (testPtr_t)testUnalignedRW;
testPtr_t p_testSmallPageBoundariesR = (testPtr_t)testSmallPageBoundariesR;
testPtr_t p_testSmallPageBoundariesRW = (testPtr_t)testSmallPageBoundariesRW;
testPtr_t p_testMediumPageBoundariesRW = (testPtr_t)testMediumPageBoundariesRW;
testPtr_t p_testNotSmallPageAligned64RW = (testPtr_t)testNotSmallPageAligned64RW;
testPtr_t p_testNotMediumPageAligned4096RW = (testPtr_t)testNotMediumPageAligned4096RW;
testPtr_t p_testNotLargePageAligned2MRW = (testPtr_t)testNotLargePageAligned2MRW;
testPtr_t p_testAllPageSizes = (testPtr_t)testAllPageSizes;
testPtr_t p_testTLBEntryUnlock = (testPtr_t)testTLBEntryUnlock;
testPtr_t p_testFullSet = (testPtr_t)testFullSet;
testPtr_t p_testFullTLB = (testPtr_t)testFullTLB;

#endif /* TESTER_TESTS_TESTS_H_ */
