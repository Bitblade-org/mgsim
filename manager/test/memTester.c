#include "memTester.h"

sl_def(memTester,, sl_shparm(sl_place_t, syscall_gateway)) {
	sl_place_t syscall_gateway = sl_getp(syscall_gateway);
	syscall_target(&syscall_gateway);

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

    //Let the compiler know we care
	sl_setp(syscall_gateway, sl_getp(syscall_gateway));
}
sl_enddef

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
