#ifndef ARCH_DRISC_MMU_TABLE_H_
#define ARCH_DRISC_MMU_TABLE_H_

#include <stddef.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <functional>
#include <iomanip>

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
	struct{
		bool 	accessed;
	} a;

	struct{
		Line	*previous;
		Line 	*next;
	} lru;
};

//INDEXABLE is a special type meaning !FREE
enum class LineType : unsigned char {FREE=0, NORMAL=1, PENDING=2, N_LOCKED=3, INDEXABLE=4};

union Line{
	Line() = delete;
	Line(const MWidth offsetWidth);
	bool is(const LineType cmp){return is(NULL, NULL, cmp);};
	bool is(const RPAddr *processId, const RMAddr *vAddr, const LineType cmp);

	struct {
		LineType type : 2;
	} type;

	struct {
		bool	present;
		bool	locked;
		Prio 	prio;
	} base;

	struct {
		bool	present;
		bool 	locked;
		Prio 	prio;
		RPAddr	processId;
		RMAddr	vAddr;
		bool	read;
		bool	write;
		RMAddr	pAddr;
	} normal;

	struct {
		bool	present;
		bool 	locked;
		Prio 	prio;
		RPAddr	processId;
		RMAddr	vAddr;
		RMAddr	d$lineId;
	} pending;
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

	//friend class TLB;
	//MLDTODO Keep statistics
	//MLDTODO Destructor...
public:
    Table(const std::string& name, Object& parent); //Lets try this without a clock
    Table(const Table&) = delete; //MLDQUESTION Needed because I use pointers?

    MWidth getOffsetWidth() const {return m_offsetWidth;}
    MWidth getVAddrWidth() const {return RMAddr::VirtWidth - m_offsetWidth;}

    Line *find(RPAddr processId, RMAddr vAddr, LineType type);

    Result storePending(RPAddr processId, RMAddr vAddr, RMAddr d$LineId, MAddr &tableLineId);

    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void Cmd_Read(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void operator=(Table&) = delete; //MLDQUESTION Needed because I use pointers?
private:
    Result storeNormal(RMAddr tableLineId, bool read, bool write, RMAddr pAddr, RMAddr &d$LineId);

    Line *find(const LineType type);
    Line *find(std::function<bool (Line&)> const&);
    MAddr getIndex(const Line &line) const;

    void setPrioHigh(Line &entry);

    Line& pickDestination();
    Line& pickVictim_random();
    Line& pickVictim_accessed();
    Line& pickVictim_lru();

    //MLDTODO What to do when a locked entry matches an invalidation?
	void freeLines();
	void freeLines(const RPAddr &processId, const RMAddr *vAddr);
	void freeLine(Line &line);

   	EvictionStrategy 	m_evictionStrategy;
   	unsigned int 		m_numLines;
   	MWidth 				m_offsetWidth;
   	std::vector<Line>	m_lines;

   	//			     e2.prev	   e2.next
   	//				   MRU			 LRU
   	//			HEAD - [e1] - [e2] - [e3] - TAIL
   	Line 				*m_head;    	// For Least Recently Used eviction
   	Line				*m_tail;
   	MWidth				m_indexWidth;   // For Accessed / Pseudo-Random eviction

   	// Can't do this due to GCC bug PR60594
   	//std::function<Line&(const Table&)> f_pickVictim;
};

EvictionStrategy getEvictionStrategy(const std::string name);
std::ostream& operator<<(std::ostream& os, EvictionStrategy strategy);

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_TABLE_H_ */
