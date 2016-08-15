

unsigned int timedRead_asm(unsigned int target, register unsigned int expect){

	register unsigned int cycles;
	register unsigned int val;
	register unsigned int a;
	register unsigned int b;

	asm volatile(".align 6");
	asm volatile("ldq %0,168($31)" : "=r"(a)                ); 		// Core cycle counter > a
	asm volatile("ldq %0,0(%1)"    : "=r"(val) : "r"(target));

	asm volatile("xor %1,%2,%0" : "=r"(val) : "r"(val), "r"(expect));

	asm volatile("ldq %0,168($31)" : "=r"(b));						// Core cycle counter > b
	asm volatile("subq %2,%1,%0" : "=rm"(cycles) : "r"(a), "r"(b)); // cycles = b - a

	if(val != 0){ return -1; }

	return cycles - 4;
}
