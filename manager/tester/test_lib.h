#ifndef TESTER_TEST_LIB_H_
#define TESTER_TEST_LIB_H_

#include <stdint.h>

typedef struct {
	int nrTests;
	int nrFailed;
} result_t;

//typedef void (*test_t)(result_t *result, char abort, char quiet);

void printRead(uint64_t addr, uint64_t data, uint64_t expect, char failed);
char read8 (uint64_t addr, uint8_t expect, char abort, char quiet);
char read16(uint64_t addr, uint16_t expect, char abort, char quiet);
char read32(uint64_t addr, uint32_t expect, char abort, char quiet);
char read64(uint64_t addr, uint64_t expect, char abort, char quiet);

void printWrite(uint64_t addr, uint64_t data);
void write8 (uint64_t addr, uint8_t data, char quiet);
void write16(uint64_t addr, uint16_t data, char quiet);
void write32(uint64_t addr, uint32_t data, char quiet);
void write64(uint64_t addr, uint64_t data, char quiet);

void printTestStart(char quiet, char name[]);
void printTestEnd(result_t *result, char quiet, char name[]);
void printString(char text[], char quiet);
void pad();

void addSResult(result_t* destination, char result);
void addResults(result_t* destination, result_t* other);
void resetResults(result_t* result);


#endif /* TESTER_TEST_LIB_H_ */
