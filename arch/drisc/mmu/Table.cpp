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
	//MLDTODO Force numLines is power of 2 when random is used (random, accessed)


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

//MLDTODO Does this function have to return a Result obj?
Result Table::storeNormal(RMAddr tableLineId, bool read, bool write, RMAddr pAddr, RMAddr &d$LineId)
{
	Line &line = m_lines.at(tableLineId.m_value);

	// Should be impossible anyway! Might as well assert!
	if(!line.is(LineType::PENDING)){
		return Result::FAILED;
	}

	d$LineId = line.pending.d$lineId;
	line.type.type = LineType::N_LOCKED;
	line.normal.read = read;
	line.normal.write = write;
	line.normal.pAddr = pAddr;

	return Result::SUCCESS;
}

Result Table::storePending(RPAddr processId, RMAddr vAddr, RMAddr d$LineId, RMAddr &tableLineId){}

void Table::Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const{
	(void)(out);
	(void)(arguments);
}
void Table::Cmd_Read(std::ostream& out, const std::vector<std::string>& arguments) const{
	(void)(out);
	(void)(arguments);
}

void Table::setPrioHigh(Line &entry){}

Line& Table::pickDestination() {
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

Line& Table::pickVictim_random() {
	MAddr mask = m_numLines - 1;
	MAddr index = 0;
	for(Line &line : m_lines){
		index ^= line.normal.vAddr.m_value & mask;
	}

	return m_lines.at(index);
}

Line& Table::pickVictim_accessed() {
	auto lambda = [](Line &line){return (line.is(LineType::NORMAL) && !line.normal.prio.a.accessed);};
	Line *res = find(lambda);

	if(res == NULL){
		return pickVictim_random();
	}

	return *res;
}

Line& Table::pickVictim_lru() {
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

void Table::printLruIndex(std::ostream& out, Line *entry) const{}

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
