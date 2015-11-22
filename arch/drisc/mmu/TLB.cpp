#include "TLB.h"

#include <stddef.h>
#include <cstring>

#include "sim/config.h"
#include <algorithm>
#include <assert.h>
#include "arch/drisc/DRISC.h"
#include "sim/inputconfig.h"


namespace Simulator {
namespace drisc {
namespace mmu {

//MLDTODO-DOC Andere ontdekkingen: RAddr is handig voor testen, maar maakt alles gecompliceert...

//MLDTODO-DOC On reserve of +L-P entry: What to do if the pickDestination algo returns a locked entry? Nothing for now...
bool TLB::OnReadRequestReceived(IODeviceID from, MemAddr address, MemSize size){
	COMMIT{
		//MLDTODO Handle
		std::cout << "Unexpected read request on " << GetName() << "from device " << from << ", Address " << address << ", size " << size << std::endl;
	}

    IOData iodata;
    iodata.size = size;
    if(address > 24){
        memset(iodata.data, 0, size);
    }else{
        memset(iodata.data, 255, size);
    }
    if (!m_ioBus.SendReadResponse(m_ioDevId, from, address, iodata))
    {
        DeadlockWrite("Unable to send ROM read response to I/O bus");
        return false;
    }
    return true;
}

TLB::TLB(const std::string& name, Object& parent, IIOBus* iobus)
	: Object(name, parent),
	InitProcess(p_transmit, doTransmit),
	m_fifo_out("m_fifo_out", *this, iobus->GetClock(), 10, 4),

	InitProcess(p_receive, doReceive),
	InitBuffer(m_fifo_in, iobus->GetClock(), "inFifoSize"),
	InitStorage(m_receiving, iobus->GetClock(), false),
	m_mgtMsgBuffer(),
	InitProcess(p_dummy, DoNothing),

