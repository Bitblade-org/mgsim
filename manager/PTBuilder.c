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
	return write_entry(pt0, pts_index, pt0, 0, next_table, free);
}

// If free==0, no new tables will be constructed.
// page_size indicates the size of a page.
// For VADDR_LSO 12 and PT_INDEX_WIDTH 9, this means:
// page_size=0 --> 4KiB
// page_size=1 --> 2MiB
// page_size=2 --> 1GiB
//MLDTODO Rewrite with section nomenclature
int write_entry(pt_t* pt0, uint64_t pts_index, void* dst, unsigned page_size, pt_t** next_table, size_t *free){
	pt_t*  table = pt0;
	pte_t* entry;

	int min = page_size;
	int max = PTS_INDEX_WIDTH / PT_INDEX_WIDTH;

	for(int i=max-1; i>=min; i--){
		uint64_t pt_index = get_index_section(pts_index, i);
		entry = &(table->entries[pt_index]);
		if(i == min){
			if(entry->p){ return -32; }

			table = construct_pt(next_table);
			*free -= 1;

			entry->p = 1;
			entry->t = 0;
			set_pointer(entry, dst);
		}else if(entry->p){
			table = get_pointer(entry);
		}else{
			if(*free == 0){	return 0; }

			table = construct_pt(next_table);
			*free -= 1;

			entry->p = entry->t = 1;
			set_pointer(entry, table);
		}
	}
	return 1;
}

pt_t* construct_pt(pt_t** next_table){
	pt_t* loc = *next_table;
	memset(*next_table, 0, sizeof(pt_t));
	*next_table += 1;
	return loc;
}
