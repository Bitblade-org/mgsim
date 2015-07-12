#include "TLB.h"

#include <stddef.h>

#include <sim/config.h>
#include <algorithm>
#include <assert.h>

namespace Simulator {
namespace drisc {
namespace mmu {

//MLDTODO-DOC On reserve of +L-P entry: What to do if the pickDestination algo returns a locked entry? Nothing for now...

TLB::TLB(const std::string& name, Object& parent)
	: Object(name, parent),
	m_numTables(GetConf("NumberOfTables", size_t)),
	m_tables(m_numTables),
	m_enabled(false),
	m_tableAddr(0, RAddr::PhysWidth),
	m_managerAddr(0)
{
	if(m_numTables == 0){
        throw exceptf<InvalidArgumentException>("%s must have at least one table.", name.c_str());
	}

	for( int i=0; i<m_numTables; i++){

		Table *table = new Table("table" + std::to_string(i), *this);
		if(i > 0 && table->getOffsetWidth() <= m_tables[i-1]->getOffsetWidth()){
            throw exceptf<InvalidArgumentException>("TLB tables must have ascending offset widths.");
		}
		m_tables[i] = table;
	}
}

TLB::~TLB(){
	auto lambda = [](Table *table){delete table;};
	std::for_each(m_tables.begin(), m_tables.end(), lambda);
}

// SUCCESS: Address in TLB
// DELAYED: Address not in TLB, expecting refill
// FAILED:  Stalled or Address not in TLB and unable to transmit refill request
// domain_error: TLB is disabled
Result TLB::lookup(RAddr const processId, RAddr const vAddr, RAddr &d$lineId, bool &r, bool &w, RAddr &pAddr, bool mayUnlock){
	if(!m_enabled){
		throw exceptf<std::domain_error>("TLB cannot handle lookups while disabled");
	}

	if(!processId.isValid()){
		throw std::invalid_argument("Process ID is not valid");
	}

	vAddr.strictExpect(m_tables[0]->getVAddrWidth());

	Line *line = find(processId, vAddr, LineTag::PRESENT);
	if(line == NULL){
		Addr tableLineId;
		Result res = m_tables[0]->storePending(processId, vAddr, d$lineId, tableLineId);

		if(res == Result::FAILED){
			return Result::FAILED;
		}

		std::cout << "I should be doing something here... What was it..." << std::endl;
		//MLDTODO Send request to manager!

		return Result::DELAYED;
	}

	if(line->locked && mayUnlock){
		line->locked = false;
	}

	r = line->read;
	w = line->write;
	pAddr = line->pAddr;

	return Result::SUCCESS;
}

Line* TLB::find(RAddr processId, RAddr vAddr, LineTag tag){
	Line *discoveredEntry = NULL; //MLDTODO Remove after debugging

	for(Table *table : m_tables){
		RAddr truncatedAddr = vAddr.truncateLsb(table->getVAddrWidth());
		Line *line = table->find(processId, truncatedAddr, tag);
		if(line != NULL){
			if(discoveredEntry != NULL){ //MLDTODO Remove after debugging
				throw exceptf<std::domain_error>("vAddr %lX exists in multiple tables!", vAddr.m_value);
			}
			discoveredEntry = line;
		}
	}

	return discoveredEntry;
}

void TLB::invalidate(){
	auto lambda = [](Table *table){table->freeLines();};
	std::for_each(m_tables.begin(), m_tables.end(), lambda);
}

void TLB::invalidate(RAddr pid){
	auto lambda = [pid](Table *table){table->freeLines(pid, NULL);};
	std::for_each(m_tables.begin(), m_tables.end(), lambda);
}

void TLB::invalidate(RAddr pid, RAddr addr){
	addr.strictExpect(RAddr::VirtWidth - getMinOffsetSize());

	for(Table *table : m_tables){
		std::cout << "Truncate to: " << unsigned(table->getVAddrWidth());
		RAddr tableAddr = addr.truncateLsb(table->getOffsetWidth() - getMinOffsetSize());
		std::cout << " result: " << tableAddr << std::endl;
		table->freeLines(pid, &tableAddr);
	}
}

Result TLB::onInvalidateMsg(RemoteMessage &msg){
	RAddr addr = RAddr(msg.tlbInvalidate.addr, RAddr::VirtWidth - getMinOffsetSize());
	RAddr processId = RAddr(msg.tlbInvalidate.processId, RAddr::ProcIdWidth);
	if(msg.tlbInvalidate.filterAddr && msg.tlbInvalidate.filterPid){
		invalidate(processId, addr);
	}else if(!msg.tlbInvalidate.filterAddr && msg.tlbInvalidate.filterPid){
		invalidate(processId);
	}else if(!msg.tlbInvalidate.filterAddr && !msg.tlbInvalidate.filterPid){
		invalidate();
	}

	return Result::SUCCESS;
}

Result TLB::onPropertyMsg(RemoteMessage &msg){
	assert(msg.tlbProperty.tlb == TlbType::DTLB);

	if(msg.tlbProperty.type == TlbPropertyMsgType::ENABLED){
		assert(msg.tlbProperty.value <= 1);
		m_enabled = msg.tlbProperty.value;
	}else if(msg.tlbProperty.type == TlbPropertyMsgType::MANAGER_ADDRESS){
		//MLDTODO Checking
		m_managerAddr = msg.tlbProperty.value;
	}else if(msg.tlbProperty.type == TlbPropertyMsgType::PT_ADDRESS){
		m_tableAddr = RAddr(RAddr::PhysWidth, msg.tlbProperty.value);
	}

	return Result::SUCCESS;
}

Result TLB::onStoreMsg(RemoteMessage &msg){
	assert(msg.dTlbStore.table < m_numTables);

	Table *table = m_tables[msg.dTlbStore.table];
	RAddr d$lineId;
	RAddr pAddr = RAddr(table->getPAddrWidth(), msg.dTlbStore.pAddr);
	RAddr lineIndex = RAddr(table->getIndexWidth(), msg.dTlbStore.lineIndex);

	Result res;
	if(msg.dTlbStore.table == 0){
		res = table->storeNormal(lineIndex, msg.dTlbStore.read, msg.dTlbStore.write, pAddr, d$lineId);
	}else{
		RAddr vAddr = RAddr(0, m_tables[0]->getVAddrWidth());
		RAddr processId = RAddr(0, RAddr::ProcIdWidth);

		m_tables[0]->getPending(lineIndex, processId, vAddr, d$lineId);

		vAddr = vAddr.truncateLsb(table->getOffsetWidth() - m_tables[0]->getOffsetWidth());
		res = table->storeNormal(vAddr, pAddr, msg.dTlbStore.read, msg.dTlbStore.write);

		if(res == Result::SUCCESS){
			m_tables[0]->releasePending(lineIndex);
		}
	}

	if(res != Result::SUCCESS){
		return Result::FAILED;
		//MLDTODO-DOC Continuatie gegarandeerd zolang een line enkel vanaf netwerk gelocked kan worden. (Line van table <> 0 kan niet pending zijn)
	}

	std::cout << "INFORM D$, LINEID " << d$lineId; //MLDTODO Inform D$

	return Result::SUCCESS;
}


void TLB::Cmd_Info(std::ostream& out, const std::vector<std::string>& /* arguments */) const{
    //MLDTODO Display statistics
    out << "The TLB blablabla\n\n";
    out << "  Number of tables: " << unsigned(m_numTables) << "\n";
    out << "             State: " << (m_enabled ? "enabled." : "DISABLED!") << "\n";
    out << "Page table address: " << m_tableAddr << "\n";
    out << "   Manager address: " << m_managerAddr << "\n\n";
    Cmd_Usage(out);
}

void TLB::Cmd_Usage(std::ostream& out) const{
	out << "Supported operations:\n";
	out << "    write s(tatus)         <1|ENABLED|0|DISABLED>\n";
	out << "    write m(anagerAddr)    <value>\n";
	out << "    write p(ta)     	   <value>\n";
	out << std::endl;
	out << "Simulate incomming messages:\n";
	out << "    write sim-l(ookup)     <values>           (PIPELINE > TLB)\n";
	out << "    write sim-i(nvalidate) (MANAGER  > TLB)\n";
	out << "      <filterPid> <ProcessId> <filterAddr> <Addr>\n";
	out << "    write sim-p(roperty)    <type> <value>     (MANAGER  > TLB)\n";
	out << "    write sim-s(tore)      (MANAGER  > TLB)\n";
	out << "      <TableId> <LineId> <ProcessId> <PAddr> <read> <write>\n";
	out << std::endl;
}

void TLB::Cmd_Write(std::ostream& out, const std::vector<std::string>& arguments)
{
	Arguments arg(arguments);

    if (arg.size() == 0){
    	Cmd_Usage(out);
    	return;
    }

    if(arg.is(0, false, "s", "status")){
    	arg.expect(2);
		out << "Changed status from " << m_enabled << " to ";
		arg.set(1, m_enabled);
		out << m_enabled << std::endl;
    }else if(arg.is(0, false, "m", "manageraddr")){
    	arg.expect(2);
    	out << "Changed Manager Address from " << m_managerAddr;
    	arg.set(1, m_managerAddr);
    	out << " to " << m_managerAddr << std::endl;
    }else if(arg.is(0, false, "p", "pta")){
    	arg.expect(2);
    	out << "Changed Page Table Address from " << m_tableAddr;
    	arg.set(1, m_tableAddr);
    	out << " to " << m_tableAddr << std::endl;
    }else if(arg.is(0, false, "sim-l", "sim-lookup")){
    	out << "Not yet implemented!" << std::endl; //MLDTODO Implement
    }else if(arg.is(0, false, "sim-i", "sim-invalidate")){
    	arg.expect(5);

    	RemoteMessage msg;
    	arg.set(1, msg.tlbInvalidate.filterPid);
    	arg.set(3, msg.tlbInvalidate.filterAddr);
    	msg.tlbInvalidate.processId = arg.getMAddr(2, RAddr::ProcIdWidth);
    	msg.tlbInvalidate.addr = arg.getMAddr(4, RAddr::VirtWidth);

    	Result res = onInvalidateMsg(msg);
    	out << "Simulated invalidation message with result: " << res << std::endl;
    }else if(arg.is(0, false, "sim-p", "sim-property")){
    	arg.expect(3);

    	RemoteMessage msg;
    	msg.tlbProperty.tlb = TlbType::DTLB;
    	msg.tlbProperty.type = getTlbPropertyMsgType(arg.getString(1, false));

    	//MLDTODO Transform using case
    	if(msg.tlbProperty.type == TlbPropertyMsgType::ENABLED){
    		arg.set(2, msg.tlbProperty.value);
    	}else if(msg.tlbProperty.type == TlbPropertyMsgType::PT_ADDRESS){
    		msg.tlbProperty.value = arg.getMAddr(2, RAddr::PhysWidth);
    	}else{
    		msg.tlbProperty.value = arg.getMAddr(2); //MLDTODO Check validitiy
    	}

    	Result res = onPropertyMsg(msg);
    	out << "Simulated property message with result: " << res << std::endl;
    }else if(arg.is(0, false, "sim-s", "sim-store")){
    	arg.expect(6);

    	RemoteMessage msg;
    	msg.dTlbStore.table = arg.getMAddr(1);
    	msg.dTlbStore.lineIndex = arg.getMAddr(2);
    	msg.dTlbStore.pAddr = arg.getMAddr(3, RAddr::ProcIdWidth);
    	msg.dTlbStore.read = arg.getBool(4);
    	msg.dTlbStore.write = arg.getBool(5);

    	Result res = onStoreMsg(msg);
    	out << "Simulated store message with result: " << res << std::endl;
    }else{
    	Cmd_Usage(out);
    }
}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
