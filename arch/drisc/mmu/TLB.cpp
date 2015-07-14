#include "TLB.h"

#include <stddef.h>

#include <sim/config.h>
#include <algorithm>
#include <assert.h>
#include <arch/drisc/DRISC.h>


namespace Simulator {
namespace drisc {
namespace mmu {

//MLDTODO-DOC On reserve of +L-P entry: What to do if the pickDestination algo returns a locked entry? Nothing for now...

TLB::TLB(const std::string& name, Object& parent)
	: Object(name, parent),
	m_numTables(GetConf("NumberOfTables", size_t)),
	m_tables(m_numTables),
	m_enabled(false),
	m_tableAddr(0, getMMU().pAW()),
	m_managerAddr(0, getMMU().netAW())
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

	Line *line = doLookup(processId, vAddr, LineTag::PRESENT);
	if(line == NULL){
		Addr tableLineId;
		Result res = m_tables[0]->storePending(processId, vAddr, d$lineId, tableLineId);

		if(res == Result::FAILED){
			return Result::FAILED;
		}

		RemoteMessage msg;
		msg.type = msg.Type::MSG_TLB_MISS_MESSAGE;
		msg.TlbMissMessage.addr = vAddr.m_value;
		msg.TlbMissMessage.processId = processId.m_value;
		msg.TlbMissMessage.lineIndex = tableLineId;
		msg.TlbMissMessage.tlb = TlbType::DTLB;
		msg.TlbMissMessage.dest = m_managerAddr.m_value;


		if(!GetDRISC().GetNetwork().SendMessage(msg)){
			return Result::FAILED;
		}

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

Line* TLB::doLookup(RAddr processId, RAddr vAddr, LineTag tag){
	processId.strictExpect(getMMU().procAW());
	vAddr.strictExpect(m_tables[0]->getVAddrWidth());

	Line *discoveredEntry = NULL; //MLDTODO Remove after debugging

	for(Table *table : m_tables){
		RAddr truncatedAddr = vAddr.truncateLsb(vAddr.m_width - table->getVAddrWidth());
		Line *line = table->lookup(processId, truncatedAddr, tag);
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
	addr.strictExpect(getMMU().vAW() - getMinOffsetSize());

	for(Table *table : m_tables){
		std::cout << "Truncate to: " << unsigned(table->getVAddrWidth());
		RAddr tableAddr = addr.truncateLsb(table->getOffsetWidth() - getMinOffsetSize());
		std::cout << " result: " << tableAddr << std::endl;
		table->freeLines(pid, &tableAddr);
	}
}

Result TLB::onInvalidateMsg(RemoteMessage &msg){
	assert(msg.type == msg.Type::MSG_TLB_INVALIDATE);

	RAddr addr = RAddr(msg.tlbInvalidate.addr, getMMU().vAW() - getMinOffsetSize());
	RAddr processId = RAddr(msg.tlbInvalidate.processId, getMMU().procAW());
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
	assert(msg.type == msg.Type::MSG_TLB_SET_PROPERTY);
	assert(msg.tlbProperty.tlb == TlbType::DTLB);

	if(msg.tlbProperty.type == TlbPropertyMsgType::ENABLED){
		assert(msg.tlbProperty.value <= 1);
		m_enabled = msg.tlbProperty.value;
	}else if(msg.tlbProperty.type == TlbPropertyMsgType::MANAGER_ADDRESS){
		m_managerAddr = msg.tlbProperty.value;
	}else if(msg.tlbProperty.type == TlbPropertyMsgType::PT_ADDRESS){
		m_tableAddr = msg.tlbProperty.value;
	}

	return Result::SUCCESS;
}

Result TLB::onStoreMsg(RemoteMessage &msg){
	assert(msg.type == msg.Type::MSG_DTLB_STORE);
	assert(msg.dTlbStore.table < m_numTables);

	Table *table = m_tables[msg.dTlbStore.table];
	RAddr d$lineId;
	RAddr pAddr = RAddr(msg.dTlbStore.pAddr, table->getPAddrWidth());
	RAddr lineIndex = RAddr(msg.dTlbStore.lineIndex, m_tables[0]->getIndexWidth());

	Result res;
	if(msg.dTlbStore.table == 0){
		res = table->storeNormal(lineIndex, msg.dTlbStore.read, msg.dTlbStore.write, pAddr, d$lineId);
	}else{
		RAddr vAddr = RAddr(0, m_tables[0]->getVAddrWidth());
		RAddr processId = RAddr(0, getMMU().procAW());

		res = m_tables[0]->getPending(lineIndex, processId, vAddr, d$lineId);
		if(res != Result::SUCCESS){
			return Result::FAILED;
		}

		vAddr = vAddr.truncateLsb(table->getOffsetWidth() - m_tables[0]->getOffsetWidth());

		res = table->storeNormal(processId, vAddr, pAddr, msg.dTlbStore.read, msg.dTlbStore.write);
		if(res == Result::SUCCESS){
			m_tables[0]->releasePending(lineIndex);
		}
	}

	if(res != Result::SUCCESS){
		return Result::FAILED;
		//MLDTODO-DOC Continuatie gegarandeerd zolang een line enkel vanaf netwerk gelocked kan worden. (Line van table <> 0 kan niet pending zijn)
	}

	std::cout << "INFORM D$, LINEID " << d$lineId << std::endl; //MLDTODO Inform D$

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
	out << "    From Pipeline:\n";
	out << "         write sim-l(ookup)     <values>\n";
	out << std::endl;
	out << "    From Manager:\n";
	out << "         write sim-i(nvalidate)\n";
	out << "            <filterPid> <ProcessId> <filterAddr> <Addr>\n";
	out << std::endl;
	out << "         write sim-p(roperty)\n";
	out << "            <ENABLED|PTA|MA> <value>\n";
	out << std::endl;
	out << "         write sim-s(tore)\n";
	out << "            <TableId> <LineId> <PAddr> <read> <write>\n";
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
		arg.expect(4);
		//lookup(procid, vaddr, d$lineid, r, w, paddr, mayunlock)
		//lookup(procid, vaddr, mayunlock)
		//lookup(d$lineid, r, w, paddr)

		RAddr procId = arg.getRMAddr(1, getMMU().procAW());
		RAddr vAddr = arg.getRMAddr(2, getMMU().vAW() - getMinOffsetSize());
		bool mayUnlock = arg.getBool(3);

		bool r;
		bool w;
		RAddr pAddr(0, getMMU().pAW() - getMinOffsetSize());
		RAddr d$LineIndex(0, getMMU().netAW());

		Result res = lookup(procId, vAddr, d$LineIndex, r, w, pAddr, mayUnlock);

		out << "Simulated lookup with result " << res << std::endl;
		if(res == Result::SUCCESS){
			out << "   d$LineIndex: " << d$LineIndex << std::endl;
			out << "   read: " << r << std::endl;
			out << "   write: " << w << std::endl;
			out << "   pAddr: " << pAddr << std::endl;
		}
	}else if(arg.is(0, false, "sim-i", "sim-invalidate")){
		arg.expect(5);

		RemoteMessage msg;
		msg.type = msg.Type::MSG_TLB_INVALIDATE;
		arg.set(1, msg.tlbInvalidate.filterPid);
		arg.set(3, msg.tlbInvalidate.filterAddr);
		msg.tlbInvalidate.processId = arg.getMAddr(2, getMMU().procAW());
		msg.tlbInvalidate.addr = arg.getMAddr(4, getMMU().vAW());

		Result res = onInvalidateMsg(msg);
		out << "Simulated invalidation message with result: " << res << std::endl;
	}else if(arg.is(0, false, "sim-p", "sim-property")){
		arg.expect(3);

		RemoteMessage msg;
		msg.type = msg.Type::MSG_TLB_SET_PROPERTY;
		msg.tlbProperty.tlb = TlbType::DTLB;
		msg.tlbProperty.type = getTlbPropertyMsgType(arg.getString(1, false));

		switch (msg.tlbProperty.type) {
			case TlbPropertyMsgType::ENABLED:
				msg.tlbProperty.value = arg.getBool(2);
				break;
			case TlbPropertyMsgType::PT_ADDRESS:
				msg.tlbProperty.value = arg.getMAddr(2, getMMU().pAW());
				break;
			case TlbPropertyMsgType::MANAGER_ADDRESS:
				msg.tlbProperty.value = arg.getMAddr(2, getMMU().netAW());
				break;
			default: UNREACHABLE
		}

		Result res = onPropertyMsg(msg);
		out << "Simulated property message with result: " << res << std::endl;
	}else if(arg.is(0, false, "sim-s", "sim-store")){
		arg.expect(6);

		RemoteMessage msg;
		msg.type = msg.Type::MSG_DTLB_STORE;
		msg.dTlbStore.table = arg.getMAddr(1);
		msg.dTlbStore.lineIndex = arg.getMAddr(2);
		msg.dTlbStore.pAddr = arg.getMAddr(3, getMMU().procAW());
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
