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

union Line;

union Prio{
	struct a{
		bool 	accessed;
	};

	struct lru{
		Line	*previous;
		Line 	*next;
	};
};

union Line{
	Line() = delete;
	Line(MWidth offsetWidth);

	struct noEntry{
		Prio 	prio;
		bool	present;
		bool	locked;
	};

	struct normalEntry{
		Prio 	prio;
		bool	present;
		bool 	locked;
		RPAddr	processId;
		RMAddr	vAddr;
		bool	read;
		bool	write;
		RMAddr	pAddr;
	};

	struct pendingEntry{
		Prio 	prio;
		bool	present;
		bool 	locked;
		RPAddr	processId;
		RMAddr	vAddr;
		RMAddr	d$lineId;
	};
};

//struct DTlbEntry{ //MLDTODO Refactor to DTlbLine
//	DTlbEntry() = delete;
//	DTlbEntry(MWidth offsetWidth);
//	RPAddr 		processId;
//	RMAddr		vAddr;
//	RMAddr		pAddr;
//
//	bool 		read;
//	bool 		write;
//	bool 		present;
//	bool 		locked;
//	Prio		algData;
//};

//MLDTODO Not extending MMIOComponent, I consider MMIOComponent deprecated.
class Table : public Object, public Inspect::Interface<Inspect::Info | Inspect::Read>
{
	//Nomenclature: A (pending) entry is written to a free (= !p && !l) line.

	friend class TLB;
	//MLDTODO Keep statistics
	//MLDTODO Destructor...
public:
    Table(const std::string& name, Object& parent); //Lets try this without a clock
    Table(const Table&) = delete; //MLDQUESTION Needed because I use pointers?

    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void Cmd_Read(std::ostream& out, const std::vector<std::string>& arguments) const override;

    MWidth getOffsetWidth(){return m_offsetWidth;}
    MWidth getVAddrWidth(){return RMAddr::VirtWidth - m_offsetWidth;}

    Line *find(RPAddr processId, RMAddr vAddr);

    Result storePendingEntry(Line &entry);

    void operator=(Table&) = delete; //MLDQUESTION Needed because I use pointers?
private:
    Line *findFree();

    void setPrioHigh(Line &entry);

    void invalidate();
    void invalidate(RPAddr processId);
    void invalidate(RPAddr processId, RMAddr vAddr);
    void invalidate(Line &entry);

    Line& pickDestination();
    Line& pickVictim_random();
    Line& pickVictim_accessed();
    Line& pickVictim_lru();

    void printLruIndex(std::ostream& out, Line *entry) const;

   	EvictionStrategy 	m_evictionStrategy;
   	unsigned int 		m_numEntries;
   	MWidth 				m_offsetWidth;
   	std::vector<Line>	m_entries;

   	//			     e2.prev	   e2.next
   	//				   MRU			 LRU
   	//			HEAD - [e1] - [e2] - [e3] - TAIL
   	Line 				*m_head;    	// For Least Recently Used eviction
   	Line				*m_tail;
   	MWidth				m_indexWidth;   // For Accessed / Pseudo-Random eviction
};

EvictionStrategy getEvictionStrategy(std::string name);
std::ostream& operator<<(std::ostream& os, EvictionStrategy strategy);

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_TABLE_H_ */
