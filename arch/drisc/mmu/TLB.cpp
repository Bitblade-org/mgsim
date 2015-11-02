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
	m_enabled(false)
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
		DeadlockWrite_("Could not send message");
		return Result::FAILED;
	}

	return Result::SUCCESS;
}

Result TLB::doReceive(){
	if(m_fifo_in.Empty()){
		return m_receiving.Clear() ? Result::SUCCESS : Result::FAILED;
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

//MLDTODO BUG push twee berichten waar maar ruimte voor een...
bool TLB::sendMgtMsg(MgtMsg msg){
	IoMsg msg0, msg1;

	msg0.addr = msg1.addr = m_mgtAddr;
	msg0.payload = msg.data.part[0];
	msg1.payload = msg.data.part[1];

	if(!m_fifo_out.Push(msg1) || !m_fifo_out.Push(msg0)){
		return false;
	}

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
Result TLB::lookup(Addr processId, Addr vAddr, bool mayUnlock, TLBResult &res){
	if(
			!m_enabled ||
			(vAddr >> 63) ||
			(vAddr < 0x440000) ||
			(vAddr > 0x550000) //||
			//(vAddr <= 0x1af87) ||
			//(vAddr >= 0x2c000 && vAddr <= 0x33917) ||
			//(vAddr >= 0x100000000 && vAddr <= 0x100007fff) ||
			//(vAddr >= 0x82000000 && vAddr < 0x85000000)
	){ //MLDTODO Generalise
		return loopback(processId, vAddr, res);
	}

	RAddr rProcessId = RAddr(processId, getMMU().procAW());
	RAddr rVAddr = RAddr::truncateLsb(vAddr, getMMU().vAW(), m_tables[0]->getOffsetWidth());

	DebugTLBWrite("Handling lookup request for %lu:0x%lX (%lu:0x%lX)", processId, rVAddr.m_value, processId, vAddr);

	return lookup(rProcessId, rVAddr, mayUnlock, res);
}

Result TLB::loopback(Addr processId, Addr vAddr, TLBResult &res){
	res.m_line = new Line(16, 64, 64);
	res.m_line->read = res.m_line->write = res.m_line->present = true;
	res.m_line->processId = processId;
	res.m_line->vAddr = vAddr;
	res.m_line->pAddr = RAddr(vAddr, 64);
	res.m_mmu = &getMMU();
	res.m_destroy = true;

	if(m_enabled){
		DebugTLBWrite("Lookup request for %lu:0x%lX looped back, TLS address", processId, vAddr);
	}else{
		//DebugTLBWrite("Lookup request for %lu:0x%lX looped back, TLB disabled", processId, vAddr);
	}

	return SUCCESS;
}

Result TLB::lookup(RAddr processId, RAddr vAddr, bool mayUnlock, TLBResult &res){
	vAddr.strictExpect(m_tables[0]->getVAddrWidth());

	Line* line = doLookup(processId, vAddr, LineTag::PRESENT);
	if(line == NULL){
		Addr tableLineId;

		if(!m_tables[0]->storePending(processId, vAddr, tableLineId, line)){
			std::cout << "Unable to store pending entry in TLB" << std::endl;
			DeadlockWrite("Unable to store pending entry in TLB");
			return Result::FAILED;
		}

		MgtMsg msg;
		msg.type = (uint64_t)MgtMsgType::MISS;
		msg.mReq.vAddr = vAddr.m_value;
		msg.mReq.contextId = processId.m_value;
		msg.mReq.lineIndex = tableLineId;
		msg.mReq.tlbType = (uint64_t)manager::TlbType::DTLB;
		msg.mReq.caller = m_ioDevId;

		if(!sendMgtMsg(msg)){
			std::cout << "Unable to send msg to Manager" << std::endl;

	        DeadlockWrite("Unable to send msg to Manager");

			return Result::FAILED;
		}

		res.m_destroy=false;
		res.m_line=line;
		res.m_mmu=&getMMU();

		DebugTLBWrite("Lookup request for %lu:0x%lX delayed, waiting for requested refill", processId.m_value, vAddr.m_value);
		return Result::DELAYED;
	}

	if(line->locked && mayUnlock){
		line->locked = false;
	}

	res = TLBResult(line, &getMMU(), false);
	DebugTLBWrite("Lookup request for %lu:0x%lX success: 0x%lx, r=%d, w=%d", processId.m_value, vAddr.m_value, line->pAddr.m_value, line->read, line->write);

	return Result::SUCCESS;
}

void TLB::getLine(TlbLineRef lineRef, TLBResult &res){
	assert(lineRef.m_line->locked == true);
	assert(lineRef.m_line->present == true);

	COMMIT{	lineRef.m_line->locked = false; }

	res = TLBResult(lineRef.m_line, &getMMU(), false);
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
	addr.strictExpect(getMMU().vAW() - getMinOffsetWidth());

	for(Table *table : m_tables){
		std::cout << "Truncate to: " << unsigned(table->getVAddrWidth());
		RAddr tableAddr = addr.truncateLsb(table->getOffsetWidth() - getMinOffsetWidth());
		std::cout << " result: " << tableAddr << std::endl;
		table->freeLines(pid, &tableAddr);
	}
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
			DeadlockWrite("Cannot find pending tlb entry"); //MLDTODO Include more info
			return Result::FAILED;
		}

		COMMIT{cache.OnTLBLookupCompleted(d$lineId.m_value, TlbLineRef{NULL}, false);}
		return SUCCESS;
	}

	TlbLineRef tlbLineRef;

	if(tableId == 0){
		tlbLineRef.m_line = table->fillPending(lineIndex, msg.dRefill.read, msg.dRefill.write, pAddr, d$lineId);
	}else{
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
		if(tlbLineRef.m_line == NULL){
			// DeadlockWrite done in storeNormal
			return FAILED;
		}
		m_tables[0]->releasePending(lineIndex);
	}

	//MLDTODO-DOC Continuatie gegarandeerd zolang een line enkel vanaf netwerk gelocked kan worden. (Line van table <> 0 kan niet pending zijn)

	//MLDTODO D$ locken / process claimen
	COMMIT{cache.OnTLBLookupCompleted(d$lineId.m_value, tlbLineRef, true);}

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
		arg.expect(4);
		//lookup(procid, vaddr, d$lineid, r, w, paddr, mayunlock)
		//lookup(procid, vaddr, mayunlock)
		//lookup(d$lineid, r, w, paddr)

		RAddr procId = arg.getRMAddr(1, getMMU().procAW());
		RAddr vAddr = arg.getRMAddr(2, getMMU().vAW());
		vAddr >>= getMinOffsetWidth();
		bool mayUnlock = arg.getBool(3);

		TLBResult data;

		Result res = lookup(procId, vAddr, mayUnlock, data);

		out << "Simulated lookup with result " << res << std::endl;
		if(res == Result::SUCCESS){
			out << "   d$LineIndex: " << data.dcacheReference() << std::endl;
			out << "   read: " << data.read() << std::endl;
			out << "   write: " << data.write() << std::endl;
			out << "   pAddr: " << data.pAddr() << std::endl;
		}
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

unsigned TLBResult::dcacheReference(unsigned ref) {
	assert(m_line && !m_line->present);
	unsigned old = m_line->d$lineRef;
	m_line->d$lineRef = ref;
	return old;
}

Addr TLBResult::vAddr() {
	assert(m_line);
	//MLDTODO Hack to allow for 64-bit addresses
	int offset = m_mmu->vAW() - m_line->vAddr.m_width;
	offset = offset < 0 ? 0 : offset;
	return (m_line->vAddr << (offset)).m_value;
}

Addr TLBResult::pAddr() {
	assert(m_line && m_line->present);
	//MLDTODO Hack to allow for 64-bit addresses
	int offset = m_mmu->pAW() - m_line->pAddr.m_width;
	offset = offset < 0 ? 0 : offset;
	return (m_line->pAddr << offset).m_value;
}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
