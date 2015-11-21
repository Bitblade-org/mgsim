#include "memTester.h"

sl_def(memTester, , ) {
    sl_index(i);
    unsigned pid = get_current_place();
    unsigned core_id = get_core_id();

    output_string("MemTester (thread ", 2);
    output_uint((unsigned int)i, 2);
    output_string(") now running on core ", 2);
    output_uint(core_id, 2);
    output_string(", place_id ", 2);
    output_hex(pid, 2);
    output_char('\n', 2);
    output_char('\n', 2);
    output_char('\n', 2);

    run(0);
}
sl_enddef

// Space reserved for testing:
//	0x440000 - 0x550000 inclusive
// Smallest page = 4KB = 0x440000 : 0x440FFF
void createPageEntries(int contextId, void* ptsPBase, pt_t** next_table, size_t* free){
	uint64_t index;
	index = calculate_pt_index(contextId, 0x440000ul);
	write_entry(ptsPBase, index, (void*)0x440000ul, 0, next_table, free, 1, 0, 0);
	index = calculate_pt_index(contextId, 0x441000ul);
	write_entry(ptsPBase, index, (void*)0x441000ul, 0, next_table, free, 1, 0, 0);
	index = calculate_pt_index(contextId, 0x442000ul);
	write_entry(ptsPBase, index, (void*)0x442000ul, 0, next_table, free, 1, 0, 0);
	index = calculate_pt_index(contextId, 0x443000ul);
	write_entry(ptsPBase, index, (void*)0x443000ul, 0, next_table, free, 1, 0, 0);
	index = calculate_pt_index(contextId, 0x444000ul);
	write_entry(ptsPBase, index, (void*)0x444000ul, 0, next_table, free, 1, 0, 0);

	index = calculate_pt_index(contextId, 0x500000ul);
	write_entry(ptsPBase, index, (void*)0x500000ul, 0, next_table, free, 1, 1, 1);

	index = calculate_pt_index(contextId, 0x510000ul);
	write_entry(ptsPBase, index, (void*)0x500000ul, 0, next_table, free, 1, 0, 0);

	index = calculate_pt_index(contextId, 0x520000ul);
	write_entry(ptsPBase, index, (void*)0x500000ul, 0, next_table, free, 1, 0, 0);

	index = calculate_pt_index(contextId, 0x530000ul);
	write_entry(ptsPBase, index, (void*)0x500000ul, 0, next_table, free, 0, 1, 0);
}

// Space reserved for testing:
//	0x440000 - 0x550000 inclusive
void writeStartingData(){
	for(int i=0; i<12288; i++){
		uint8_t* ptr = ((uint8_t*)0x440000ul + i);
		*ptr = i % 256;
	}

	uint64_t* mem = (uint64_t*)0x500000ul;
	*mem = 0x12345ul;
}

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
	output_string("W: 0x", 2);
	output_hex(addr, 2);
	output_string(" << 0x", 2);
	output_hex(data, 2);
	output_char('\n', 2);
}
