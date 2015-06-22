#ifndef ARCH_DRISC_MMU_TABLE_H_
#define ARCH_DRISC_MMU_TABLE_H_

#include <stddef.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <sim/inspect.h>
#include <sim/kernel.h>
#include <sim/config.h>
#include <arch/simtypes.h>

namespace Simulator {
namespace drisc {
namespace mmu {

enum class EvictionStrategy {PSEUDO_RANDOM=0, ACCESSED=1, LRU=2};

struct DTlbEntry;

struct AlgData_A{
	bool 		accessed;
};

struct AlgData_LRU{
	DTlbEntry	*previous;
	DTlbEntry 	*next;
};

union AlgData{
	AlgData_A	a;
	AlgData_LRU	lru;
};

struct DTlbEntry{
	DTlbEntry() = delete;
	DTlbEntry(MWidth offsetWidth);
	PAddr 		processId;
	MAddr		vAddr;
	MAddr		pAddr;

	bool 		read;
	bool 		write;
	bool 		present;
	AlgData		algData;
};

//MLDTODO Not extending MMIOComponent, I consider MMIOComponent deprecated.
class Table : public Object, public Inspect::Interface<Inspect::Info | Inspect::Read>
{
	//MLDTODO Keep statistics
	//MLDTODO Destructor...
public:
    Table(const std::string& name, Object& parent); //Lets try this without a clock
//    Table(Table &&other):
//    	 m_evictionStrategy(other.m_evictionStrategy),
//		 m_numEntries(other.m_numEntries),
//		 m_offsetWidth(other.m_offsetWidth),
//		 m_entries(std::move(other.m_entries)),
//		 m_head(other.m_head),
//		 m_indexWidth(other.m_indexWidth){}
    //Table(Table&) = delete; //MLDQUESTION Needed because I use pointers?

    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void Cmd_Read(std::ostream& out, const std::vector<std::string>& arguments) const override;

    void store(DTlbEntry &entry);
    DTlbEntry* lookup(PAddr processId, MAddr vAddr);
    void invalidate();
    void invalidate(PAddr processId);
    void invalidate(PAddr processId, MAddr vAddr);

    MWidth getOffsetWidth(){return m_offsetWidth;}
    MWidth getVAddrWidth(){return MAddr::VAddrWidth - m_offsetWidth;}

    //void operator=(Table&) = delete; //MLDQUESTION Needed because I use pointers?
private:
    void invalidate(DTlbEntry &entry);
    DTlbEntry& pickDestination();
    DTlbEntry* pickEmpty();
    void prioritizeEntry(DTlbEntry &entry);
    DTlbEntry& pickVictim_random();
    DTlbEntry& pickVictim_accessed();
    DTlbEntry& pickVictim_lru();
    MAddr_base getIndex(DTlbEntry *entry) const;


   	EvictionStrategy 		m_evictionStrategy;
   	unsigned int 			m_numEntries;
   	MWidth 					m_offsetWidth;
   	std::vector<DTlbEntry>	m_entries;

   	// For Least Recently Used eviction
   	DTlbEntry 				*m_head;
   	// For Accessed / Pseudo-Random eviction
   	MWidth					m_indexWidth;
};

EvictionStrategy getEvictionStrategy(std::string name);
std::ostream& operator<<(std::ostream& os, EvictionStrategy strategy);

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_TABLE_H_ */
