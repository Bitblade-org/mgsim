#include "test_lib.h"

#include <string.h>
#include <stddef.h>
#include <svp/abort.h>
#include <svp/testoutput.h>

char read8(uint64_t addr, uint8_t expect, char abort, char quiet){
	uint8_t data = *((uint8_t*)addr);
	char failed = (data != expect);
	if(quiet == 0){ printRead(addr, data, expect, failed); }

	if(failed == 1 && abort == 1){
		svp_abort();
	}

	return failed;
}

char read16(uint64_t addr, uint16_t expect, char abort, char quiet){
	uint16_t data = *((uint16_t*)addr);
	char failed = (data != expect);
	if(quiet == 0){ printRead(addr, data, expect, failed); }

	if(failed == 1 && abort == 1){
		svp_abort();
	}

	return failed;
}

char read32(uint64_t addr, uint32_t expect, char abort, char quiet){
	uint32_t data = *((uint32_t*)addr);
	char failed = (data != expect);
	if(quiet == 0){ printRead(addr, data, expect, failed); }

	if(failed == 1 && abort == 1){
		svp_abort();
	}

	return failed;
}

char read64(uint64_t addr, uint64_t expect, char abort, char quiet){
	uint64_t data = *((uint64_t*)addr);
	char failed = (data != expect);
	if(quiet == 0){ printRead(addr, data, expect, failed); }

	if(failed == 1 && abort == 1){
		svp_abort();
	}

	return failed;
}

void printRead(uint64_t addr, uint64_t data, uint64_t expect, char failed){
	pad();
	output_string("R: 0x", 2);
	output_hex(addr, 2);
	output_string(" == 0x", 2);
	output_hex(data, 2);

	if(failed == 0){
		output_string(" : OK\n", 2);
	}else{
		output_string(" : FAIL (0x", 2);
		output_hex(expect, 2);
		output_string(")\n", 2);
	}
}

void write8(uint64_t addr, uint8_t data, char quiet){
	*((uint8_t*)addr) = data;
	if(quiet == 0){ printWrite(addr, data); }
}

void write16(uint64_t addr, uint16_t data, char quiet){
	*((uint16_t*)addr) = data;
	if(quiet == 0){ printWrite(addr, data); }
}

void write32(uint64_t addr, uint32_t data, char quiet){
	*((uint32_t*)addr) = data;
	if(quiet == 0){ printWrite(addr, data); }
}

void write64(uint64_t addr, uint64_t data, char quiet){
	*((uint64_t*)addr) = data;
	if(quiet == 0){ printWrite(addr, data); }
}

void printWrite(uint64_t addr, uint64_t data){
	pad();
	output_string("W: 0x", 2);
	output_hex(addr, 2);
	output_string(" << 0x", 2);
	output_hex(data, 2);
	output_char('\n', 2);
}

void resetResults(result_t* result){
	result->nrFailed = 0;
	result->nrTests = 0;
	memset(result->resultText, 0, sizeof(result->resultText));
}

void addSResult(result_t* destination, char result){
	if(destination != NULL){
		destination->nrTests += 1;
		destination->nrFailed += result;
	}
}

void addResults(result_t* destination, result_t* other){
	if(destination != NULL){
		destination->nrTests += other->nrTests;
		destination->nrFailed += other->nrFailed;
	}
}

void printTestStart(char quiet, const char name[]){
	if(quiet == 0){
		output_string("Starting test \"", 2);
		output_string(name, 2);
		output_string("\"\n", 2);
	}else{
		output_string("Running test \"", 2);
		output_string(name, 2);
		output_string("\"...\n", 2);
	}
}

void printTestEnd(const result_t *result, char quiet, const char name[]){
	if(quiet == 0){
		pad();
		output_string("End of test \"", 2);
		output_string(name, 2);
		output_string("\": ", 2);
		output_uint(result->nrFailed, 2);
		output_string(" out of ", 2);
		output_uint(result->nrTests, 2);
		output_string(" failed\n", 2);
	}else{
		//output_string("\033[1A", 2);
		if(result->nrFailed == 0){
			output_string(" [ ", 2);
			output_string("\033[1;32m", 2);
			output_string("SUCCESS", 2);
			output_string("\033[0m", 2);
			output_string(" ]  \"", 2);
		}else{
			output_string(" [ ", 2);
			output_string("\033[1;31m", 2);
			output_string("FAILURE", 2);
			output_string("\033[0m", 2);
			output_string(" ]  \"", 2);
		}
		output_string(name, 2);
		output_string("\": ", 2);
		output_uint(result->nrTests - result->nrFailed, 2);
		output_string(" / ", 2);
		output_uint(result->nrTests, 2);
		if(result->resultText[0] != 0){
			output_string(" | ", 2);
			output_string(result->resultText, 2);
		}
		output_char('\n', 2);
	}
}

void pad(){
	output_string("              ", 2);
}

void printString(const char text[], char quiet){
	if(quiet == 0){
		pad(0);
		output_string(text, 2);
	}
}

void printUint(uint64_t val, char quiet){
	if(quiet == 0){
		pad(0);
		output_uint(val, 2);
	}
}

void printChar(char c, char quiet){
	if(quiet == 0){
		pad(0);
		output_char(c, 2);
	}
}
