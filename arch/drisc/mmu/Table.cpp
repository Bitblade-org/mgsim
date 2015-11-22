#include "Table.h"

#include "MMU.h"
#include <sim/log2.h>
namespace Simulator {
namespace drisc {
namespace mmu {

Table::Table(const std::string& name, Object& parent):
Object(name, parent),
m_offsetWidth(GetConf("OffsetSize", size_t)),
m_vWidth(getMMU().vAW() - m_offsetWidth),
m_pWidth(getMMU().pAW() - m_offsetWidth),
m_numLines(GetConf("NumberOfLines", size_t)),
m_evictionStrategy(getEvictionStrategy(GetConf("EvictionStrategy", std::string))),
m_lines(m_numLines,	Line(getMMU().procAW(), m_vWidth, m_pWidth)),
m_head(NULL),
m_tail(NULL),
m_indexWidth(ilog2(m_numLines))
{
	if(m_numLines == 0){
		throw exceptf<std::invalid_argument>("A table cannot be initialised with 0 lines");
	}
	initStrategy();
}

AddrWidth Table::getVAddrWidth() const {return getMMU().vAW() - m_offsetWidth;} //MLDTODO Inline? Smth else?
AddrWidth Table::getPAddrWidth() const {return getMMU().pAW() - m_offsetWidth;}

void Table::initStrategy(){
	if (m_evictionStrategy == EvictionStrategy::LRU) {
		// Initially link all nodes in ascending order.
		m_head = &(m_lines[0]);
		m_tail = &(m_lines[m_numLines-1]);
		m_head->previous = m_tail->next = NULL;

		for(unsigned int i=1; i<m_numLines; i++){
			m_lines[i-1].next = &(m_lines[i]);
			m_lines[i].previous = &(m_lines[i-1]);
		}
	}else{
		if (((m_numLines & (m_numLines - 1)) != 0)) {
			throw exceptf<std::invalid_argument>("This eviction strategy requires the number of lines to be a factor of 2");
		}
	}
}

Line *Table::lookup(RAddr processId, RAddr vAddr, LineTag type){
	processId.strictExpect(getMMU().procAW());
	vAddr.strictExpect(m_vWidth);

	Line* line = find(processId, vAddr, type);
	if(line != NULL) { setPrioHigh(*line); }

	return line;
}


Line *Table::find(RAddr processId, RAddr vAddr, LineTag type){
	processId.strictExpect(getMMU().procAW());
	vAddr.strictExpect(m_vWidth);

	auto lambda =
			[&processId, &vAddr, &type]
			(Line &line)
			{return line.is(&processId, &vAddr, type);};
	return find(lambda);
}

Line *Table::find(LineTag type)
{
	auto lambda = [type](Line &line) {return line.is(type);};
	return find(lambda);
}

Line *Table::find(std::function<bool (Line&)> const &lambda)
{
	auto res = std::find_if(m_lines.begin(), m_lines.end(), lambda);
	return (res == m_lines.end())? NULL : &(*res);
}

bool Table::getPending(RAddr tableLineId, RAddr &processId, RAddr &vAddr, RAddr &d$LineId){
	tableLineId.strictExpect(m_indexWidth);

	Line &line = m_lines.at(tableLineId.m_value);

	if(!line.is(LineTag::PENDING)){
		return false;
	}

	d$LineId = line.d$lineRef;
	processId = line.processId;
	vAddr = line.vAddr;

	return true;
}

void Table::releasePending(RAddr tableLineId){
	tableLineId.strictExpect(m_indexWidth);

	Line &line = m_lines.at(tableLineId.m_value);

	assert(line.is(LineTag::PENDING));

	COMMIT{
		line.locked = false;
	}
}

Line* Table::storeNormal(RAddr processId, RAddr vAddr, RAddr pAddr, bool read, bool write){
	vAddr.strictExpect(m_vWidth);
	pAddr.strictExpect(m_pWidth);
	processId.strictExpect(getMMU().procAW());

	Line &dst = pickDestination();
	if(dst.locked){
		DeadlockWrite("Chosen destination TLB line is locked");
		return NULL;
	}

	freeLine(dst);

	COMMIT{
		dst.present = true;
		dst.locked = true;
		dst.processId = processId;
		dst.vAddr = vAddr;
		dst.pAddr = pAddr;
		dst.read = read;
		dst.write = write;
	}

	return &dst;
}

Line* Table::fillPending(RAddr tableLineId, bool read, bool write, RAddr pAddr, RAddr &d$LineId)
{
	pAddr.strictExpect(m_pWidth);
	tableLineId.strictExpect(m_indexWidth);

	Line &line = m_lines[tableLineId.m_value];
	assert(line.is(LineTag::PENDING));

	d$LineId = line.d$lineRef;

	COMMIT{
		// locked stays true
		line.present = true;
		line.read = read;
		line.write = write;
		line.pAddr = pAddr;
	}

	return &line;
}

bool Table::storePending(RAddr processId, RAddr vAddr, Addr &tableLineId, Line* &line){
	processId.strictExpect(getMMU().procAW());
	vAddr.strictExpect(m_vWidth);

	//line = find(processId, vAddr, LineTag::PENDING);

	//if(line == NULL){
		line = &pickDestination();
		if(line->locked){
			return false;
		}

		COMMIT{
			freeLine(*line);

			line->present = false;
			line->locked = true;
			line->processId = processId;
			line->vAddr = vAddr;
		}
	//}

	tableLineId = getIndex(*line);


	return true;
}

void Table::setPrioHigh(Line &line) {
	if (this->m_evictionStrategy == EvictionStrategy::ACCESSED) {
		line.accessed = true;
	} else if (this->m_evictionStrategy == EvictionStrategy::LRU) {
		if(&line != m_head){
			if(&line == m_tail){
				m_tail = line.previous;
				m_tail->next = NULL;
			}else{
				Line *prev = line.previous;
				Line *next = line.next;
				prev->next = next;
				next->previous = prev;
			}
			line.previous = NULL;
			line.next = m_head;
			m_head->previous = &line;
			m_head = &line;
		}
	}
}
Line& Table::pickDestination(){
	Line *dest = find(LineTag::FREE);

	if (dest == NULL) {
		switch(m_evictionStrategy){
		case EvictionStrategy::PSEUDO_RANDOM:
			dest = &pickVictim_random(); break;
		case EvictionStrategy::ACCESSED:
			dest = &pickVictim_accessed(); break;
		case EvictionStrategy::LRU:
			dest = &pickVictim_lru(); break;
		default:
			UNREACHABLE
		}
	}

	return *dest;
}

Line& Table::pickVictim_random(){
	Addr mask = m_numLines - 1;
	Addr index = 0;
	for(Line &line : m_lines){
		index ^= line.vAddr.m_value & mask;
	}

	return m_lines.at(index);
}

Line& Table::pickVictim_accessed(){
	auto lambda = [](Line &line){return (line.is(LineTag::NORMAL) && !line.accessed);};
	Line *res = find(lambda);

	return (res == NULL) ? pickVictim_random() : *res;
}

Line& Table::pickVictim_lru(){
	return *m_tail;
}

void Table::freeLines(){
	auto lambda = [&](Line &line) {freeLine(line);};
	std::for_each(m_lines.begin(), m_lines.end(), lambda);
}

void Table::freeLines(const RAddr &processId, const RAddr *vAddr){ //MLDTODO Document / find out why processId is ref and vAddr is pointer
	auto lambda =
			[&]
			(Line &line)
			{if(line.is(&processId, vAddr, LineTag::INDEXABLE)){freeLine(line);}};
	std::for_each(m_lines.begin(), m_lines.end(), lambda);
}


void Table::freeLine(Line &line){
	//MLDTODO Figure out how to handle locked lines while invalidating.
	if(line.locked == true){
		throw exceptf<std::domain_error>("Attempt to invalidate locked TLB line");
	}

	COMMIT{
		line.present = false;
	}
}

Line::Line(AddrWidth procWidth, AddrWidth vAddrWidth, AddrWidth pAddrWidth):
	present(false),
	locked(false),
	accessed(false),
	previous(NULL),
	next(NULL),
	processId(0,procWidth),
	vAddr(0, vAddrWidth),
	pAddr(0, pAddrWidth),
	d$lineRef(0),
	read(false),
	write(false)
{}

/*
 * p(0)	l(0) : bit 0 set  --> tag 1
 * p(0) l(1) : bit 1 set  --> tag 2
 * p(1) l(0) : bit 2 set  --> tag 4
 * p(1) l(1) : bit 3 set  --> tag 8
 *
 * is(0b0000): Always false
 * is(0b0001): True if p=0 && l=0
 * is(0b0011): True if p=0 (l may be 0 or 1)
 * is(ob0110): True if (p=0 && l=1) || (p=1 && l=0)
 */
bool Line::is(LineTag cmp){
	unsigned char type = (present << 1) | locked;
	return (((unsigned)cmp) & ( 1 << type )) >> type;
}

bool Line::is(const RAddr *cmpProcessId, const RAddr *cmpVAddr, LineTag cmp)
{
	if(is(cmp)){
		if(cmpProcessId == NULL || *cmpProcessId == this->processId){
				if(cmpVAddr == NULL || *cmpVAddr == this->vAddr){
					return true;
				}
		}
	}
	return false;
}

EvictionStrategy getEvictionStrategy(const std::string name){
	if (name == "PSEUDO_RANDOM" || name == "RANDOM" || name == "R") {
		return EvictionStrategy::PSEUDO_RANDOM;
	}
	if (name == "ACCESSED" || name == "A") {
		return EvictionStrategy::ACCESSED;
	}
	if (name == "LRU") {
		return EvictionStrategy::LRU;
	}
	throw exceptf<SimulationException>("Unknown eviction strategy");
}

std::ostream& operator<<(std::ostream& os, EvictionStrategy strategy) {
	switch (strategy) {
		case EvictionStrategy::PSEUDO_RANDOM: os << "PSEUDO_RANDOM"; break;
		case EvictionStrategy::ACCESSED: os << "ACCESSED"; break;
		case EvictionStrategy::LRU:	os << "LRU"; break;
		default: UNREACHABLE
	}

	return os;
}

void Table::Cmd_Info(std::ostream& out, const std::vector<std::string>& /* arguments */) const {
	out << "The Table blablabla\n\n"; //MLDTODO Maybe add some meaningfull, inspiring and brilliant text
	out << "      Eviction strategy: " << m_evictionStrategy << "\n";
	out << "            Offset size: " << unsigned(m_offsetWidth) << " bytes\n";
	out << "        Number of lines: " << unsigned(m_numLines) << "\n";
	out << "  Virtual address width: " << unsigned(m_vWidth) << "\n";
	out << " Physical address width: " << unsigned(m_pWidth) << "\n";
	Cmd_Usage(out);

	//MLDTODO Display statistics
}

void Table::Cmd_Read(std::ostream& out,	const std::vector<std::string>& /* arguments */) const {
	using namespace std;

	if(this->m_evictionStrategy == EvictionStrategy::LRU){
		out << "LRU Linked list: (head) " << getIndex(*m_head) << " <" << unsigned(m_numLines);
		out << " entries> " << getIndex(*m_tail) << " (tail)" << endl;
	}

	out	<< "  # | P | L";

	if (this->m_evictionStrategy == EvictionStrategy::ACCESSED) {
		out << " | A";
	} else if (this->m_evictionStrategy == EvictionStrategy::LRU) {
		out << " | Prev | Next";
	}

	out << " | " << left << setw(RAddr::getPrintWidth(getMMU().procAW())) << "Proc. ID";
	out << " | " << left << setw(RAddr::getPrintWidth(getVAddrWidth())) << "vAddr";
	out << " | " << "Type-specific Fields";
	out << endl;

	for (unsigned i = 0; i < m_lines.size(); i++) {
		Line line = m_lines.at(i);
		out << noboolalpha << noshowbase << setfill(' ') << dec;
		out << setw(3) << i << " | ";
		out << setw(1) << line.present << " | ";
		out << setw(1) << line.locked << " | ";

		if (this->m_evictionStrategy == EvictionStrategy::ACCESSED) {
			out << setw(1) << line.accessed << " | ";
		} else if (this->m_evictionStrategy == EvictionStrategy::LRU) {
			out << setw(4);
			if(line.previous == NULL){	out << "-"; }
			else{ out << getIndex(*line.previous); }

			out << " | " << setw(4);

			if(line.next == NULL){	out << "-"; }
			else{ out << getIndex(*line.next); }

			out << " | ";
		}

		out << line.processId << " | ";
		out << line.vAddr << " | ";

		if(line.is(LineTag::FREE)){
			out << "FREE: -" << endl;
		}else if(line.is(LineTag::PENDING)){
			out << dec << "PENDING: D$-line: " << line.d$lineRef << endl;
		}else if(line.is(LineTag::PRESENT)){
			out << "PRESENT: PA=" << line.pAddr << " ";
			out << "R=" << dec << setw(1) << line.read << " ";
			out << "W=" << setw(1) << line.write;
			out << endl;
		}
	}
}

void Table::Cmd_Write(std::ostream& out, const std::vector<std::string>& arguments)
{
	Arguments arg(arguments);

    if (arg.size() == 0){
    	Cmd_Usage(out);
    	return;
    }

    if(arg.is(0, false, "s", "strategy")){
    	arg.expect(2);
		EvictionStrategy old = m_evictionStrategy;

		m_evictionStrategy = getEvictionStrategy(arguments[1]);
		initStrategy();
		out << "Changed eviction strategy from " << old << " to " << m_evictionStrategy << std::endl;
		out << "Priorities have been reset where applicable" << std::endl;
    }else if(arg.is(0, false, "h", "head")){
    	arg.expect(2);
    	assert(m_evictionStrategy == EvictionStrategy::LRU);

    	Addr index = arg.getULL(1, m_numLines);
    	Line *line = &(m_lines[index]);

    	out << "Changed LRU Head from " << getIndex(*m_head);
    	m_head = line;
    	out << " to " << index << std::endl;
    }else if(arg.is(0, false, "t", "tail")){
    	arg.expect(2);
    	assert(m_evictionStrategy == EvictionStrategy::LRU);

    	Addr index = arg.getULL(1, m_numLines);
    	Line *line = &(m_lines[index]);

    	out << "Changed LRU Tail from " << getIndex(*m_tail);
    	m_tail = line;
    }else if(arg.is(0, false, "l", "line")){
    	arg.expect(2, SIZE_MAX);

		Addr lineIndex = arg.getULL(1, m_numLines);
		Line *line = &(m_lines[lineIndex]);

		arg.namedSet("p", true, line->present);
		arg.namedSet("pid", true, line->processId);
		arg.namedSet("pa", true, line->pAddr);
		arg.namedSet("l", true, line->locked);
		arg.namedSet("va", true, line->vAddr);
		arg.namedSet("rd", true, line->read);
		arg.namedSet("wr", true, line->write);
		arg.namedSet("d$", true, (Addr&)line->d$lineRef);
		arg.namedSet("a", true, line->accessed);

		RAddr lineAddr = RAddr(0, m_numLines);
		if(arg.namedSet("prev", true, lineAddr)){
    		line->previous = &(m_lines[lineAddr.m_value]);
		}

		if(arg.namedSet("next", true, lineAddr)){
			line->next = &(m_lines[lineAddr.m_value]);
		}

		out << "Line " << unsigned(lineIndex) << " has been updated" << std::endl;
    }else{
    	Cmd_Usage(out);
    }
}

void Table::Cmd_Usage_write_line(std::ostream& out) const{
	out << "write line <lineId> <var=val>...:\n";
	out << "    variables:\n";
	out << "        p    Present\n";
	out << "        pid  Process ID\n";
	out << "        pa   Physical Address\n";
	out << "        prev Previous Line (Only for LRU tables)\n";
	out << "        l    Locked\n";
	out << "        va   Virtual Address\n";
	out << "        rd   Read (Only when present)\n";
	out << "        wr   Write (Only when present)\n";
	out << "        d$   D-Cache Line id (Only when locked and not present)\n";
	out << "        a    Accessed (Only for ACCESSED tables)\n";
	out << "        nxt  Next line (Only for LRU tables)\n";
	out << std::endl;
}

void Table::Cmd_Usage(std::ostream& out) const{
	out << "Supported operations:\n";
	out << "    read\n";
	out << "    write s(trategy) <RANDOM|ACCESSED|LRU>\n";
	out << "    write h(head)    <index>\n";
	out << "    write t(ail)     <index>\n";
	out << "    write l(ine)     <index> <variable=value>...\n";
	out << std::endl;
}


} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
