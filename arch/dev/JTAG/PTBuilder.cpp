#include "PTBuilder.h"

#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <cstring>

#include "../../Address.h"

namespace Simulator{

PTE::~PTE(){
	if(p && t){
		delete (PT*)getPointer();
	}
}

static void* PT::operator new(size_t sz){
	assert(sz == 4096);
	void* ptr = aligned_alloc(4096, 4096);
	memset(ptr, 0, 4096);
	return ptr;
}

static void* PT::operator new[](std::size_t sz){
	void* ptr = aligned_alloc(4096, sz);
	memset(ptr, 0, sz);
	return ptr;
}

void* PTE::getPointer(){
	return (void*)((uint64_t)addr << PT_MIN_OFFSET);
}

// Will (recursively) free the old pointer!
PT* PTE::replaceLinkedTable(PT* newPtr){
	PT* oldPtr = (PT*)getPointer();
	if(oldPtr != NULL){
		oldPtr->~PT();
	}

	p=t=1;

	if(newPtr == NULL){
		newPtr = new PT;
	}
	uint64_t ptrAddr = (uint64_t)newPtr;
	ptrAddr >>= PT_MIN_OFFSET;
	addr = ptrAddr;

	return newPtr;
}

void PTE::setPagePointer(void* ptr){
	p=1;
	t=0;
	addr = (uint64_t)ptr;
}

void PTBuilder::insert(Addr procId, Addr vAddr, Addr pAddr, bool r, bool w, bool x, unsigned lvl){
	RAddr r_procId = RAddr(procId, 16);
	RAddr r_vAddr = RAddr(vAddr, 52);
	RAddr r_pAddr = RAddr(pAddr, 48);
	insert(r_procId, r_vAddr, r_pAddr, r, w, x, lvl);
}

void PTBuilder::insert(RAddr procId, RAddr vAddr, RAddr pAddr, bool r, bool w, bool x, unsigned lvl){
	unsigned index[6];

	assert(lvl > 2);

	index[0] = procId[{9,15}].m_value;
	index[1] = procId[{0, 8}].m_value;
	index[2] = vAddr[{39,47}].m_value;
	index[3] = vAddr[{30,38}].m_value;
	index[4] = vAddr[{21,29}].m_value;
	index[5] = vAddr[{12,20}].m_value;


	PT* cur = m_start;
	for(unsigned i=0; i<lvl; i++){
		if(cur->pte[index[i]].p){
			cur = (PT*)cur->pte[index[i]].getPointer();
		}else{
			cur = cur->pte[index[i]].replaceLinkedTable();
		}
	}

	cur->pte[index[lvl]].setPagePointer(pAddr.ptr<PT>());
	cur->pte[index[lvl]].PagePointer.r = r;
	cur->pte[index[lvl]].PagePointer.w = w;
	cur->pte[index[lvl]].PagePointer.x = x;
}

/*
 * 		PID			vAddr			pAddr			flags
 * 		0x42		0x4200|000		0x84|000		!RWX!
 */
void PTBuilder::demoBuild(){
	//LVL5 16      36+12           40+12                            0         0         127       506-511   ------------
	//LVL5 0xFFFF  0xFFFFFFFFF000  0xFFFFFFFFFF000                  111111111 111111111 111111111 111111111 000000000000
	insert(0xFFFF, 0x00000FFFF000, 0x0000000084000, 0, 0, 0, 5); // 000000000 000000000 001111111 111111111 000000000000
	insert(0xFFFF, 0x00000FFFE000, 0x0000000085000, 0, 0, 1, 5); // 000000000 000000000 001111111 111111110 000000000000
	insert(0xFFFF, 0x00000FFFD000, 0x0000000086000, 0, 1, 0, 5); // 000000000 000000000 001111111 111111101 000000000000
	insert(0xFFFF, 0x00000FFFC000, 0x0000000087000, 0, 1, 1, 5); // 000000000 000000000 001111111 111111100 000000000000
	insert(0xFFFF, 0x00000FFFB000, 0x0000000088000, 1, 0, 0, 5); // 000000000 000000000 001111111 111111011 000000000000
	insert(0xFFFF, 0x00000FFFA000, 0x0000000089000, 1, 0, 1, 5); // 000000000 000000000 001111111 111111010 000000000000

	//LVL4 0xFFFF  27+21           31+21                            0         0         33|35     --------- ------------
	//LVL4 0xFFFF  0xFFFFFFE00000  0xFFFFFFFE00000                  111111111 111111111 111111111 000000000 000000000000
	insert(0xFFFF, 0x000004200000, 0x0000004200000, 1, 1, 0, 4); // 000000000 000000000 000100001 000000000 000000000000
	insert(0xFFFF, 0x000004600000, 0x0000004400000, 1, 1, 1, 4); // 000000000 000000000 000100011 000000000 000000000000

	//LVL3 0xFFFF  18+30           22+30                            511       511       --------- --------- ------------
	//LVL3 0xFFFF  0xFFFFC0000000  0xFFFFFC0000000                  111111111 111111111 000000000 000000000 000000000000
	insert(0xFFFF, 0xFFFFC0000000, 0xAAAAA00000000, 1, 1, 1, 3); // 111111111 111111111 000000000 000000000 000000000000
/*
																	0         0         33|35     --------- -----------
																	0         0         127       506-511   ------------
																	511       511       --------- --------- ------------




	*/


}

} /* namespace Simulator */
