
#ifndef TESTER_TESTS_TESTS_H_
#define TESTER_TESTS_TESTS_H_

#define DECLARE_TEST(NAME) \
	int NAME ## _pre(tlbRef_t tlbReference, char quiet); \
	void NAME ## _run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet); \
	int NAME ## _post(tlbRef_t tlbReference, char quiet); \
	const char* NAME ## _name(void); \
	sl_decl(NAME,,sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet))


#include "../test_def.h"
#include "../test_lib.h"
#include "../../tlbLib/tlbRef.h"
#include "../../tlbLib/tlbControl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <svp/abort.h>


//MLDTODO Aanpassingen voor gemak (dbg makefile, logging filter enz) ook in scriptie.

// make a type (http://dare.uva.nl/document/2/109511, pg80)
//sl_typedef_fptr(testPtr_t,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));

DECLARE_TEST(action);
DECLARE_TEST(trCal);
DECLARE_TEST(b_4k);
DECLARE_TEST(b_4k_p);
DECLARE_TEST(b_4k_t);
DECLARE_TEST(b_4k_d);
DECLARE_TEST(b_4k_pt);
DECLARE_TEST(b_4k_pd);
DECLARE_TEST(b_4k_td);
DECLARE_TEST(b_4k_ptd);

DECLARE_TEST(b_2m);
DECLARE_TEST(b_2m_p);
DECLARE_TEST(b_2m_t);
DECLARE_TEST(b_2m_d);
DECLARE_TEST(b_2m_pt);
DECLARE_TEST(b_2m_pd);
DECLARE_TEST(b_2m_td);
DECLARE_TEST(b_2m_ptd);

DECLARE_TEST(b_1g);
DECLARE_TEST(b_1g_p);
DECLARE_TEST(b_1g_t);
DECLARE_TEST(b_1g_d);
DECLARE_TEST(b_1g_pt);
DECLARE_TEST(b_1g_pd);
DECLARE_TEST(b_1g_td);
DECLARE_TEST(b_1g_ptd);


DECLARE_TEST(testClock);
DECLARE_TEST(testSyscall);
DECLARE_TEST(testSmallPageAlignedRW);
DECLARE_TEST(testNotLineAlignedR);
DECLARE_TEST(testNotLineAlignedRW);
DECLARE_TEST(testLineBoundariesR);
DECLARE_TEST(testLineBoundariesRW);
DECLARE_TEST(testUnalignedR);
DECLARE_TEST(testUnalignedRW);
DECLARE_TEST(testSmallPageBoundariesR);
DECLARE_TEST(testSmallPageBoundariesRW);
DECLARE_TEST(testMediumPageBoundariesRW);
DECLARE_TEST(testNotSmallPageAligned64RW);
DECLARE_TEST(testNotMediumPageAligned4096RW);
DECLARE_TEST(testNotLargePageAligned2MRW);
DECLARE_TEST(testAllPageSizes);
DECLARE_TEST(testTLBEntryUnlock);
DECLARE_TEST(testFullSet);
DECLARE_TEST(testFullTLB);


//sl_decl(testSyscall,, 					sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testSmallPageAlignedRW,, 		sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testNotLineAlignedR,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testNotLineAlignedRW,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testLineBoundariesR,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testLineBoundariesRW,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testUnalignedR,, 				sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testUnalignedRW,, 				sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testSmallPageBoundariesR,, 		sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testSmallPageBoundariesRW,, 	sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testMediumPageBoundariesRW,, 	sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testNotSmallPageAligned64RW,, 	sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testNotMediumPageAligned4096RW,,sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testNotLargePageAligned2MRW,, 	sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testAllPageSizes,, 				sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testTLBEntryUnlock,, 			sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testFullSet,, 					sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));
//sl_decl(testFullTLB,, 					sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet));


//testPtr_t p_testSyscall = (testPtr_t)testSyscall;
//testPtr_t p_testSmallPageAlignedRW = (testPtr_t)testSmallPageAlignedRW;
//testPtr_t p_testNotLineAlignedR = (testPtr_t)testNotLineAlignedR;
//testPtr_t p_testNotLineAlignedRW = (testPtr_t)testNotLineAlignedRW;
//testPtr_t p_testLineBoundariesR = (testPtr_t)testLineBoundariesR;
//testPtr_t p_testLineBoundariesRW = (testPtr_t)testLineBoundariesRW;
//testPtr_t p_testUnalignedR = (testPtr_t)testUnalignedR;
//testPtr_t p_testUnalignedRW = (testPtr_t)testUnalignedRW;
//testPtr_t p_testSmallPageBoundariesR = (testPtr_t)testSmallPageBoundariesR;
//testPtr_t p_testSmallPageBoundariesRW = (testPtr_t)testSmallPageBoundariesRW;
//testPtr_t p_testMediumPageBoundariesRW = (testPtr_t)testMediumPageBoundariesRW;
//testPtr_t p_testNotSmallPageAligned64RW = (testPtr_t)testNotSmallPageAligned64RW;
//testPtr_t p_testNotMediumPageAligned4096RW = (testPtr_t)testNotMediumPageAligned4096RW;
//testPtr_t p_testNotLargePageAligned2MRW = (testPtr_t)testNotLargePageAligned2MRW;
//testPtr_t p_testAllPageSizes = (testPtr_t)testAllPageSizes;
//testPtr_t p_testTLBEntryUnlock = (testPtr_t)testTLBEntryUnlock;
//testPtr_t p_testFullSet = (testPtr_t)testFullSet;
//testPtr_t p_testFullTLB = (testPtr_t)testFullTLB;

#endif /* TESTER_TESTS_TESTS_H_ */
