#include <stdint.h>
#include <stdio.h>
#include "timedRead.h"

#define VARIANT2


void __attribute__((optimize("O0"))) timedRead_asm(timedReadData_t* data){
//	uint64_t expect = data->input.expectation;
//	uint64_t* ptr = (uint64_t*)data->input.address;
//	asm("stq $31, 648($31);");
//	uint64_t current = *ptr;
//	asm("stq $31, 648($31);");

//	printf("TimedRead_asm will be using 0x%p for input, "
//			"hopefully leaving 0x%p alone!, reading from address 0x%X (%p), "
//			"expecting to find %d / 0x%X. The address currently contains %d / 0x%X\n",
//			data,
//			&expect, data->input.address, ptr,
//			data->input.expectation, data->input.expectation,
//			current, current);

#if defined(VARIANT1)
	asm volatile("" ::: "memory");
	asm volatile(
			"ldq $11, 0(%0);"		/* MEM[%0] to $11 */
			"ldq $12, 8(%0);"		/* Expected value to $12 */
			"wmb; mb;"
			".align 6;"
			"ldq $13, 168($31);"	/* Core cycle count to $13 */
			"ldq $11, 0($11);"		/* MEM[$11] to $11 */
			"xor $11, $12, $12;"	/* xor $11, $12 --> $12 */
			"ldq $14, 168($31);"	/* Core cycle count to $14 */
			"subq $14, $13, $13;"	/* $14 - $13 ---> $13 */
			"nop; nop;"
			"stq $11, 536($31);"	/* Print $11 */
			"nop; nop;"
			"stq $11, 0(%0);"		/* $11 to MEM[%0] */
			"stq $12, 8(%0);"		/* $12 to MEM[%0 + 8] */
			"stq $13, 16(%0);"		/* $13 to MEM[%0 + 16] */
			"wmb; mb;"
			: /* none */
			: "r"(data)
			: "$11", "$12", "$13", "$14", "memory"
	);
	asm volatile("" ::: "memory");

#elif defined(VARIANT2)
	uint64_t dummy0;
	uint64_t dummy1;
	uint64_t dummy2;
	uint64_t dummy3;

	asm volatile("" ::: "memory");
	asm volatile(
			"ldq %0, 0(%4);"		/* MEM[%4] to %0 */
			"ldq %1, 8(%4);"		/* Expected value to %1 */
			"mb;"
			"wmb;"
			".align 6;"
			"ldq %2, 168($31);"		/* Core cycle count to %2 */
			"ldq %0, 0(%0);"		/* MEM[%0] to %0 */
			"xor %0, %1, %1;"		/* xor %0, %1 --> %1 */
			"ldq %3, 168($31);"		/* Core cycle count to %3 */
			"subq %3, %2, %2;"		/* %3 - %2 ---> %2 */
			"nop;"
			"nop;"
		/*	"stq %0, 536($31);"		Print %0 */
			"nop;"
			"nop;"
			"stq %0, 0(%4);"		/* %0 to MEM[%4] */
			"stq %1, 8(%4);"		/* %1 to MEM[%4 + 8] */
			"stq %2, 16(%4);"		/* %2 to MEM[%4 + 16] */
			"mb;"
			"wmb;"
			: "=&r"(dummy0), "=&r"(dummy1), "=&r"(dummy2), "=&r"(dummy3)
			: "r"(data)
			: "memory"
	);
	asm volatile("" ::: "memory");



#endif

	if(data->output.xor_result != 0){
//		printf("Timed read returned %u / 0x%X instead of expected value %u / 0x%X\n\n", data[0], data[0], expect, expect);
		svp_abort();
	}
}


void __attribute__((optimize("O0"))) timedRead_asm_calibrationRun(timedReadData_t* data){
	uint64_t expect = data->input.expectation;

	uint64_t dummy0;
	uint64_t dummy1;
	uint64_t dummy2;
	uint64_t dummy3;

	asm volatile("" ::: "memory");
	asm volatile(
			"ldq %0, 0(%4);"		/* MEM[%4] to %0 */
			"ldq %1, 8(%4);"		/* Expected value to %1 */
			"mb;"
			"wmb;"
			".align 6;"
			"ldq %2, 168($31);"		/* Core cycle count to %2 */
			"ldq %0, 0(%0);"		/* MEM[%0] to %0 */
			"xor %0, %1, %1;"		/* xor %0, %1 --> %1 */
			"ldq %3, 168($31);"		/* Core cycle count to %3 */
			"subq %3, %2, %2;"		/* %3 - %2 ---> %2 */
			"nop;"
			"nop;"
		/*	"stq %0, 536($31);"		Print %0 */
			"nop;"
			"nop;"
			"stq %0, 0(%4);"		/* %0 to MEM[%4] */
			"stq %1, 8(%4);"		/* %1 to MEM[%4 + 8] */
			"stq %2, 16(%4);"		/* %2 to MEM[%4 + 16] */
			"mb;"
			"wmb;"
			: "=&r"(dummy0), "=&r"(dummy1), "=&r"(dummy2), "=&r"(dummy3)
			: "r"(data)
			: "memory"
	);
	asm volatile("" ::: "memory");

	if(data->output.xor_result != 0){
		printf("Timed read returned %u / 0x%X instead of expected value %u / 0x%X\n\n", data[0], data[0], expect, expect);
	}
}
