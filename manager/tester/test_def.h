/*
 * test_def.h
 *
 *  Created on: 27 Dec 2015
 *      Author: nijntje
 */

#ifndef TESTER_TEST_DEF_H_
#define TESTER_TEST_DEF_H_

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

#endif /* TESTER_TEST_DEF_H_ */
