#include "tests.h"

unsigned int __attribute__((optimize("O0"))) clock_asm0();
unsigned int __attribute__((optimize("O0"))) clock_asm1();
unsigned int __attribute__((optimize("O0"))) clock_asm2();
unsigned int __attribute__((optimize("O0"))) clock_asmc();


const char* testClock_name(){
	return "Alignment & IPC reference test";
}

int testClock_pre(tlbRef_t tlbReference, char quiet){
	return 0;
}

void testClock_run(sl_place_t destination, tlbRef_t tlbReference, result_t* result, char abort, char quiet){
	sl_create(,destination,,,,, (sl__exclusive, sl__force_wait),
			testClock,
			sl_sharg(result_t*, result, result),
			sl_sharg(char,, abort),
			sl_sharg(char,, quiet));
	sl_sync();
}

int testClock_post(tlbRef_t tlbReference, char quiet){ return 0; };

sl_def(testClock,, sl_shparm(result_t*, result), sl_shparm(char, abort), sl_shparm(char, quiet)){
	result_t* result = sl_getp(result);
	char abort = sl_getp(abort);
	char quiet = sl_getp(quiet);

	uint64_t data = 0;

//	$0 (v0) Used for expression evaluations and to hold the integer function results. Not preserved across procedure calls.
//	$1-$8 (t0-t7) Temporary registers used for expression evaluations. Not preserved across procedure calls.
//	$9-$14 (s0-s5) Saved registers. Preserved across procedure calls.
//	$15 or $fp (s6 or fp) Contains the frame pointer (if needed); otherwise, a saved register.
//	$16-$21 (a0-a5)	Used to pass the first six integer type arguments. Not preserved across procedure calls.
//	$22-$25 (t8-t11) Temporary registers used for expression evaluations. Not preserved across procedure calls.
//	$26 (ra) Contains the return address. Preserved across procedure calls.
//	$27 (pv or t12) Contains the procedure value and used for expression evaluation. Notpreserved across procedure calls.
//	$28 or $at (AT)	Reserved for the assembler. Not preserved across procedure calls.
//	$29 or $gp (gp)	Contains the global pointer. Not preserved across procedure calls.
//	$30 or $sp(sp) Contains the stack pointer. Preserved across procedure calls.
//	$31(zero) Always has the value 0.

	unsigned int cycles = clock_asm0();
	if(cycles == 4){
		addSResult(result, 0);
	}else{
		addSResult(result, 1);
		snprintf(result->resultText, 80, "Test 0 took %d clockcycles where 4 cycles was expected.", cycles);
	}

	cycles = clock_asm1();
	if(cycles == 5){
		addSResult(result, 0);
	}else{
		addSResult(result, 1);
		snprintf(result->resultText, 80, "Test 1 took %d clockcycles where 4 cycles was expected.", cycles);
	}

	cycles = clock_asm2();
	if(cycles == 6){
		addSResult(result, 0);
	}else{
		addSResult(result, 1);
		snprintf(result->resultText, 80, "Test 2 took %d clockcycles where 4 cycles was expected.", cycles);
	}

	cycles = clock_asmc();
	if(cycles == 28){
		addSResult(result, 0);
	}else{
		addSResult(result, 1);
		snprintf(result->resultText, 80, "Test c took %d clockcycles where 28 cycles was expected.", cycles);
	}


	sl_setp(result, result); //Every time we forget to do this, a compiler
	sl_setp(abort, abort);	 //somewhere on the world throws a null pointer
	sl_setp(quiet, quiet);	 //exception.
}
sl_enddef;


