#include "Table.h"

namespace Simulator {
namespace drisc {
namespace mmu {


Table::Table(const std::string& name, Object& parent):
Object(name, parent),
m_offsetWidth(GetConf("OffsetSize", size_t)),
m_vWidth(RAddr::VirtWidth - m_offsetWidth),
m_pWidth(RAddr::PhysWidth - m_offsetWidth),
m_evictionStrategy(getEvictionStrategy(GetConf("EvictionStrategy", std::string))),
m_numLines(GetConf("NumberOfEntries", uint64_t)), //MLDTODO Refactor to numLines
m_lines(m_numLines,	Line(m_vWidth, m_pWidth, ADDR_WIDTH_MAX)),
m_head(NULL),
m_tail(NULL),
m_indexWidth(std::log2(m_numLines))
{
	if(m_numLines == 0){
		throw exceptf<std::invalid_argument>("A table cannot be initialised with 0 lines");
	}

	initStrategy();

}

void Table::initStrategy(){

	if (m_evictionStrategy == EvictionStrategy::LRU) {
		// Initially link all nodes ascending.
		m_head = &(m_lines[0]);
		m_tail = &(m_lines[m_numLines-1]);
		m_head->prio.lru.previous = m_tail->prio.lru.next = NULL;

		for(unsigned int i=1; i<m_numLines; i++){
			m_lines[i-1].prio.lru.next = &(m_lines[i]);
			m_lines[i].prio.lru.previous = &(m_lines[i-1]);
		}
	}else{
		if (((m_numLines & (m_numLines - 1)) != 0)) {
			throw exceptf<std::invalid_argument>("This eviction strategy requires the number of lines to be a factor of 2");
		}
	}
}

Line *Table::find(RAddr processId, RAddr vAddr, LineTag type)
{
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
	if (res == m_lines.end()) {
		return NULL;
	} else {
		return &(*res);
	}
}

Addr Table::getIndex(const Line &line) const{
	for(unsigned int i=0; i<m_numLines; i++){
		if(&(m_lines.at(i)) == &line){
			return i;
		}
	}
	throw exceptf<std::invalid_argument>("The line does not exist!");
}

Result Table::getPending(RAddr tableLineId, RAddr &processId, RAddr &vAddr, RAddr &d$LineId){
	tableLineId.strictExpect(m_indexWidth);

	Line &line = m_lines.at(tableLineId.m_value);

	// Should be impossible anyway! Might as well assert!
	if(!line.is(LineTag::PENDING)){
		return Result::FAILED;
	}

	d$LineId = line.d$lineId;
	processId = line.processId;
	vAddr = line.vAddr;

	return Result::SUCCESS;
}

//MLDTODO Does this function have to return a Result obj?
Result Table::releasePending(RAddr tableLineId){
	tableLineId.strictExpect(m_indexWidth);

	Line &line = m_lines.at(tableLineId.m_value);

	// Should be impossible anyway! Might as well assert!
	if(!line.is(LineTag::PENDING)){
		return Result::FAILED;
	}

	// present stays false
	line.locked = false;

	return Result::SUCCESS;
}

Result Table::storeNormal(RAddr vAddr, RAddr pAddr, bool read, bool write){
	throw exceptf<std::domain_error>("Not Implemented");
}


//MLDTODO Does this function have to return a Result obj?
Result Table::storeNormal(RAddr tableLineId, bool read, bool write, RAddr pAddr, RAddr &d$LineId)
{
	pAddr.strictExpect(m_pWidth);
	tableLineId.strictExpect(m_indexWidth);

	Line &line = m_lines.at(tableLineId.m_value);

	// Should be impossible anyway! Might as well assert!
	if(!line.is(LineTag::PENDING)){
		return Result::FAILED;
	}

	d$LineId = line.d$lineId;
	// locked stays true
	line.present = true;
	line.read = read;
	line.write = write;
	line.pAddr = pAddr;

	return Result::SUCCESS;
}

Result Table::storePending(RAddr processId, RAddr vAddr, RAddr &d$LineId, Addr &tableLineId){
	vAddr.strictExpect(m_vWidth);

	Line *dst = find(processId, vAddr, LineTag::PENDING);

	if(dst == NULL){
		dst = &pickDestination();
		if(dst->locked){
			return Result::FAILED;
		}

		freeLine(*dst);

		dst->present = false;
		dst->locked = true;
		dst->processId = processId;
		dst->vAddr = vAddr;
	}

	RAddr oldD$LineId = dst->d$lineId;
	dst->d$lineId = d$LineId;

	d$LineId = oldD$LineId;
	tableLineId = getIndex(*dst);

	return Result::SUCCESS;
}

void Table::setPrioHigh(Line &line) {
	if (!line.present){ return; } //MLDTODO Correct check?

	if (this->m_evictionStrategy == EvictionStrategy::ACCESSED) {
		line.prio.a.accessed = true;
	} else if (this->m_evictionStrategy == EvictionStrategy::LRU) {
		if(&line != m_head){
			if(&line == m_tail){
				m_tail = line.prio.lru.previous;
				m_tail->prio.lru.next = NULL;
			}else{
				Line *prev = line.prio.lru.previous;
				Line *next = line.prio.lru.next;
				prev->prio.lru.next = next;
				next->prio.lru.previous = prev;
			}
			line.prio.lru.previous = NULL;
			line.prio.lru.next = m_head;
			m_head->prio.lru.previous = &line;
			m_head = &line;
		}
	}
}
Line& Table::pickDestination(){
	Line *dest = find(LineTag::FREE);

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
	Addr mask = m_numLines - 1;
	Addr index = 0;
	for(Line &line : m_lines){
		index ^= line.vAddr.m_value & mask;
	}

	return m_lines.at(index);
}

Line& Table::pickVictim_accessed(){
	auto lambda = [](Line &line){return (line.is(LineTag::NORMAL) && !line.prio.a.accessed);};
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

void Table::freeLines(const RAddr &processId, const RAddr *vAddr){ //MLDTODO Document / find out why processId is ref and vAddr is pointer
	auto lambda =
			[&]
			(Line &line)
			{if(line.is(&processId, vAddr, LineTag::INDEXABLE)){freeLine(line);}};
	std::for_each(m_lines.begin(), m_lines.end(), lambda);
}


void Table::freeLine(Line &line){
	if(line.locked == true){
		throw exceptf<std::domain_error>("Attempt to invalidate locked TLB line");
	}

	line.present = false;
}

Line::Line(AddrWidth vAddrWidth, AddrWidth pAddrWidth, AddrWidth d$AddrWidth):
	present(false),
	locked(false),
	processId(0, RAddr::ProcIdWidth),
	vAddr(0, vAddrWidth),
	pAddr(0, pAddrWidth),
	d$lineId(0, d$AddrWidth),
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

void Table::Cmd_Info(std::ostream& out, const std::vector<std::string>& /* arguments */) const {
	out << "The Table blablabla\n\n"; //MLDTODO Maybe add some meaningfull, inspiring and brilliant text
	out << "  Eviction strategy: " << m_evictionStrategy << "\n";
	out << "        Offset size: " << unsigned(m_offsetWidth) << " bytes\n";
	out << "    Number of lines: " << unsigned(m_numLines) << "\n\n";
	Cmd_Usage(out);

	//MLDTODO Display statistics
}

//MLDTODO Needs to be rewritten to account for new line types
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

	out << " | " << left << setw(RAddr::getPrintWidth(RAddr::ProcIdWidth)) << "Proc. ID";
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
			out << setw(1) << line.prio.a.accessed << " | ";
		} else if (this->m_evictionStrategy == EvictionStrategy::LRU) {
			out << setw(4);
			if(line.prio.lru.previous == NULL){	out << "-"; }
			else{ out << getIndex(*line.prio.lru.previous); }

			out << " | " << setw(4);

			if(line.prio.lru.next == NULL){	out << "-"; }
			else{ out << getIndex(*line.prio.lru.next); }

			out << " | ";
		}

