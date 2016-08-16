#include "pagetable.h"

inline void* get_pointer(pte_t* entry){
	uint64_t ptrValue = (uint64_t)entry->addr;
	return (void*)(ptrValue << VADDR_LSO);
}

inline void set_pointer(pte_t* entry, void* pointer){
	entry->addr = ((uint64_t)pointer) >> VADDR_LSO;
}