	m_ioDevId(GetConfOpt("DeviceID", IODeviceID, iobus->GetNextAvailableDeviceID())),
	m_ioBus(*iobus),
	m_mgtAddr({0,2}),
	m_numTables(GetConf("NumberOfTables", size_t)),
	m_tables(m_numTables),
	m_enabled(false),
	m_lastLine(nullptr)
{
	//By using offsetWidth (e.g. 12) instead of offset (e.g. 4KB), we guarantee the offset
	//is always a power of two, as is required for the TLB to function.

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

	m_ioBus.RegisterClient(m_ioDevId, *this);

    m_fifo_out.Sensitive(p_transmit);
    m_fifo_in.Sensitive(p_dummy);
    m_receiving.Sensitive(p_receive);

    std::cout << "TLB " << GetName() << " initialised with IO devID " << m_ioDevId << std::endl;
}

TLB::~TLB(){
	auto lambda = [](Table *table){delete table;};
	std::for_each(m_tables.begin(), m_tables.end(), lambda);
}

Result TLB::doTransmit(){
	IoMsg item = m_fifo_out.Front();
	m_fifo_out.Pop();

	if(!m_ioBus.SendNotification(item.addr.devid, item.addr.chan, item.payload)){
		DeadlockWrite("Could not send message");
		DebugTLBWrite("message 1 not send");
		return FAILED;
	}

	return SUCCESS;
}

Result TLB::doReceive(){
	if(m_fifo_in.Empty()){
		return m_receiving.Clear() ? SUCCESS : FAILED;
	}

	IoMsg item = m_fifo_in.Front();
	m_fifo_in.Pop();
	unsigned addr = item.addr.chan / 8;

	if(addr <= 1){
		assert(item.addr.devid == m_mgtAddr.devid);

		m_mgtMsgBuffer.data.part[addr] = item.payload;

		if(addr == 0){
			return handleMgtMsg(m_mgtMsgBuffer);
		}
	}else if(addr == 2){
		MgtMsg singleMessage;
		singleMessage.data.part[0] = item.payload;
		assert(singleMessage.type == (uint64_t)MgtMsgType::SET);
		return handleMgtMsg(singleMessage);
	}else{
		UNREACHABLE;
	}
	return Result::SUCCESS;
}

Result TLB::handleMgtMsg(MgtMsg msg){
	if(msg.type == (uint64_t)MgtMsgType::REFILL){
		return onStoreMsg(msg);
	}else if(msg.type == (uint64_t)MgtMsgType::INVALIDATE){
		return onInvalidateMsg(msg);
	}else if(msg.type == (uint64_t)MgtMsgType::SET){
		return onPropertyMsg(msg);
	}else{
		UNREACHABLE
	}
}

bool TLB::sendMgtMsg(MgtMsg msg){
	IoMsg msg0, msg1;

	msg0.addr = msg1.addr = m_mgtAddr;
	msg0.payload = msg.data.part[0];
	msg1.payload = msg.data.part[1];

	if(!m_fifo_out.Push(msg1) || !m_fifo_out.Push(msg0)){
		DebugTLBWrite("message 2 not send");
		return false;
	}
	DebugTLBWrite("message 2 SEND");


	return true;
}

bool TLB::OnWriteRequestReceived(IODeviceID from, MemAddr address, const IOData& data){
	IoMsg msg;
	msg.addr.devid = from;
	msg.addr.chan = address;
	msg.payload = *((uint64_t*)data.data);

	if(!m_fifo_in.Push(msg)){ return false; }

	m_receiving.Set();
	return true;
}

// SUCCESS: Address in TLB
// DELAYED: Address not in TLB, expecting refill
// FAILED:  Stalled or Address not in TLB and unable to transmit refill request
// Expects address including bits within page (offset)
Result TLB::lookup(Addr processId, Addr vAddr, bool mayUnlock, TLBResultMessage &res){
	if(!m_enabled){
		//DebugTLBWrite("Lookup request for %lu:0x%lX looped back, TLB disabled", processId, vAddr);
		return loopback(processId, vAddr, res);
	}else if(vAddr >> 63){
		DebugTLBWrite("Lookup request for %lu:0x%lX looped back, TLS address", processId, vAddr);
		return loopback(processId, vAddr, res);
	}

	unsigned dcache_linesize = GetDRISCParent()->GetDCache().GetLineSize();
	if(GetDRISCParent()->isMapped(vAddr, dcache_linesize)){
		if(
				(vAddr < 0x440000)
				|| (vAddr > 0x550000 && vAddr  < 0x200000000)
				|| (					vAddr >= 0x600000000)
		){
			DebugTLBWrite("Lookup request for %lu:0x%lX looped back, mapped to old-style Virtual Memory", processId, vAddr);
			return loopback(processId, vAddr, res);
		}else{
			throw exceptf<std::logic_error>("Address 0x%lX is both within TLB reserved space and is mapped to old-style Virtual Memory", vAddr);
		}
	}

	RAddr rProcessId = RAddr(processId, getMMU().procAW());
	RAddr rVAddr = RAddr::truncateLsb(vAddr, getMMU().vAW(), m_tables[0]->getOffsetWidth());

	DebugTLBWrite("Handling lookup request for %lu:0x%lX (%lu:0x%lX)", processId, rVAddr.m_value, processId, vAddr);

	return lookup(rProcessId, rVAddr, mayUnlock, res);
}

Result TLB::loopback(Addr /*processId*/, Addr vAddr, TLBResultMessage &res){
	m_lastLine = nullptr;
	res.pAddr = vAddr;
	res.tlbOffsetWidth = 0;
	res.pending = false;
	res.read = true;
	res.write = true;
	return SUCCESS;
}

Result TLB::lookup(RAddr processId, RAddr vAddr, bool mayUnlock, TLBResultMessage &res){
	vAddr.strictExpect(m_tables[0]->getVAddrWidth());

	TlbLineRef ref = doLookup(processId, vAddr, LineTag::INDEXABLE);
	Line* line = ref.m_line;
	Table* table = ref.m_table;
	if(line != NULL && line->present == false){
		res.pending = true;
		res.write = false;
		res.read = false;
		res.tlbOffsetWidth = 0;
		res.pAddr = 0;
		m_lastLine = line;
		DebugTLBWrite("Lookup request for %lu:0x%lX is already pending...", processId.m_value, vAddr.m_value);

		return DELAYED;
	}
	if(line == NULL){
		Addr tableLineId;

		if(!m_tables[0]->storePending(processId, vAddr, tableLineId, line)){
			DeadlockWrite("Unable to store pending entry in TLB");
			return FAILED;
		}

		MgtMsg msg;
		msg.type = (uint64_t)MgtMsgType::MISS;
		msg.mReq.vAddr = vAddr.m_value;
		msg.mReq.contextId = processId.m_value;
		msg.mReq.lineIndex = tableLineId;
		msg.mReq.tlbType = (uint64_t)manager::TlbType::DTLB;
		msg.mReq.caller = m_ioDevId;

		if(!sendMgtMsg(msg)){
			DeadlockWrite("Unable to send request to memory manager");
			return FAILED;
		}

		res.pending = true;
		res.write = false;
		res.read = false;
		res.tlbOffsetWidth = 0;
		res.pAddr = 0;
		m_lastLine = line;

		DebugTLBWrite("Lookup request for %lu:0x%lX delayed, waiting for requested refill", processId.m_value, vAddr.m_value);
		return Result::DELAYED;
	}

	if(line->locked && mayUnlock){
		COMMIT{line->locked = false;}
	}

	res.pending = false;
	res.write = line->write;
	res.read = line->read;
	res.tlbOffsetWidth = table->getOffsetWidth();
	res.pAddr = line->pAddr.m_value;
	m_lastLine = NULL;
	return SUCCESS;

	DebugTLBWrite("Lookup request for %lu:0x%lX success: 0x%lx, r=%d, w=%d", processId.m_value, vAddr.m_value, line->pAddr.m_value, line->read, line->write);

	return Result::SUCCESS;
}

TLBResultMessage TLB::getLine(TlbLineRef lineRef){
	assert(lineRef.m_line->locked == true);
	assert(lineRef.m_line->present == true);

	TLBResultMessage msg;

	COMMIT{	lineRef.m_line->locked = false; }

	msg.pAddr = lineRef.m_line->pAddr.m_value;
	msg.pending = false;
	msg.read = lineRef.m_line->read;
	msg.write = lineRef.m_line->write;
	msg.tlbOffsetWidth = lineRef.m_table->getOffsetWidth();

	return msg;
}

TlbLineRef TLB::doLookup(RAddr processId, RAddr vAddr, LineTag tag){
	processId.strictExpect(getMMU().procAW());
	vAddr.strictExpect(m_tables[0]->getVAddrWidth());

	TlbLineRef discoveredEntry{0,0}; //MLDTODO Remove after debugging

	for(Table *table : m_tables){
		RAddr truncatedAddr = vAddr.truncateLsb(vAddr.m_width - table->getVAddrWidth());
		Line *line = table->lookup(processId, truncatedAddr, tag);
		if(line != NULL){
			if(discoveredEntry.m_line != NULL){ //MLDTODO Remove after debugging
				throw exceptf<std::domain_error>("vAddr %lX exists in multiple tables!", vAddr.m_value);
			}
			discoveredEntry.m_line = line;
			discoveredEntry.m_table = table;
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
	addr.strictExpect(getMMU().vAW() - getMinOffsetWidth());

	for(Table *table : m_tables){
		std::cout << "Truncate to: " << unsigned(table->getVAddrWidth());
		RAddr tableAddr = addr.truncateLsb(table->getOffsetWidth() - getMinOffsetWidth());
		std::cout << " result: " << tableAddr << std::endl;
		table->freeLines(pid, &tableAddr);
	}
}

void TLB::setDCacheReference(unsigned dRef){
	assert(m_lastLine != NULL);
	assert(!m_lastLine->present);
	m_lastLine->d$lineRef = dRef;
}

Result TLB::onInvalidateMsg(MgtMsg &msg){
	RAddr addr = RAddr(msg.iReq.vAddr, getMMU().vAW() - getMinOffsetWidth());
	RAddr context = RAddr(msg.iReq.contextId, getMMU().procAW());

	if(msg.iReq.filterVAddr && msg.iReq.filterContext){
		COMMITCLI{ invalidate(context, addr); }
	}else if(!msg.iReq.filterVAddr && msg.iReq.filterContext){
		COMMITCLI{ invalidate(context); }
	}else if(!msg.iReq.filterVAddr && !msg.iReq.filterContext){
		COMMITCLI{invalidate(); }
	}else{
		UNREACHABLE
	}

	return Result::SUCCESS;
}

Result TLB::onPropertyMsg(MgtMsg &msg){
	assert(msg.type == (uint64_t)MgtMsgType::SET);

	if(msg.set.property == (uint64_t)manager::SetType::SET_STATE_ON_TLB){
		COMMITCLI{
			if(msg.set.val1){
				DebugTLBWrite("TLB enabled by IO Message");
				m_enabled = true;
			}else{
				DebugTLBWrite("TLB disabled by IO Message");
				m_enabled = false;
			}
		}
	}else if(msg.set.property == (uint64_t)manager::SetType::SET_MGT_ADDR_ON_TLB){
		COMMITCLI{
			m_mgtAddr.devid = msg.set.val1;
			m_mgtAddr.chan = msg.set.val2;
			DebugTLBWrite("TLB manager address set to %u channel %u by IO Message", m_mgtAddr.devid, m_mgtAddr.chan);
		}
	}else{
		UNREACHABLE
	}

	return Result::SUCCESS;
}

Result TLB::onStoreMsg(MgtMsg &msg){
	assert(msg.type == (uint64_t)MgtMsgType::REFILL);
	assert(msg.refill.table < m_numTables);

	DebugTLBWrite("Handling store message");

	DCache& cache = GetDRISCParent()->GetDCache();

	char tableId = (m_tables.size() - 1) - msg.refill.table;

	Table *table = m_tables[tableId];
	RAddr d$lineId;
	RAddr pAddr = RAddr(msg.refill.pAddr, table->getPAddrWidth());
	RAddr lineIndex = RAddr(msg.refill.lineIndex, m_tables[0]->getIndexWidth());
	RAddr vAddr;
	RAddr processId;

	if(msg.dRefill.present == 0){
		bool res;
		vAddr = RAddr(0, m_tables[0]->getVAddrWidth());
		processId = RAddr(0, getMMU().procAW());

		res = m_tables[0]->getPending(lineIndex, processId, vAddr, d$lineId);
		if(!res){
			throw exceptf<std::domain_error>("Cannot find pending tlb entry"); //MLDTODO Add information
		}
		DebugTLBWrite("Lookup request for %lu:0x%lX << %d failed: No page table entry", processId.m_value, vAddr.m_value, table->getOffsetWidth());

		COMMIT{
			//MLDTODO Generalise response to DCache to get rid of hacky TlbLineRef usage
			cache.OnTLBLookupCompleted(d$lineId.m_value, TlbLineRef{(Line*)vAddr.m_value, (Table*)processId.m_value}, false);
		}
		return SUCCESS;
	}

	TlbLineRef tlbLineRef;

	if(tableId == 0){
		DebugTLBWrite("Lookup request for %lu:0x%lX succeeded: placing in table 0", processId.m_value, vAddr.m_value);
		tlbLineRef.m_line = table->fillPending(lineIndex, msg.dRefill.read, msg.dRefill.write, pAddr, d$lineId);
		tlbLineRef.m_table = table;
	}else{
		DebugTLBWrite("Lookup request for %lu:0x%lX succeeded: placing in table %d", processId.m_value, vAddr.m_value, tableId);

		bool res;
		vAddr = RAddr(0, m_tables[0]->getVAddrWidth());
		processId = RAddr(0, getMMU().procAW());

		res = m_tables[0]->getPending(lineIndex, processId, vAddr, d$lineId);
		if(!res){
			DeadlockWrite("Cannot find pending TLB line at lineIndex %ld", lineIndex.m_value);
			return FAILED;
		}

		vAddr = vAddr.truncateLsb(table->getOffsetWidth() - m_tables[0]->getOffsetWidth());

		tlbLineRef.m_line = table->storeNormal(processId, vAddr, pAddr, msg.dRefill.read, msg.dRefill.write);
		tlbLineRef.m_table = table;
		if(tlbLineRef.m_line == NULL){
			// DeadlockWrite done in storeNormal
			return FAILED;
		}
		m_tables[0]->releasePending(lineIndex);
	}

	//MLDTODO-DOC Continuatie gegarandeerd zolang een line enkel vanaf netwerk gelocked kan worden. (Line van table <> 0 kan niet pending zijn)

	//MLDTODO D$ locken / process claimen
	COMMIT{
		cache.OnTLBLookupCompleted(d$lineId.m_value, tlbLineRef, true);
	}

	return Result::SUCCESS;
}


void TLB::Cmd_Info(std::ostream& out, const std::vector<std::string>& /* arguments */) const{
    //MLDTODO Display statistics
    out << "The TLB blablabla\n\n";
    out << "  Number of tables: " << unsigned(m_numTables) << "\n";
    out << "             State: " << (m_enabled ? "enabled." : "DISABLED!") << "\n";
    out << "   Manager address: " << m_mgtAddr.devid << " : " << m_mgtAddr.chan << "\n\n";
    Cmd_Usage(out);
}

void TLB::Cmd_Usage(std::ostream& out) const{
	out << "Supported operations:\n";
	out << "    write s(tatus)         <1|ENABLED|0|DISABLED>\n";
	out << "    write m(anagerAddr)    <value>\n";
	out << std::endl;
	out << "Simulate incomming messages:\n";
	out << "    From Pipeline:\n";
	out << "         write sim-l(ookup)\n";
	out << "            <processId> <vAddr> <mayUnlock>\n";
	out << std::endl;
	out << "    From Manager:\n";
	out << "         write sim-i(nvalidate)\n";
	out << "            <filterPid> <ProcessId> <filterAddr> <Addr>\n";
	out << std::endl;
	out << "         write sim-p(roperty)\n";
	out << "            <ENABLED|MA> <value> [value]\n";
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
		arg.expect(3);
		out << "Changed Manager Address from " << m_mgtAddr.devid << ":" << m_mgtAddr.chan;
		m_mgtAddr.devid = arg.getUnsigned(1);
		m_mgtAddr.chan = arg.getUnsigned(2);
		out << " to " << m_mgtAddr.devid << ":" << m_mgtAddr.chan << std::endl;
	}else if(arg.is(0, false, "sim-l", "sim-lookup")){
//		arg.expect(4);
//		//lookup(procid, vaddr, d$lineid, r, w, paddr, mayunlock)
//		//lookup(procid, vaddr, mayunlock)
//		//lookup(d$lineid, r, w, paddr)
//
//		RAddr procId = arg.getRMAddr(1, getMMU().procAW());
//		RAddr vAddr = arg.getRMAddr(2, getMMU().vAW());
//		vAddr >>= getMinOffsetWidth();
//		bool mayUnlock = arg.getBool(3);
//
//		TLBResult data;
//
//		Result res = lookup(procId, vAddr, mayUnlock, data);
//
//		out << "Simulated lookup with result " << res << std::endl;
//		if(res == Result::SUCCESS){
//			out << "   d$LineIndex: " << data.dcacheReference() << std::endl;
//			out << "   read: " << data.read() << std::endl;
//			out << "   write: " << data.write() << std::endl;
//			out << "   pAddr: " << data.pAddr() << std::endl;
//		}
	}else if(arg.is(0, false, "sim-i", "sim-invalidate")){
		arg.expect(5);
		MgtMsg msg;
		msg.type = (uint64_t)MgtMsgType::INVALIDATE;
		msg.iReq.filterContext = arg.getBool(1);
		msg.iReq.filterVAddr = arg.getBool(3);

		msg.iReq.contextId = arg.getMAddr(2, getMMU().procAW());
		msg.iReq.vAddr = arg.getMAddr(4, getMMU().vAW());

		Result res = onInvalidateMsg(msg);
		out << "Simulated invalidation message with result: " << res << std::endl;
	}else if(arg.is(0, false, "sim-p", "sim-property")){
		arg.expect(3);

		MgtMsg msg;
		msg.type = (uint64_t)MgtMsgType::SET;
		std::string property = arg.getString(1, false);

		if (property == "ENABLED" || property == "E") {
			msg.set.property = (uint64_t)manager::SetType::SET_STATE_ON_TLB;
			msg.set.val1 = arg.getBool(2);
		}else if (property == "MANAGER_ADDRESS" || property == "MANAGER" || property == "MA") {
			msg.set.property = (uint64_t)manager::SetType::SET_MGT_ADDR_ON_TLB;
			msg.set.val1 = arg.getMAddr(2, getMMU().netAW());
			msg.set.val2 = arg.getUnsigned(3);
		}else{
			UNREACHABLE
		}

		Result res = onPropertyMsg(msg);
		out << "Simulated property message with result: " << res << std::endl;
	}else if(arg.is(0, false, "sim-s", "sim-store")){
		arg.expect(6);

		MgtMsg msg;
		msg.type = (uint64_t)MgtMsgType::REFILL;
		msg.refill.table = arg.getMAddr(1);
		msg.refill.lineIndex = arg.getMAddr(2);
		msg.refill.pAddr = arg.getMAddr(3, getMMU().procAW());
		msg.dRefill.read = arg.getBool(4);
		msg.dRefill.write = arg.getBool(5);

		Result res = onStoreMsg(msg);
		out << "Simulated store message with result: " << res << std::endl;
	}else{
		Cmd_Usage(out);
	}
}

//unsigned TLBResult::dcacheReference(unsigned ref) {
//	assert(m_line && !m_line->present);
//	unsigned old = m_line->d$lineRef;
//	m_line->d$lineRef = ref;
//	return old;
//}
//
//Addr TLBResult::vAddr() {
//	assert(m_line);
//	//MLDTODO Hack to allow for 64-bit addresses
//	int offset = m_mmu->vAW() - m_line->vAddr.m_width;
//	offset = offset < 0 ? 0 : offset;
//	return (m_line->vAddr << (offset)).m_value;
//}
//
//Addr TLBResult::pAddr() {
//	assert(m_line && m_line->present);
//	//MLDTODO Hack to allow for 64-bit addresses
//	int offset = m_mmu->pAW() - m_line->pAddr.m_width;
//	offset = offset < 0 ? 0 : offset;
//	return (m_line->pAddr << offset).m_value;
//}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
