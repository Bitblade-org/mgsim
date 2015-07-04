#ifndef DEMO_MM_PAGETABLETYPES_H_
#define DEMO_MM_PAGETABLETYPES_H_

struct PTE;

struct PT{
	PTE pte[512];
};

union PTE{


	__attribute__((packed)) struct {
		uint64_t p		: 1;	// Present bit
		uint64_t t		: 1;	// Table bit. 0 for page, 1 for table.
		uint64_t content:62;	// Type-specific bits.
	};

	__attribute__((packed)) struct TablePointer{
		uint64_t p		: 1;	// Present bit
		uint64_t t		: 1;	// Table bit. Must be 1 for table pointer.
		uint64_t res	: 6;	// Reserved. Must be 0.
		uint64_t _ptr	:56;	// Address bits

		PT getTable();
	};

	__attribute__((packed)) struct PagePointer{
		uint64_t p		: 1;	// Present bit
		uint64_t t		: 1;	// Table bit. Must be 0 for page pointer.
		uint64_t r		: 1;	// R(ead) bit
		uint64_t w		: 1;	// W(rite) bit
		uint64_t x		: 1;	// X(ecute) bit
		uint64_t res	: 3;	// Reserved. Must be 0. //TODO Decide on destination bits, mmio, noc etc.
		uint64_t _ptr	: 56;	// Address bits

		void *getPtr();
	};
};


#endif /* DEMO_MM_PAGETABLETYPES_H_ */
