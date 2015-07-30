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

//MLDTODO-DOC On reserve of +L-P entry: What to do if the pickDestination algo returns a locked entry? Nothing for now...
bool TLB::OnReadRequestReceived(IODeviceID from, MemAddr address, MemSize size){
	std::cout << "Unexpected read request on " << GetName() << "from device " << from << ", Address " << address << ", size " << size << std::endl;

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

	COMMIT{
		std::cout << "Transmitted IoMsg" << std::endl;
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

	assert(addr <= 1);
	assert(item.addr.devid == m_mgtAddr.devid);

		m_mgtMsgBuffer.data.part[addr] = item.payload;

		if(addr == 0){
			return handleMgtMsg(m_mgtMsgBuffer);
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
// domain_error: TLB is disabled
Result TLB::lookup(RAddr const processId, RAddr const vAddr, RAddr& d$lineId, bool& r, bool& w, RAddr& pAddr, bool mayUnlock){

	if(!m_enabled || (vAddr >> 63)){ //MLDTODO Generalise
		r = w = 1;
		pAddr = vAddr.m_value;
		return Result::SUCCESS;
	}

	processId.strictExpect();
	vAddr.strictExpect(m_tables[0]->getVAddrWidth());

	Line *line = doLookup(processId, vAddr, LineTag::PRESENT);
	if(line == NULL){
		Addr tableLineId;
		Result res = m_tables[0]->storePending(processId, vAddr, d$lineId, tableLineId);

		if(res == Result::FAILED){ return Result::FAILED; }

		MgtMsg msg;
		msg.type = (uint64_t)MgtMsgType::MISS;
		msg.mReq.vAddr = vAddr.m_value;
		msg.mReq.contextId = processId.m_value;
		msg.mReq.lineIndex = tableLineId;
		msg.mReq.tlbType = (uint64_t)manager::TlbType::DTLB;
		msg.mReq.caller = m_ioDevId;

		if(!sendMgtMsg(msg)){
			return Result::FAILED;
		}

		return Result::DELAYED;
	}

	if(line->locked && mayUnlock){
		line->locked = false;
	}

	r = line->read;
	w = line->write;
	pAddr = line->pAddr << (getMMU().pAW() - line->pAddr.m_width);

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

Result TLB::onInvalidateMsg(MgtMsg &msg){
	RAddr addr = RAddr(msg.iReq.vAddr, getMMU().vAW() - getMinOffsetSize());
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
		assert(msg.set.val0 <= 1);
		COMMITCLI{	m_enabled = msg.set.val0; }
	}else if(msg.set.property == (uint64_t)manager::SetType::SET_MGT_ADDR_ON_TLB){
		COMMITCLI{
			m_mgtAddr.devid = msg.set.val0;
			m_mgtAddr.chan = msg.set.val1;
		}
	}else{
		UNREACHABLE
	}

	return Result::SUCCESS;
}

Result TLB::onStoreMsg(MgtMsg &msg){
	assert(msg.type == (uint64_t)MgtMsgType::REFILL);
	assert(msg.refill.table < m_numTables);

	Table *table = m_tables[msg.refill.table];
	RAddr d$lineId;
	RAddr pAddr = RAddr(msg.refill.pAddr, table->getPAddrWidth());
	RAddr lineIndex = RAddr(msg.refill.lineIndex, m_tables[0]->getIndexWidth());

	Result res;
	if(msg.refill.table == 0){
		COMMITCLI{ //MLDTODO Place commit section in storeNormal
			res = table->storeNormal(lineIndex, msg.dRefill.read, msg.dRefill.write, pAddr, d$lineId);
		}
	}else{
		RAddr vAddr = RAddr(0, m_tables[0]->getVAddrWidth());
		RAddr processId = RAddr(0, getMMU().procAW());

		res = m_tables[0]->getPending(lineIndex, processId, vAddr, d$lineId);
		if(res != Result::SUCCESS){
			return Result::FAILED;
		}

		vAddr = vAddr.truncateLsb(table->getOffsetWidth() - m_tables[0]->getOffsetWidth());

		COMMITCLI{ //MLDTODO Place commit section in storeNormal
			res = table->storeNormal(processId, vAddr, pAddr, msg.dRefill.read, msg.dRefill.write);
			if(res == Result::SUCCESS){
				m_tables[0]->releasePending(lineIndex);
			}
		}
	}

	if(res != Result::SUCCESS){
		return Result::FAILED;
		//MLDTODO-DOC Continuatie gegarandeerd zolang een line enkel vanaf netwerk gelocked kan worden. (Line van table <> 0 kan niet pending zijn)
	}

	COMMIT{
		TestNet::d$Push(d$lineId.m_value); //MLDTODO Inform D$
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
		arg.expect(4);
		//lookup(procid, vaddr, d$lineid, r, w, paddr, mayunlock)
		//lookup(procid, vaddr, mayunlock)
		//lookup(d$lineid, r, w, paddr)

		RAddr procId = arg.getRMAddr(1, getMMU().procAW());
		RAddr vAddr = arg.getRMAddr(2, getMMU().vAW());
		vAddr >>= getMinOffsetSize();
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
			msg.set.val0 = arg.getBool(2);
		}else if (property == "MANAGER_ADDRESS" || property == "MANAGER" || property == "MA") {
			msg.set.property = (uint64_t)manager::SetType::SET_MGT_ADDR_ON_TLB;
			msg.set.val0 = arg.getMAddr(2, getMMU().netAW());
			msg.set.val1 = arg.getUnsigned(3);
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

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
