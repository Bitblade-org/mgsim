unsigned int __attribute__((optimize("O0"))) clock_asm0(){

	register unsigned int cycles;
	register unsigned int a;
	register unsigned int b;

	asm volatile(".align 6");
	asm volatile("ldq %0,168($31)" : "=r"(a)); 						// Core cycle counter > a
	asm volatile("ldq %0,168($31)" : "=r"(b));						// Core cycle counter > b
	asm volatile("subq %2,%1,%0" : "=rm"(cycles) : "r"(a), "r"(b)); // cycles = b - a


	return cycles;
}

unsigned int __attribute__((optimize("O0"))) clock_asm1(){

	register unsigned int cycles;
	register unsigned int a;
	register unsigned int b;

	asm volatile(".align 6");
	asm volatile("ldq %0,168($31)" : "=r"(a)); 						// Core cycle counter > a
	asm volatile("nop;");
	asm volatile("ldq %0,168($31)" : "=r"(b));						// Core cycle counter > b
	asm volatile("subq %2,%1,%0" : "=rm"(cycles) : "r"(a), "r"(b)); // cycles = b - a


	return cycles;
}

unsigned int __attribute__((optimize("O0"))) clock_asm2(){

	register unsigned int cycles;
	register unsigned int a;
	register unsigned int b;

	asm volatile(".align 6");
	asm volatile("ldq %0,168($31)" : "=r"(a)); 						// Core cycle counter > a
	asm volatile("nop; nop;");
	asm volatile("ldq %0,168($31)" : "=r"(b));						// Core cycle counter > b
	asm volatile("subq %2,%1,%0" : "=rm"(cycles) : "r"(a), "r"(b)); // cycles = b - a


	return cycles;
}

unsigned int __attribute__((optimize("O0"))) clock_asmc(){

	register unsigned int cycles;
	register unsigned int a;
	register unsigned int b;

	asm volatile(".align 6");
	asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
	asm volatile("ldq %0,168($31)" : "=r"(a)); 						// Core cycle counter > a
	asm volatile("ldq %0,168($31)" : "=r"(b));						// Core cycle counter > b
	asm volatile("subq %2,%1,%0" : "=rm"(cycles) : "r"(a), "r"(b)); // cycles = b - a


	return cycles;
}

