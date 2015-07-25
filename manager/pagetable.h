#ifndef MANAGER_PAGETABLE_H_
#define MANAGER_PAGETABLE_H_

#include <stddef.h>

#include "defines.h"

struct __attribute__((packed)) pte_tableptr{ //MLDTODO Use C packed equivalent
	uint64_t p		: 1;	// Present bit
	uint64_t t		: 1;	// Table bit. 0 for page, 1 for table.
	uint64_t _res   :10;	// Type-specific bits.
	uint64_t addr	:52;	// Address bits
};

struct __attribute__((packed)) pte_pageptr{ //MLDTODO Use C packed equivalent
	uint64_t p		: 1;	// Present bit
	uint64_t t		: 1;	// Table bit. Must be 0 for page pointer.
	uint64_t r		: 1;	// R(ead) bit
	uint64_t w		: 1;	// W(rite) bit
	uint64_t x		: 1;	// X(ecute) bit
	uint64_t _res	: 7;	// Reserved. Must be 0. //MLDTODO Decide on destination bits, mmio, noc etc.
	uint64_t addr	: 52;	// Address bits
};

struct __attribute__((packed)) pte_base{
	uint64_t p		: 1;	// Present bit
	uint64_t t		: 1;	// Table bit. 0 for page, 1 for table.
	uint64_t _res   :10;	// Type-specific bits.
	uint64_t addr	:52;	// Address bits
};

typedef union {
	struct pte_base		base;
	struct pte_tableptr table_p;
	struct pte_pageptr 	page_p;
} pte_t;

void* getPointer(pte_t* pte);

typedef struct {
	pte_t entries[512];
} pt_t;


/*
 * Will not test if p=1
 *///MLDTODO Really....
void* getPointer(pte_t* pte){ return (void*)(uint64_t)(pte->base.addr << ((uint64_t)PT_LSB_OFFSET)); }



#endif /* MANAGER_PAGETABLE_H_ */
