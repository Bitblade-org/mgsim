#ifndef MANAGER_PT_INDEX_H_
#define MANAGER_PT_INDEX_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../defines.h"
#include "../debug.h"

#define PTI_VADDR_SECTIONS		((VADDR_WIDTH - VADDR_LSO) / PT_INDEX_WIDTH)
#define PTI_CONTEXT_SECTIONS	(CONTEXTID_WIDTH / PT_INDEX_WIDTH)
#define PTI_SECTIONS			(PTI_VADDR_SECTIONS + PTI_CONTEXT_SECTIONS)

uint64_t calculate_pt_index(uint64_t context_id, uint64_t vAddr);
void 	 print_pt_index(uint64_t index);
uint64_t get_index_section(uint64_t index, unsigned section);


#endif /* MANAGER_PT_INDEX_H_ */
