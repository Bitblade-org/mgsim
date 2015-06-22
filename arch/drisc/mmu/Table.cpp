#include "Table.h"

#include <cstdint>
#include <cstring>

#include <sim/config.h>
#include <sim/except.h>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <ctgmath>


namespace Simulator{
namespace drisc{
namespace mmu{

Table::Table(const std::string& name, Object& parent)
	: Object(name, parent),
	m_evictionStrategy (getEvictionStrategy(GetConf("EvictionStrategy", std::string))),
	m_numEntries (GetConf("NumberOfEntries", uint64_t)),
	m_offsetWidth (GetConf("OffsetSize", size_t)),
	m_entries(m_numEntries, DTlbEntry(m_offsetWidth)),
	m_head(NULL),
	m_indexWidth(std::log2(m_numEntries))
{
	if(m_evictionStrategy != EvictionStrategy::LRU){
		if( ((m_numEntries & (m_numEntries - 1)) != 0) ){
			//MLDTODO Throw exception!!!!!!!
			std::cerr << "NumEntries is not a power of 2!!!";
		}
	}
}

//MLDTODO Initialise algData
DTlbEntry::DTlbEntry(MWidth offsetWidth):
	processId(0),
	vAddr(MAddr(0, MAddr::VAddrWidth - offsetWidth)),
	pAddr(MAddr(0, MAddr::PAddrWidth - offsetWidth)),
	read(0),
	write(0),
	present(0)
{}

void Table::invalidate(){
	auto lambda = [](DTlbEntry &entry){entry.present = false;};
	std::for_each(m_entries.begin(), m_entries.end(), lambda);
	m_head = NULL; //For LRU
}

void Table::invalidate(PAddr processId){
	for(DTlbEntry &entry : m_entries){
		if(entry.processId == processId){
			invalidate(entry);
		}
	}
}

void Table::invalidate(PAddr processId, MAddr vAddr){
	DTlbEntry *entry = lookup(processId, vAddr);
	if(entry != NULL){
		invalidate(*entry);
	}
}

void Table::invalidate(DTlbEntry &entry){
	if(entry.present == false){return;}
	entry.present = false;

	if(this->m_evictionStrategy == EvictionStrategy::LRU){
		if(entry.algData.lru.previous == NULL){
			if(entry.algData.lru.next == NULL){
				m_head = NULL;
			}else{
				m_head = entry.algData.lru.next;
				m_head->algData.lru.previous = NULL;
			}
		}else if(entry.algData.lru.next == NULL){
			entry.algData.lru.previous->algData.lru.next = NULL;
		}else{
			DTlbEntry *prev = entry.algData.lru.previous;
			DTlbEntry *next = entry.algData.lru.next;
			prev->algData.lru.next = next;
			next->algData.lru.previous = prev;
		}
		entry.algData.lru.next = entry.algData.lru.previous = NULL;
	}
}

DTlbEntry* Table::lookup(PAddr processId, MAddr vAddr){
	for(DTlbEntry &entry : m_entries){
		if(entry.vAddr == vAddr && entry.processId == processId){
			return &entry;
		}
	}
	return NULL;
}

void Table::store(DTlbEntry &entry){
	std::cout << "Table::store" <<std::endl;
	//MLDTODO Have to do better than this...s
	DTlbEntry *dest = &pickDestination();
	dest->processId = entry.processId;
	dest->vAddr = entry.vAddr;
	dest->pAddr = entry.pAddr;
	dest->read = entry.read;
	dest->write = entry.write;
	dest->present = true;

	if(this->m_evictionStrategy == EvictionStrategy::ACCESSED){
		dest->algData.a.accessed = true;
	}else if(this->m_evictionStrategy == EvictionStrategy::LRU){
		dest->algData.lru.next = m_head;
		m_head->algData.lru.previous = &entry;
		m_head = &entry;
	}
}

void Table::prioritizeEntry(DTlbEntry &entry){
	if(this->m_evictionStrategy == EvictionStrategy::ACCESSED){
		entry.algData.a.accessed = true;
	}else if(this->m_evictionStrategy == EvictionStrategy::LRU){
		if(entry.algData.lru.previous != NULL){
			//Remove from list
			if(entry.algData.lru.next == NULL){
				entry.algData.lru.previous->algData.lru.next = NULL;
			}else{
				DTlbEntry *prev = entry.algData.lru.previous;
				DTlbEntry *next = entry.algData.lru.next;
				prev->algData.lru.next = next;
				next->algData.lru.previous = prev;
			}

			//Reinsert as head
			entry.algData.lru.next = m_head;
			entry.algData.lru.previous = NULL;
			m_head->algData.lru.previous = &entry;
			m_head = &entry;
		}
	}
}

DTlbEntry& Table::pickDestination(){
	DTlbEntry *dest = pickEmpty();
	if(dest == NULL){
		if(m_evictionStrategy == EvictionStrategy::PSEUDO_RANDOM){
			dest = &pickVictim_random();
		}else if(m_evictionStrategy == EvictionStrategy::ACCESSED){
			dest = &pickVictim_accessed();
		}else if(m_evictionStrategy == EvictionStrategy::LRU){
			dest = &pickVictim_lru();
		}
		invalidate(*dest);
	}

	return *dest; // Guaranteed not to be nullptr
}

DTlbEntry& Table::pickVictim_random(){
	MAddr_base index = 0;
	MAddr_base mask = ((~MAddr_base(0)) - 1);
	for(unsigned i=0; i<m_numEntries; i++){
		DTlbEntry entry = m_entries.at(i);

		index |= (entry.pAddr.m_value & mask) << i;
	}
	return m_entries.at(index);
}

DTlbEntry& Table::pickVictim_accessed(){
	auto lambda = [](DTlbEntry &entry){return (entry.algData.a.accessed == true);};
	auto res = std::find_if(m_entries.begin(), m_entries.end(), lambda);
	if(res == m_entries.end()){
		return pickVictim_random();
	}else{
		return *res;
	}
}

DTlbEntry& Table::pickVictim_lru(){
	return *m_head;
}

DTlbEntry* Table::pickEmpty(){
	auto lambda = [](DTlbEntry &entry){return (entry.present == false);};
	auto res = std::find_if(m_entries.begin(), m_entries.end(), lambda);
	if(res == m_entries.end()){
		return NULL;
	}else{
		return &(*res);
	}
}

MAddr_base Table::getIndex(DTlbEntry *entry) const{
	for(unsigned i=0; i<m_numEntries; i++){
		DTlbEntry other = m_entries.at(i);
		if(entry == &other){
			return i;
		}
	}
	return m_numEntries;
}


void Table::Cmd_Info(std::ostream& out, const std::vector<std::string>& /* arguments */) const{
    out << "The Table blablabla\n\n";
    out << "  Eviction strategy: " << m_evictionStrategy << "\n";
    out << "        Offset size: " << unsigned(m_offsetWidth) << " bytes\n";
    out << "    Number of lines: " << unsigned(m_numEntries) << "\n\n";
    out << "Supported operations:\n";
    out << "  None implemented yet!" << std::endl;

    //MLDTODO Display statistics
}

void Table::Cmd_Read(std::ostream& out, const std::vector<std::string>& /* arguments */) const{
	using namespace std;

	string algHeader = "";
	if(this->m_evictionStrategy == EvictionStrategy::ACCESSED){
		algHeader = " | A";
	}else if(this->m_evictionStrategy == EvictionStrategy::LRU){
		algHeader = " | Prev | Next";
	}

	out << "  # | Proc. ID |  Virtual Address   |  Physical Address  | R | W | P" << algHeader << endl;
	for(unsigned i=0; i<m_entries.size(); i++){
		DTlbEntry entry = m_entries.at(i);
		out << noboolalpha << noshowbase << setfill(' ') << dec;
		out << setw(3)  << i 						<< " | ";
		out << showbase << setfill('0') << hex;
		out << setw(8)  << entry.processId.m_value 	<< " | ";
		out << setw(18) << entry.vAddr.m_value 		<< " | ";
		out << setw(18) << entry.pAddr.m_value 		<< " | ";
		out << noshowbase << dec;
		out << setw(1) 	<< entry.read				<< " | ";
		out << setw(1) 	<< entry.write				<< " | ";
		out << setw(1) 	<< entry.present;
		if(this->m_evictionStrategy == EvictionStrategy::ACCESSED){
			out << " | " << setw(1) << entry.algData.a.accessed << endl;
		}else if(this->m_evictionStrategy == EvictionStrategy::LRU){
			unsigned index = getIndex(entry.algData.lru.previous);
			if(index == m_numEntries){
				out << " | " << "????";
			}else{
				out << " | " << setw(4) << index;
			}

			index = getIndex(entry.algData.lru.next);
			if(index == m_numEntries){
				out << " | " << "????" << endl;
			}else{
				out << " | " << setw(4) << index << endl;
			}
		}
	}
}

EvictionStrategy getEvictionStrategy(std::string name){
	if(name == "PSEUDO_RANDOM") {return EvictionStrategy::PSEUDO_RANDOM;}
	if(name == "ACCESSED") 		{return EvictionStrategy::ACCESSED;		}
	if(name == "LRU")			{return EvictionStrategy::LRU;			}
    throw exceptf<SimulationException>("Unknown eviction strategy");
}

std::ostream& operator<<(std::ostream& os, EvictionStrategy strategy)
{
	switch(strategy) {
         case EvictionStrategy::PSEUDO_RANDOM 	: os << "PSEUDO_RANDOM"; break;
         case EvictionStrategy::ACCESSED 		: os << "ACCESSED"; break;
         case EvictionStrategy::LRU  			: os << "LRU";  break;
         default : os.setstate(std::ios_base::failbit);
    }
    return os;
}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
