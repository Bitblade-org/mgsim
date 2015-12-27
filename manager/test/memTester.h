#ifndef MANAGER_MEMTESTER_H_
#define MANAGER_MEMTESTER_H_

#include <stdio.h>
#include <stdint.h>
#include <svp/abort.h>
#include <svp/delegate.h>  // voor get_current_place()
#include <svp/testoutput.h>

#include "mgsim.h"

#include "tests.h"
#include "../defines.h"
#include "../pagetable.h"
#include "../PTBuilder.h"
#include "../syscall.h"

sl_decl(memTester,, sl_shparm(sl_place_t, syscall_gateway));

void createPageEntries(int contextId, void* ptsPBase, pt_t** next_table, size_t* free);
void writeStartingData();

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


#endif /* MANAGER_MEMTESTER_H_ */