		out << line.processId << " | ";
		out << line.vAddr << " | ";

		if(line.is(LineTag::FREE)){
			out << "FREE: -" << endl;
		}else if(line.is(LineTag::PENDING)){
			out << dec << "PENDING: D$-line: " << line.d$lineId << endl;
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
    	try{
    		m_evictionStrategy = getEvictionStrategy(arguments[1]);
    		initStrategy();
    		out << "Changed eviction strategy from " << old << " to " << m_evictionStrategy << std::endl;
    		out << "Priorities have been reset where applicable" << std::endl;
    	}catch (std::exception &e){
            out << "An exception occured while changing strategy:" << std::endl;
            out << e.what() << std::endl;
            m_evictionStrategy = old;
        }
    }else if(arg.is(0, false, "h", "head")){
    	arg.expect(2);

    	//MLDTODO Handle through argument.h/cpp
    	if(m_evictionStrategy != EvictionStrategy::LRU){
    		out << "Eviction strategy is not set to LRU" << std::endl;
    	}

    	try{
    		Addr index = std::strtoul(arguments[1].c_str(), NULL, 0 );
    		Line line = m_lines[index];
    		m_head = &line;

    		out << "Changed LRU head to " << index << std::endl;
    	}catch (std::exception &e){
            out << "An exception occured while changing LRU head:" << std::endl;
            out << e.what() << std::endl;
        }
    }else if(arg.is(0, false, "t", "tail")){
    	arg.expect(2);

    	if(m_evictionStrategy != EvictionStrategy::LRU){
    		out << "Eviction strategy is not set to LRU" << std::endl;
    	}

    	try{
    		Addr index = std::strtoul(arguments[1].c_str(), NULL, 0 );
    		Line line = m_lines[index];
    		m_tail = &line;

    		out << "Changed LRU tail to " << index << std::endl;
    	}catch (std::exception &e){
            out << "An exception occured while changing LRU tail:" << std::endl;
            out << e.what() << std::endl;
        }
    }else if(arg.is(0, false, "l", "line")){
    	//MLDTODO Somewhere, vaddr-width is set to 0!
    	arg.expect(2, SIZE_MAX);

    	Addr lineIndex;
    	Line *line;
    	try{
    		lineIndex = std::strtoul(arguments[1].c_str(), NULL, 0 );
        	line = &(m_lines[lineIndex]);
    	}catch (std::exception &e){
            out << "Could not find line:" << std::endl;
            out << e.what() << std::endl;
            return;
    	}

    	std::map<std::string, Addr> settings;

    	for(size_t i=2; i<arguments.size(); i++){
    		std::string str = arguments[i];
    		size_t pos = str.find('=');
    		if(pos == std::string::npos){
    			out << "Invalid argument `" << str << "`" << std::endl;
        		Cmd_Usage_write_line(out);
        		return;
    		}
    		std::string varName = str.substr(0, pos);

    		if(std::string("=p=pid=pa=prev=l=va=rd=wr=d$=a=nxt=").find("=" + varName + "=") == std::string::npos){
    			out << "Invalid variable name `" << varName << "`" << std::endl;
        		Cmd_Usage_write_line(out);
        		return;
    		}

    		Addr value = strtoull(str.substr(pos + 1).c_str(), NULL, 0);

    		if(varName=="p" || varName=="l" || varName=="rd" || varName=="wr" || varName=="a"){
    			if(value > 1){
        			out << "Invalid value for variable `" << varName << "`. The value must be 0 or 1" << std::endl;
            		Cmd_Usage_write_line(out);
            		return;
    			}
    		}else if(varName=="prev" || varName=="nxt"){
    	    	try{
    	    		value = (Addr)&(m_lines[lineIndex]);
    	    	}catch (std::exception &e){
    	            out << "Could not find line:" << std::endl;
    	            out << e.what() << std::endl;
    	            return;
    	    	}
    		}else if(varName=="pid"){
    			if(!RAddr::isValid(value, RAddr::ProcIdWidth)){
    	            out << "Value for pid is out of range. (Max width: " << RAddr::ProcIdWidth << ")" << std::endl;
    	            return;
    			}
    		}else if(varName=="pa"){
    			if(!RAddr::isValid(value, RAddr::PhysWidth)){
    	            out << "Value for pa is out of range. (Max width: " << RAddr::PhysWidth << ")" << std::endl;
    	            return;
    			}
    		}else if(varName=="va"){
    			if(!RAddr::isValid(value, RAddr::VirtWidth)){
    	            out << "Value for va is out of range. (Max width: " << RAddr::VirtWidth << ")" << std::endl;
    	            return;
    			}
    		}

    		settings[varName] = value;
    	}

		if(settings.count("p"))		{line->present = settings["p"];}
		if(settings.count("l"))		{line->locked = settings["l"];}
		if(settings.count("rd"))	{line->read = settings["rd"];}
		if(settings.count("wr"))	{line->write = settings["wr"];}
		if(settings.count("a"))		{line->prio.a.accessed = settings["a"];}
		if(settings.count("prev"))	{line->prio.lru.previous = (Line*)settings["prev"];}
		if(settings.count("nxt"))	{line->prio.lru.next = (Line*)settings["nxt"];}
		if(settings.count("pid"))	{line->processId = settings["pid"];}
		if(settings.count("pa"))	{line->pAddr = settings["pa"];}
		if(settings.count("va"))	{line->vAddr = settings["va"];}
		if(settings.count("d$"))	{line->d$lineId = settings["d$"];}

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
