#include "PTBuilder.h"

//MLDNOTE No forward declaration in header because it doesn't exactly do what it says on the tin. Should probably find a better name for it.
//Only accurate for powers of two!
unsigned quickLog2(unsigned power_of_two){
  unsigned res = 0;
  while(power_of_two >>= 1) { res++; }
  return res;
}

// Both pt0 and v_base have to be aligned to the page selected with INITIAL_RESERVE_LVL_OFFSET
int init_pt_set(pt_t* pt0, unsigned context_id, void *v_base, pt_t** next_table, size_t* free){
	//Store a pointer to a free location for the next table,
	//and the number of tables that can be created there (assuming we reserve 2 MiB)
	*next_table = pt0;
	*free = 1 << (VADDR_LSO + (INITIAL_RESERVE_LVL_OFFSET * PT_INDEX_WIDTH) - quickLog2(sizeof(pt_t)));

	//Construct L0 table
	construct_pt(next_table);
	*free -= 1;

	// Construct the pt_index for context_id and v_base
	uint64_t pts_index = calculate_pt_index(context_id, (uint64_t)v_base);

	//Reserve a 2MiB page at physical location p_base, mapped pt_index
	//Create new tables where needed, starting at next_table. (a 2MiB page has a 19-bit offset)
	return write_entry(pt0, pts_index, 1, next_table, free);
}

// If free==0, no new tables will be constructed.
int write_entry(pt_t* pt0, uint64_t pts_index, unsigned lvl_offset, pt_t** next_table, size_t *free){
	pt_t* current = pt0;
	for(int i=PTS_NR_LVL; i > lvl_offset; i--){
		unsigned pt_index = PT_INDEX_MASK & pts_index >> ((i-2) * PT_INDEX_WIDTH);
		pte_t* entry = &(current->entries[pt_index]);
		if(entry->p){
			current = get_pointer(entry);
		}else{
			if(*free == 0){	return 0; }
			current = construct_pt(next_table);
			entry->p = entry->t = 1;
			set_pointer(entry, current);
			*free -= 1;
		}
	}
	return 1;
}

uint64_t calculate_pt_index(uint64_t context_id, uint64_t vAddr){
	size_t context_width = CONTEXTID_WIDTH;
	size_t vaddr_width = VADDR_WIDTH;
	size_t vaddr_lso = VADDR_LSO;
	uint64_t index = 0;

	//The LSB falling inside the LSO (Least Significant Offset) are ignored for creation
	//of the index since all entries in the PT will have at least this offset.
	vaddr_width -= vaddr_lso;
	vAddr >>= vaddr_lso;

	//The LSB part of the index contains the virtual address (bits 0 - vaddr_width)
	index = vAddr;

	//The MSB part of the index contains the context id
	context_id <<= vaddr_width;
	index |= context_id;

	return index;
}

pt_t* construct_pt(pt_t** next_table){
	pt_t* loc = *next_table;
	printf("Constructing new PT at %p\n", *next_table);
	memset(*next_table, 0, sizeof(pt_t));
	*next_table += 1;
	return loc;
}
