#include "Table.h"

namespace Simulator {
namespace drisc {
namespace mmu {


Table::Table(const std::string& name, Object& parent):
Object(name, parent),
m_evictionStrategy(getEvictionStrategy(GetConf("EvictionStrategy", std::string))),
m_numLines(GetConf("NumberOfEntries", uint64_t)), //MLDTODO Refactor to numLines
m_offsetWidth(GetConf("OffsetSize", size_t)),
m_lines(m_numLines,	Line(m_offsetWidth)),
m_head(NULL),
m_tail(NULL),
m_indexWidth(std::log2(m_numLines))
{
	if(m_numLines == 0){
		throw exceptf<std::invalid_argument>("A table cannot be initialised with 0 lines");
	}

	if (m_evictionStrategy == EvictionStrategy::LRU) {
		// Initially link all nodes ascending.
		m_head = &(m_lines[0]);
		m_tail = &(m_lines[m_numLines-1]);

		for(unsigned int i=1; i<m_numLines; i++){
			m_lines[i-1].base.prio.lru.next = &(m_lines[i]);
			m_lines[i].base.prio.lru.previous = &(m_lines[i-1]);
		}
	}else{
		if (((m_numLines & (m_numLines - 1)) != 0)) {
			throw exceptf<std::invalid_argument>("The number of entries in a table must be a factor of 2");
		}
	}
}

Line *Table::find(RPAddr processId, RMAddr vAddr, LineType type)
{
	auto lambda =
			[&processId, &vAddr, &type]
			(Line &line)
			{return line.is(&processId, &vAddr, type);};
	return find(lambda);
}

Line *Table::find(LineType type)
{
	auto lambda = [type](Line &line) {return line.is(type);};
	return find(lambda);
}

Line *Table::find(std::function<bool (Line&)> const &lambda)
{
	auto res = std::find_if(m_lines.begin(), m_lines.end(), lambda);
	if (res == m_lines.end()) {
		return NULL;
	} else {
		return &(*res);
	}
}

MAddr Table::getIndex(const Line &line) const{
	for(unsigned int i=0; i<m_numLines; i++){
		if(&(m_lines.at(i)) == &line){
			return i;
		}
	}
	throw exceptf<std::invalid_argument>("The line does not exist!");
}

//MLDTODO Does this function have to return a Result obj?
Result Table::storeNormal(RMAddr tableLineId, bool read, bool write, RMAddr pAddr, RMAddr &d$LineId)
{
	Line &line = m_lines.at(tableLineId.m_value);

	// Should be impossible anyway! Might as well assert!
	if(!line.is(LineType::PENDING)){
		return Result::FAILED;
	}

	d$LineId = line.pending.d$lineId;
	line.type.type = LineType::N_LOCKED; // Keeps the line locked
	line.normal.read = read;
	line.normal.write = write;
	line.normal.pAddr = pAddr;

	return Result::SUCCESS;
}

Result Table::storePending(RPAddr processId, RMAddr vAddr, RMAddr d$LineId, MAddr &tableLineId){
	Line dst = pickDestination();

	if(dst.base.locked){
		return Result::FAILED;
	}

	freeLine(dst);

	dst.type.type = LineType::PENDING;
	dst.pending.processId = processId;
	dst.pending.vAddr = vAddr;
	dst.pending.d$lineId = d$LineId;

	tableLineId = getIndex(dst);

	std::cout << "Calculated line index: " << tableLineId << std::endl;

	return Result::SUCCESS;
}

void Table::Cmd_Info(std::ostream& out, const std::vector<std::string>& /* arguments */) const {
	out << "The Table blablabla\n\n"; //MLDTODO Maybe add some meaningfull, inspiring and brilliant text
	out << "  Eviction strategy: " << m_evictionStrategy << "\n";
	out << "        Offset size: " << unsigned(m_offsetWidth) << " bytes\n";
	out << "    Number of lines: " << unsigned(m_numLines) << "\n\n";
	out << "Supported operations:\n";
	out << "  Some!" << std::endl; //MLDTODO Maybe add some meaningfull, inspiring and brilliant text

	//MLDTODO Display statistics
}

//MLDTODO Needs to be rewritten to account for new line types
void Table::Cmd_Read(std::ostream& out,	const std::vector<std::string>& /* arguments */) const {
	using namespace std;

	if(this->m_evictionStrategy == EvictionStrategy::LRU){
		out << "Linked list head: " << getIndex(*m_head) << endl;
		out << "Linked list tail: " << getIndex(*m_tail) << endl;
	}

	out	<< "  # | Proc. ID |  Virtual Address   |  Physical Address  | R | W | P";

	if (this->m_evictionStrategy == EvictionStrategy::ACCESSED) {
		out << " | A" << endl;
	} else if (this->m_evictionStrategy == EvictionStrategy::LRU) {
		out << " | Prev | Next" << endl;
	}

	for (unsigned i = 0; i < m_lines.size(); i++) {
		Line line = m_lines.at(i);
		out << noboolalpha << noshowbase << setfill(' ') << dec;
		out << setw(3) << i << " | ";
		out << showbase << setfill(' ') << hex;
		out << setw(8) << line.normal.processId.m_value << " | ";
		out << setw(18) << line.normal.vAddr.m_value << " | ";
		out << setw(18) << line.normal.pAddr.m_value << " | ";
		out << noshowbase << dec;
		out << setw(1) << line.normal.read << " | ";
		out << setw(1) << line.normal.write << " | ";
		out << setw(1) << line.normal.present;
		if (this->m_evictionStrategy == EvictionStrategy::ACCESSED) {
			out << " | " << setw(1) << line.base.prio.a.accessed << endl;
		} else if (this->m_evictionStrategy == EvictionStrategy::LRU) {
			out << " | " << setw(4);
			out << getIndex(*line.base.prio.lru.previous);
			out << " | " << setw(4);
			out << getIndex(*line.base.prio.lru.next);
			out << endl;
		}
	}
}

void Table::setPrioHigh(Line &line) {
	if (!line.base.present){ return; } //MLDTODO Correct check?

	if (this->m_evictionStrategy == EvictionStrategy::ACCESSED) {
		line.base.prio.a.accessed = true;
	} else if (this->m_evictionStrategy == EvictionStrategy::LRU) {
		if(&line != m_head){
			if(&line == m_tail){
				m_tail = line.base.prio.lru.previous;
				m_tail->base.prio.lru.next = NULL;
			}else{
				Line *prev = line.base.prio.lru.previous;
				Line *next = line.base.prio.lru.next;
				prev->base.prio.lru.next = next;
				next->base.prio.lru.previous = prev;
			}
			line.base.prio.lru.previous = NULL;
			line.base.prio.lru.next = m_head;
			m_head->base.prio.lru.previous = &line;
			m_head = &line;
		}
	}
}
Line& Table::pickDestination(){
	Line *dest = find(LineType::FREE);

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

Line& Table::pickVictim_random(){
	MAddr mask = m_numLines - 1;
	MAddr index = 0;
	for(Line &line : m_lines){
		index ^= line.normal.vAddr.m_value & mask;
	}

	return m_lines.at(index);
}

Line& Table::pickVictim_accessed(){
	auto lambda = [](Line &line){return (line.is(LineType::NORMAL) && !line.normal.prio.a.accessed);};
	Line *res = find(lambda);

	if(res == NULL){
		return pickVictim_random();
	}

	return *res;
}

Line& Table::pickVictim_lru(){
	return *m_tail;
}

void Table::freeLines(){
	auto lambda = [&](Line &line) {freeLine(line);};
	std::for_each(m_lines.begin(), m_lines.end(), lambda);
}

//void Table::freeLines(const RPAddr &processId){
//	auto lambda =
//			[&, processId]
//			(Line &line)
//			{if(line.is(&processId, NULL, LineType::INDEXABLE)){freeLine(line);}};
//	std::for_each(m_lines.begin(), m_lines.end(), lambda);
//}

void Table::freeLines(const RPAddr &processId, const RMAddr *vAddr){
	auto lambda =
			[&]
			(Line &line)
			{if(line.is(&processId, vAddr, LineType::INDEXABLE)){freeLine(line);}};
	std::for_each(m_lines.begin(), m_lines.end(), lambda);
}


void Table::freeLine(Line &line){
	if(line.base.locked == true){
		throw exceptf<std::domain_error>("Attempt to invalidate locked TLB line");
	}

	line.base.present = false;
}

bool Line::is(const RPAddr *processId, const RMAddr *vAddr, LineType cmp)
{
	if(is(cmp)){
		if(processId == NULL || *processId == normal.processId){
				if(vAddr == NULL || *vAddr == normal.vAddr){
					return true;
				}
		}
	}
	return false;
}

EvictionStrategy getEvictionStrategy(const std::string name){
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
