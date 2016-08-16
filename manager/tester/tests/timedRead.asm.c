#include <stdint.h>
#include <stdio.h>


unsigned int __attribute__((optimize("O0"))) timedRead_asm(uint64_t target, uint64_t expect){

	uint64_t data[3];

	data[0] = target;
	data[1] = expect;

//	uint64_t xor_result;
//	uint64_t read_result;
//	uint64_t clock_result;

//
//	volatile register uint64_t* r13 asm("$13");
//	volatile register uint64_t* r9 asm("$9") = 0;
//	volatile register uint64_t* r10 asm("$10") = 0;
//
//	*r13 = expect;

	asm volatile("" ::: "memory");
	asm volatile(
			"ldq $10, 0(%0);"
			"ldq $11, 8(%0);"
			".align 6;"
			"ldq $12, 168($31);"
			"ldq $10, 0($10);"
			"xor $10, $11, $11;"
			"ldq $13, 168($31);"
			"subq $13, $12, $12;"
			"nop; nop;"
		/*	"stq $10, 536($31);"	*/
			"nop; nop;"
			"stq $10, 0(%0);"
			"stq $11, 8(%0);"
			"stq $12, 16(%0);"
			: /* none */
			: "r"(&data)
	);
//	register unsigned int cycles;
//	register unsigned int val;
//	register unsigned int a;
//	register unsigned int b;
//
//	asm volatile(".align 6");
//	asm volatile("" ::: "memory");
//	asm volatile("ldq $8,168($31)" ::: "$8", "memory"); 								// Core cycle counter > a
//	asm volatile("ldq $10,0(%1)" ::: "r"(target) : "memory");				// Read from memory
//	asm volatile("xor %1,%2,%0"    : "=r" (expect) : "r"(val),     "r"(expect) : "memory");// Use register
//	asm volatile("ldq %0,168($31)" : "=r" (b) :: "memory");									// Core cycle counter > b
//	asm volatile("subq %2,%1,%0"   : "=rm"(cycles) : "r"(a),       "r"(b) : "memory"); 	// Cycles = b - a
//	asm volatile("nop; nop; nop; nop; nop; nop; nop;" ::: "memory");
//	asm volatile("" ::: "memory");

	if(data[1] != 0){
		printf("Timed read returned %u / 0x%X instead of expected value %u / 0x%X\n\n", data[0], data[0], expect, expect);
		return -1;
	}

//	if(expect != 0){
////		printf("Read returned %u / 0X%X\n", val, val);
//		return -1;
//	}

	return data[2] - 4;
}
