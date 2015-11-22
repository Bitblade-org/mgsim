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
#include <cstdlib>

#include <sim/inspect.h>
#include <sim/kernel.h>
#include <sim/config.h>
#include <cli/argument.h>
#include <arch/simtypes.h>
#include <arch/Memory.h>
#include <arch/drisc/forward.h>


namespace Simulator {
namespace drisc {
namespace mmu {
//
enum class EvictionStrategy {PSEUDO_RANDOM=0, ACCESSED=1, LRU=2};

enum class LineTag : unsigned char {
	FREE		=0x1,
	PENDING		=0x2,
	NORMAL		=0x4,
	N_LOCKED	=0x8,
	INDEXABLE	=(PENDING | NORMAL  | N_LOCKED),
	PRESENT		=(NORMAL  | N_LOCKED),
	LOCKED		=(PENDING | N_LOCKED)
};

struct Line{
	Line(AddrWidth procWidth, AddrWidth vAddrWidth, AddrWidth pAddrWidth);
	bool is(const LineTag cmp);
	bool is(const RAddr *processId, const RAddr *vAddr, const LineTag cmp);
	bool	present;
	bool 	locked;

	bool 	accessed;
	Line	*previous;
	Line	*next;

	RAddr	processId;
	RAddr	vAddr;
	RAddr	pAddr;

	unsigned d$lineRef;

	bool	read;
	bool	write;
};

class Table : public Object, public Inspect::Interface<Inspect::Info | Inspect::Read | Inspect::Write>
{
	//Nomenclature: A (pending) entry is written to a free (= !p && !l) line.

	//MLDTODO Keep statistics
	/*
	 * - Number of hits
	 * - Number of misses
	 * 		- line ==, etc
	 * - Number of flushes
	 * - Number of PID invalidates
	 * - Number of vAddr+PID invalidates
	 */
public:
    Table(const std::string& name, Object& parent); //Lets try this without a clock
    Table(const Table&) = delete;
    ~Table(){} //Lines will be deleted by vector destructor.

    AddrWidth getOffsetWidth() const {return m_offsetWidth;}
    AddrWidth getVAddrWidth() const;
    AddrWidth getPAddrWidth() const;
    AddrWidth getIndexWidth() const {return m_indexWidth;}

Line *lookup(RAddr processId, RAddr vAddr, LineTag type);

    bool getPending(RAddr tableLineId, RAddr &processId, RAddr &vAddr, RAddr &d$LineId);
    void releasePending(RAddr tableLineId);
    bool storePending(RAddr processId, RAddr vAddr, Addr &tableLineId, Line* &line);
    Line* fillPending(RAddr tableLineId, bool read, bool write, RAddr pAddr, RAddr &d$LineId);
    Line* storeNormal(RAddr processId, RAddr vAddr, RAddr pAddr, bool read, bool write);

    //MLDTODO What to do when a locked entry matches an invalidation?
	void freeLines();
	void freeLines(const RAddr &processId, const RAddr *vAddr);

    void Cmd_Info (std::ostream& out, const std::vector<std::string>& arguments) const;
    void Cmd_Read (std::ostream& out, const std::vector<std::string>& arguments) const;
    void Cmd_Write(std::ostream& out, const std::vector<std::string>& arguments);
    void Cmd_Usage(std::ostream& out) const;
    void Cmd_Usage_write_line(std::ostream& out) const;

    void operator=(Table&) = delete;
private:
    void initStrategy();

    Line *find(const LineTag type);
    Line *find(std::function<bool (Line&)> const&);
    Line *find(RAddr processId, RAddr vAddr, LineTag type);
    Addr getIndex(const Line &line) const { return (Addr)(&line - &m_lines[0]); }

    void setPrioHigh(Line &entry);

    Line& pickDestination();
    Line& pickVictim_random();
    Line& pickVictim_accessed();
    Line& pickVictim_lru();

	void freeLine(Line &line);

   	AddrWidth 	m_offsetWidth;
   	AddrWidth	m_vWidth;
   	AddrWidth	m_pWidth;
   	size_t		m_numLines;

   	EvictionStrategy 	m_evictionStrategy;
   	std::vector<Line>	m_lines;

   	Line 		*m_head;    	// For Least Recently Used eviction
   	Line		*m_tail;		// [Head] [Head.next] [...] [Tail.prev] [Tail=LRU]
   	AddrWidth	m_indexWidth;	// For Accessed / Pseudo-Random eviction

   	//MLDNOTE Can't do this due to GCC bug PR60594
   	//std::function<Line&(const Table&)> f_pickVictim;

   	MMU&	getMMU() const { return (MMU&)*GetParent()->GetParent(); } //MLDTODO Could be defined for Object.h?
    Object& GetDRISCParent() const { return *GetParent()->GetParent()->GetParent(); } //MLDTODO Could be defined for Object.h?
};

EvictionStrategy getEvictionStrategy(const std::string name);
std::ostream& operator<<(std::ostream& os, EvictionStrategy strategy);

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_TABLE_H_ */
