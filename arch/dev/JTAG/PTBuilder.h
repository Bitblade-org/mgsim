#ifndef ARCH_DEV_JTAG_PTBUILDER_H_
#define ARCH_DEV_JTAG_PTBUILDER_H_

#include "pageTableTypes.h"
#include <cstring>

#define PT_MIN_OFFSET 12

namespace Simulator{

struct PT;
union PTE{
	PTE(){memset(this, 0, 8);}
	~PTE();

	//PTE's can only be allocated as part of a PT
    static void* operator new(size_t sz) = delete;
    static void* operator new[](std::size_t sz) = delete;

	RAddr getAddr(){return RAddr(addr, 52) << PT_MIN_OFFSET;}
	void* getPointer();
	PT* replaceLinkedTable(PT* newPtr = NULL);
	void setPagePointer(void* ptr);

	struct __attribute__((packed)) {
		uint64_t p		: 1;	// Present bit
		uint64_t t		: 1;	// Table bit. 0 for page, 1 for table.
		uint64_t res    :10;	// Type-specific bits.
		uint64_t addr	:52;	// Address bits
	};

	struct __attribute__((packed)) {
		uint64_t p		: 1;	// Present bit
		uint64_t t		: 1;	// Table bit. Must be 1 for table pointer.
		uint64_t res	:10;	// Reserved. Must be 0.
		uint64_t addr   :52;	// Address bits
	} TablePointer;

	struct __attribute__((packed)){
		uint64_t p		: 1;	// Present bit
		uint64_t t		: 1;	// Table bit. Must be 0 for page pointer.
		uint64_t r		: 1;	// R(ead) bit
		uint64_t w		: 1;	// W(rite) bit
		uint64_t x		: 1;	// X(ecute) bit
		uint64_t res	: 7;	// Reserved. Must be 0. //MLDTODO Decide on destination bits, mmio, noc etc.
		uint64_t addr	: 52;	// Address bits
	} PagePointer;
};

struct PT{
	PT(){};
    static void* operator new(size_t sz);
    static void* operator new[](std::size_t sz);

	PTE pte[512];
};


class PTBuilder {
public:
	PTBuilder() : m_start(new PT){}
	~PTBuilder() { delete m_start; }
	PTBuilder(const PTBuilder&) = delete;

	void* getStart(){ return m_start ;}

	void insert(Addr procId, Addr vAddr, Addr pAddr, bool r, bool w, bool x, unsigned lvl = 5);
	void insert(RAddr procId, RAddr vAddr, RAddr pAddr, bool r, bool w, bool x, unsigned lvl = 5);
	void demoBuild();

	PTBuilder& operator=(const PTBuilder&) = delete;
private:
	PT* m_start;

};

} /* namespace Simulator */

#endif /* ARCH_DEV_JTAG_PTBUILDER_H_ */
