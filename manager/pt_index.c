#include "pt_index.h"

uint64_t calculate_pt_index(uint64_t context_id, uint64_t complete_vAddr){
	size_t context_width = CONTEXTID_WIDTH;
	uint64_t index = 0;

	//The LSB falling inside the LSO (Least Significant Offset) are ignored for creation
	//of the index since all entries in the PT will have at least this offset.
	complete_vAddr = complete_vAddr >> VADDR_LSO;

	//The LSB part of the index contains the virtual address (bits 0 - vaddr_width)
	index = complete_vAddr;

	//The MSB part of the index contains the context id
	context_id <<= VADDR_WIDTH - VADDR_LSO;
	index |= context_id;

	return index;
}

void print_pt_index(uint64_t index){
	size_t vaddr_sections = PTI_VADDR_SECTIONS;
	size_t context_sections = PTI_CONTEXT_SECTIONS;

	for(int i=0; i<PTI_CONTEXT_SECTIONS; i++){
		for(int j=0; j<PT_INDEX_WIDTH; j++){
			putchar('C');
		}
		putchar(' ');
	}

	for(int i=0; i<PTI_VADDR_SECTIONS; i++){
		for(int j=0; j<PT_INDEX_WIDTH; j++){
			putchar('V');
		}
		putchar(' ');
	}

	putchar('\n');

	for(int i=PTS_INDEX_WIDTH-1; i>=0; i--){
		int val = (index >> i) & 1;
		if((i+1)%PT_INDEX_WIDTH == 0 && i < PTS_INDEX_WIDTH-1){
			putchar(' ');
		}
		putchar(val + '0');
	}
	putchar('\n');

	for(int i=PTI_SECTIONS-1; i >= 0; i--){
		uint64_t val = get_index_section(index, i);

		//uint64_t val = (index >> (PTI_SECTIONS - i - 1) * PT_INDEX_WIDTH);
		//val &= PT_INDEX_MASK;
		printf("%*d ", PT_INDEX_WIDTH, val);
	}

	putchar('\n');
}

uint64_t get_index_section(uint64_t index, unsigned section){
	index >>= PT_INDEX_WIDTH * section;
	index &= PT_INDEX_MASK;
	return index;
}
