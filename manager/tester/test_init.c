#include "test_init.h"

#include <stddef.h>
#include <stdint.h>

#include "../ptLib/pagetable.h"
#include "../ptLib/pt_index.h"
#include "../ptLib/PTBuilder.h"
#include "test_def.h"

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
	index = calculate_pt_index(contextId, COUNT8R_START);
	write_entry(ptsPBase, index,   (void*)0x440000ul, 0, next_table, free, 1, 0, 0); //COUNT8
	index = calculate_pt_index(contextId, COUNT64R_START);
	write_entry(ptsPBase, index,   (void*)0x441000ul, 0, next_table, free, 1, 0, 0); //COUNT64-1
	index = calculate_pt_index(contextId, 0x442000ul);
	write_entry(ptsPBase, index,   (void*)0x442000ul, 0, next_table, free, 1, 0, 0); //COUNT64-2
	index = calculate_pt_index(contextId, 0x443000ul);
	write_entry(ptsPBase, index,   (void*)0x443000ul, 0, next_table, free, 1, 0, 0); //COUNT64-3
	index = calculate_pt_index(contextId, ADDR64_START);
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

	//Fill 0x44 00 00 - 0x44 08 00
	//with bytes containing the positive numbers (0-255) in sequence, rolling over where applicable.
	uint64_t* ptr = (uint64_t*)COUNT8R_START;
	for(int i=0; i<256; i++, ptr++){
		uint64_t buffer = 0;
		for(int j=0; j<8; j++){
			buffer |= (uint64_t)((i*8+j) % 256) << (8*(j));
		}
		*ptr = buffer;
	}


	//Fill 0x44 10 00 - 0x44 11 00 with words representing 0, 1, 2, ..., 31
	//Fill 0x44 1e 00 - 0x44 21 00 with words representing 480, 481, ,,, 543 (512 +/- 32)
	//Fill 0x44 2e 00 - 0x44 31 00 with words representing 992, 993, .., 1056 (1024 +/- 32)
	//Fill 0x44 3e 00 - 0x44 00 00 with words representing 1504, 1505, ..,  1535
	uint64_t* ptrStart = (uint64_t*)COUNT64R_START;
	ptr = ptrStart;
	for(int i=0; i<1536; i++, ptr++){
		if(i ==    0 + 32){ i= 512 - 32; ptr = ptrStart + i; }
		if(i ==  512 + 32){ i=1024 - 32; ptr = ptrStart + i; }
		if(i == 1024 + 32){ i=1536 - 32; ptr = ptrStart + i; }
		*ptr = i;
	}

	//Fill 0x44 40 00 - 0x44 48 00 with words representing 255, 254, ..., 0
	ptr = (uint64_t*)ADDR64_START;
	for(int i=255; i>=0; i--, ptr++){
		*ptr = i;//(uint64_t)ptr;
	}
}


