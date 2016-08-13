#include "test_init.h"

#include <stddef.h>
#include <stdint.h>

#include "../ptLib/pagetable.h"
#include "../ptLib/pt_index.h"
#include "../ptLib/PTBuilder.h"

/**
 * Method to initialise the test environment.
 * Run this method on a core _WITHOUT_ tlb enabled.
 */
void tester_init(int contextId, void* ptsPBase, pt_t** next_table, size_t* free){
	createPageEntries(contextId, ptsPBase, next_table, free);
	writeStartingData();
}

// Space reserved for testing small pages (4KiB):
//	0x440000 - 0x550000 inclusive
// Smallest page = 4KB = 0x440000 : 0x440FFF
//
// Space reserved for testing medium pages (2MiB)
// V: 0x2 0000 0000 - 0x3 0000 0000
// P: 0x3 0000 0000 - 0x4 0000 0000
// larger page = 2MiB = 0x2 0000 0000 : 0x2 0020 0000
//
// Space reserved for testing large pages (1GiB)
// V: 0x4 0000 0000 - 0x5 0000 0000
// P: 0x5 0000 0000 - 0x6 0000 0000
// larger page = 1GiB = 0x4 0000 0000 : 0x4 4000 0000
void createPageEntries(int contextId, void* ptsPBase, pt_t** next_table, size_t* free){
	uint64_t index;
	index = calculate_pt_index(contextId, 0x440000ul);
	write_entry(ptsPBase, index,   (void*)0x440000ul, 0, next_table, free, 1, 0, 0); //COUNT8
	index = calculate_pt_index(contextId, 0x441000ul);
	write_entry(ptsPBase, index,   (void*)0x441000ul, 0, next_table, free, 1, 0, 0); //COUNT64-1
	index = calculate_pt_index(contextId, 0x442000ul);
	write_entry(ptsPBase, index,   (void*)0x442000ul, 0, next_table, free, 1, 0, 0); //COUNT64-2
	index = calculate_pt_index(contextId, 0x443000ul);
	write_entry(ptsPBase, index,   (void*)0x443000ul, 0, next_table, free, 1, 0, 0); //COUNT64-3
	index = calculate_pt_index(contextId, 0x444000ul);
	write_entry(ptsPBase, index,   (void*)0x444000ul, 0, next_table, free, 1, 0, 0); //unused

	index = calculate_pt_index(contextId, 0x500000ul);
	write_entry(ptsPBase, index,   (void*)0x500000ul, 0, next_table, free, 1, 1, 1); //SANDBOX-R0
	index = calculate_pt_index(contextId, 0x501000ul);
	write_entry(ptsPBase, index,   (void*)0x501000ul, 0, next_table, free, 1, 1, 1); //SANDBOX-R1
	index = calculate_pt_index(contextId, 0x502000ul);
	write_entry(ptsPBase, index,   (void*)0x502000ul, 0, next_table, free, 1, 1, 1); //SANDBOX-R2
	index = calculate_pt_index(contextId, 0x503000ul);
	write_entry(ptsPBase, index,   (void*)0x503000ul, 0, next_table, free, 1, 1, 1); //SANDBOX-R3
	index = calculate_pt_index(contextId, 0x504000ul);
	write_entry(ptsPBase, index,   (void*)0x504000ul, 0, next_table, free, 1, 1, 1); //SANDBOX-R4

	index = calculate_pt_index(contextId, 0x510000ul);
	write_entry(ptsPBase, index,   (void*)0x500000ul, 0, next_table, free, 1, 0, 0); //SANDBOX-R0-alias

	index = calculate_pt_index(contextId, 0x520000ul);
	write_entry(ptsPBase, index,   (void*)0x500000ul, 0, next_table, free, 1, 1, 0); //SANDBOX-RW0-alias

	index = calculate_pt_index(contextId, 0x530000ul);
	write_entry(ptsPBase, index,   (void*)0x500000ul, 0, next_table, free, 0, 1, 0); //SANDBOX-W0
	index = calculate_pt_index(contextId, 0x531000ul);
	write_entry(ptsPBase, index,   (void*)0x501000ul, 0, next_table, free, 0, 1, 0); //SANDBOX-W0
	index = calculate_pt_index(contextId, 0x532000ul);
	write_entry(ptsPBase, index,   (void*)0x502000ul, 0, next_table, free, 0, 1, 0); //SANDBOX-W0
	index = calculate_pt_index(contextId, 0x533000ul);
	write_entry(ptsPBase, index,   (void*)0x503000ul, 0, next_table, free, 0, 1, 0); //SANDBOX-W0
	index = calculate_pt_index(contextId, 0x534000ul);
	write_entry(ptsPBase, index,   (void*)0x504000ul, 0, next_table, free, 0, 1, 0); //SANDBOX-W0

	uint64_t mod;

	uint64_t baseV = 0x200000000ul;
	uint64_t baseP = 0x300000000ul;
	mod = 0;         index = calculate_pt_index(contextId, baseV + mod);
	write_entry(ptsPBase, index, (void*)baseP + mod, 1, next_table, free, 1, 0, 0); //M-SANDBOX-R0
	mod += 0x200000; index = calculate_pt_index(contextId, baseV + mod);
	write_entry(ptsPBase, index, (void*)baseP + mod, 1, next_table, free, 1, 0, 0); //M-SANDBOX-R1
	mod += 0x200000; index = calculate_pt_index(contextId, baseV + mod);
	write_entry(ptsPBase, index, (void*)baseP + mod, 1, next_table, free, 1, 0, 0); //M-SANDBOX-R2
	mod += 0x200000; index = calculate_pt_index(contextId, baseV + mod);
	write_entry(ptsPBase, index, (void*)baseP + mod, 1, next_table, free, 1, 0, 0); //M-SANDBOX-R3
	mod += 0x200000; index = calculate_pt_index(contextId, baseV + mod);
	write_entry(ptsPBase, index, (void*)baseP + mod, 1, next_table, free, 1, 0, 0); //M-SANDBOX-R4

	baseV = 0x210000000ul;
	baseP = 0x300000000ul;
	mod = 0;       	 index = calculate_pt_index(contextId, baseV + mod);
	write_entry(ptsPBase, index, (void*)baseP + mod, 1, next_table, free, 0, 1, 0); //M-SANDBOX-W0
	mod += 0x200000; index = calculate_pt_index(contextId, baseV + mod);
	write_entry(ptsPBase, index, (void*)baseP + mod, 1, next_table, free, 0, 1, 0); //M-SANDBOX-W1
	mod += 0x200000; index = calculate_pt_index(contextId, baseV + mod);
	write_entry(ptsPBase, index, (void*)baseP + mod, 1, next_table, free, 0, 1, 0); //M-SANDBOX-W2
	mod += 0x200000; index = calculate_pt_index(contextId, baseV + mod);
	write_entry(ptsPBase, index, (void*)baseP + mod, 1, next_table, free, 0, 1, 0); //M-SANDBOX-W3
	mod += 0x200000; index = calculate_pt_index(contextId, baseV + mod);
	write_entry(ptsPBase, index, (void*)baseP + mod, 1, next_table, free, 0, 1, 0); //M-SANDBOX-W4

	baseV = 0x400000000ul;
	baseP = 0x500000000ul;
	index = calculate_pt_index(contextId, baseV);
	write_entry(ptsPBase, index, (void*)baseP, 2, next_table, free, 1, 0, 0); //L-SANDBOX-R

	baseV = 0x440000000ul;
	baseP = 0x500000000ul;
	index = calculate_pt_index(contextId, baseV);
	write_entry(ptsPBase, index, (void*)baseP, 2, next_table, free, 0, 1, 0); //L-SANDBOX-W
}

// Space reserved for testing:
//	0x440000 - 0x550000 inclusive
void writeStartingData(){
	for(int i=0; i<192; i++){
		uint8_t* ptr = ((uint8_t*)0x440000ul + i);
		*ptr = i % 256;
	}

	uint64_t* ptr = (uint64_t*)0x441000ul;
	for(int i=0; i<1536; i++){
		*ptr = i;
		ptr += 1;
	}
}


