#include "Table.h"

#include <cstdint>
#include <cstring>

#include <sim/config.h>
#include <sim/except.h>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <ctgmath>

namespace Simulator {
namespace drisc {
namespace mmu {

Line::Line(MWidth offsetWidth){

}


Table::Table(const std::string& name, Object& parent) :
		Object(name, parent), m_evictionStrategy(getEvictionStrategy(GetConf("EvictionStrategy", std::string))),
		m_numEntries(GetConf("NumberOfEntries", uint64_t)), //MLDTODO Refactor to numLines
		m_offsetWidth(GetConf("OffsetSize", size_t)),
		m_entries(m_numEntries,	Line(m_offsetWidth)),
		m_head(NULL),
		m_tail(NULL),
		m_indexWidth(std::log2(m_numEntries))
{

	if(m_numEntries == 0){
		throw exceptf<std::invalid_argument>("A table cannot be initialised with 0 lines");
	}

	if (m_evictionStrategy != EvictionStrategy::LRU) {
		if (((m_numEntries & (m_numEntries - 1)) != 0)) {
			throw exceptf<std::invalid_argument>("The number of entries in a table must be a factor of 2");
		}

		m_head = &(m_entries[0]);
		m_tail = &(m_entries[m_numEntries-1]);

		for(int i=1; i<m_numEntries; i++){
			m_entries[i-1].noEntry. .algData.lru.next = &(m_entries[i]);
			m_entries[i].algData.lru.previous = &(m_entries[i-1]);
		}
	}
}

Result Table::storePendingEntry(DTlbEntry &entry){}

////SUCCESS: Successfully stored.
////FAIL	 : Destination entry locked
//Result Table::storePendingEntry(DTlbEntry &entry) {
//	DTlbEntry *dest = find(entry.processId, entry.vAddr);
//
//	if (dest == NULL) {
//		dest = &pickDestination();
//		if (dest->locked) {
//			return Result::FAILED;
//		}
//		invalidate(*dest);
//
//		dest->processId = entry.processId;
//		dest->vAddr = entry.vAddr;
//	} else if (dest->locked) {
//		return Result::FAILED;
//	}
//
//	dest->pAddr = entry.pAddr;
//	dest->read = entry.read;
//	dest->write = entry.write;
//	dest->present = true;
//
//	setPrioHigh(*dest);
//
//	return Result::SUCCESS;
//}


//MLDTODO What to do when a locked entry matches an invalidation?
void Table::invalidate() {
	auto lambda = [](DTlbEntry &entry) {invalidate(entry);};
	std::for_each(m_entries.begin(), m_entries.end(), lambda);
}

void Table::invalidate(RPAddr processId) {
	for (DTlbEntry &entry : m_entries) {
		if (entry.processId == processId) {
			invalidate(entry);
		}
	}
}

void Table::invalidate(RPAddr processId, RMAddr vAddr) {
	DTlbEntry *entry = find(processId, vAddr);
	if (entry != NULL) {
		invalidate(*entry);
	}
}

void Table::invalidate(DTlbEntry &entry) {
	if(entry.locked == true){
		throw exceptf<std::domain_error>("Attempt to invalidate locked TLB line");
	}

	entry.present = false;
}


DTlbEntry* Table::find(RPAddr processId, RMAddr vAddr) {
	for (DTlbEntry &entry : m_entries) {
		if (entry.vAddr == vAddr && entry.processId == processId) {
			return &entry;
		}
	}
	return NULL;
}

void Table::setPrioHigh(DTlbEntry &entry) {
	if (this->m_evictionStrategy == EvictionStrategy::ACCESSED) {
		entry.algData.a.accessed = true;
	} else if (this->m_evictionStrategy == EvictionStrategy::LRU) {
		if(&entry != m_head){
			if(&entry == m_tail){
				m_tail = entry.algData.lru.previous;
				m_tail->algData.lru.next = NULL;
			}else{
				DTlbEntry *prev = entry.algData.lru.previous;
				DTlbEntry *next = entry.algData.lru.next;
				prev->algData.lru.next = next;
				next->algData.lru.previous = prev;
			}
			entry.algData.lru.previous = NULL;
			entry.algData.lru.next = m_head;
			m_head->algData.lru.previous = &entry;
			m_head = &entry;
		}
	}
}

DTlbEntry& Table::pickDestination() {
	DTlbEntry *dest = findFree();

	if (dest == NULL) {
		if (m_evictionStrategy == EvictionStrategy::PSEUDO_RANDOM) {
			dest = &pickVictim_random();
		} else if (m_evictionStrategy == EvictionStrategy::ACCESSED) {
			dest = &pickVictim_accessed();
		} else if (m_evictionStrategy == EvictionStrategy::LRU) {
			dest = &pickVictim_lru();
		}
	}

	return *dest;
}

DTlbEntry& Table::pickVictim_random() {
	MAddr index = 0;
	MAddr mask = ((~MAddr(0)) - 1);
	for (unsigned i = 0; i < m_numEntries; i++) {
		DTlbEntry entry = m_entries.at(i);

		index |= (entry.pAddr.m_value & mask) << i;
	}
	return m_entries.at(index);
}

DTlbEntry& Table::pickVictim_accessed() {
	auto lambda =
			[](DTlbEntry &entry) {return (entry.algData.a.accessed == true);};
	auto res = std::find_if(m_entries.begin(), m_entries.end(), lambda);
	if (res == m_entries.end()) {
		return pickVictim_random();
	} else {
		return *res;
	}
}

DTlbEntry& Table::pickVictim_lru() {
	return *m_tail;
}

DTlbEntry* Table::findFree() {
	auto lambda = [](DTlbEntry &entry) {return (entry.present == false);};
	auto res = std::find_if(m_entries.begin(), m_entries.end(), lambda);
	if (res == m_entries.end()) {
		return NULL;
	} else {
		return &(*res);
	}
}

void Table::Cmd_Info(std::ostream& out,
		const std::vector<std::string>& /* arguments */) const {
	out << "The Table blablabla\n\n";
	out << "  Eviction strategy: " << m_evictionStrategy << "\n";
	out << "        Offset size: " << unsigned(m_offsetWidth) << " bytes\n";
	out << "    Number of lines: " << unsigned(m_numEntries) << "\n\n";
	out << "Supported operations:\n";
	out << "  None implemented yet!" << std::endl;

	//MLDTODO Display statistics
}

void Table::Cmd_Read(std::ostream& out,
		const std::vector<std::string>& /* arguments */) const {
	using namespace std;

	string algHeader = "";
	if (this->m_evictionStrategy == EvictionStrategy::ACCESSED) {
		algHeader = " | A";
	} else if (this->m_evictionStrategy == EvictionStrategy::LRU) {
		out << "Linked list head: ";
		printLruIndex(out, m_head);
		out << std::endl;
		algHeader = " | Prev | Next";
	}

	out
			<< "  # | Proc. ID |  Virtual Address   |  Physical Address  | R | W | P"
			<< algHeader << endl;
	for (unsigned i = 0; i < m_entries.size(); i++) {
		DTlbEntry entry = m_entries.at(i);
		out << noboolalpha << noshowbase << setfill(' ') << dec;
		out << setw(3) << i << " | ";
		out << showbase << setfill(' ') << hex;
		out << setw(8) << entry.processId.m_value << " | ";
		out << setw(18) << entry.vAddr.m_value << " | ";
		out << setw(18) << entry.pAddr.m_value << " | ";
		out << noshowbase << dec;
		out << setw(1) << entry.read << " | ";
		out << setw(1) << entry.write << " | ";
		out << setw(1) << entry.present;
		if (this->m_evictionStrategy == EvictionStrategy::ACCESSED) {
			out << " | " << setw(1) << entry.algData.a.accessed << endl;
		} else if (this->m_evictionStrategy == EvictionStrategy::LRU) {
			out << " | " << setw(4);
			printLruIndex(out, entry.algData.lru.previous);
			out << " | " << setw(4);
			printLruIndex(out, entry.algData.lru.next);
			out << endl;
		}
	}
}

void Table::printLruIndex(std::ostream& out, DTlbEntry *entry) const {
	if (entry == NULL) {
		out << "----";
		return;
	}

	for (unsigned i = 0; i < m_numEntries; i++) {
		if (entry == &(m_entries[i])) {
			out << i;
			return;
		}
	}

	out << "????";
}

EvictionStrategy getEvictionStrategy(std::string name) {
	if (name == "PSEUDO_RANDOM") {
		return EvictionStrategy::PSEUDO_RANDOM;
	}
	if (name == "ACCESSED") {
		return EvictionStrategy::ACCESSED;
	}
	if (name == "LRU") {
		return EvictionStrategy::LRU;
	}
	throw exceptf<SimulationException>("Unknown eviction strategy");
}

std::ostream& operator<<(std::ostream& os, EvictionStrategy strategy) {
	switch (strategy) {
	case EvictionStrategy::PSEUDO_RANDOM:
		os << "PSEUDO_RANDOM";
		break;
	case EvictionStrategy::ACCESSED:
		os << "ACCESSED";
		break;
	case EvictionStrategy::LRU:
		os << "LRU";
		break;
	default:
		os.setstate(std::ios_base::failbit);
	}
	return os;
}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
