/*
 * JTAG.cpp
 *
 *  Created on: 18 Jul 2015
 *      Author: nijntje
 */

#include "JTAG.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <sim/process.h>

#include "../../Address.h"
#include "MMUTester.h"
#include "MgrTester.h"

namespace Simulator {
using drisc::RemoteMessage;
using manager::MgtMsg;

tlbRefillMsg TestNet::s_netMsg;
int JTAG::s_indent = 0;

JTAG::JTAG(const std::string& name, Object& parent, IIOBus& iobus, IODeviceID devid):
		Object(name, parent),
		m_fifo_out("m_fifo_out", *this, iobus.GetClock(), 10, 4),
		InitBuffer(m_fifo_in, iobus.GetClock(), "inFifoSize"),
		InitProcess(p_transmit, doTransmit),
		InitProcess(p_receive, doReceive),
		InitStorage(m_receiving, iobus.GetClock(), false),
		m_mmuTester(0),
		m_mgrTester(new MgrTester("mgrTester", *this)),
		m_testNet("testNet", *this),
		m_ioDevId(devid),
		m_ioBus(iobus),
		m_mgtAddr({0,2}),
		m_mgtMsgBuffer(),
		InitProcess(p_dummy, DoNothing)
{
	if(&(JTAG::getJTAG()) != NULL){
		throw std::logic_error("What part of \"And one JTAG to rule them all\" don't you understand?");
	}

	m_ioBus.RegisterClient(m_ioDevId, *this);

	getJTAG(this);

    m_fifo_out.Sensitive(p_transmit);
    m_fifo_in.Sensitive(p_dummy);
    m_receiving.Sensitive(p_receive);

    std::cout << "JTAG " << GetName() << " initialised with IO devID " << m_ioDevId << std::endl;

    //m_transmitting.Sensitive(p_transmit);
}

JTAG& JTAG::getJTAG(JTAG* ref){
	static JTAG* jtag = 0;

	if(ref != NULL){
		jtag = ref;
	}

	return *jtag;
}

//MLDTODO Figure out where to do this
//if (!DeviceDatabase::GetDatabase().FindDeviceByName("MGSim", "UART", id))
//{
//    throw InvalidArgumentException(*this, "Device identity not registered");
//}

void JTAG::start(){
	m_mgrTester->start();

	Object* cpu = findObject(GetParent(), "cpu" + std::to_string(JTAG_CPU));
	assert(cpu != NULL);
	MMU* mmu = (MMU*)findObject(cpu, "cpu" + std::to_string(JTAG_CPU) + ".mmu");
	assert(mmu != NULL);
	m_mmuTester = new MMUTester("mmuTester", *this, mmu);
	m_mmuTester->start();
}

Result JTAG::doTransmit(){
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

Result JTAG::doReceive(){
	if(m_fifo_in.Empty()){
		m_receiving.Clear();
		return Result::SUCCESS;
	}

	IoMsg item = m_fifo_in.Front();
	m_fifo_in.Pop();
	unsigned addr = item.addr.chan / 8;
	if(addr == 5){
		COMMIT{
			m_mgtAddr.devid = item.addr.devid;
			m_mgtAddr.chan = item.payload;
			startMgrTest();
		}
		return SUCCESS;
	}
	assert(addr <= 1);
	assert(item.addr.devid == m_mgtAddr.devid);

	COMMIT{
		m_mgtMsgBuffer.data.part[addr] = item.payload;

		if(addr == 0){
			handleMgtMsg(m_mgtMsgBuffer);
		}
	}

	return Result::SUCCESS;
}

void JTAG::startMgrTest(){
	sectionStart("MgrTest");

	std::cout << "Manager detected on id: " << m_mgtAddr.devid << ", channel:" << m_mgtAddr.chan << std::endl;
	MgtMsg msg;
//	msg.set.property = (uint64_t)manager::SetType::SET_PT_ON_MGT;
//	msg.set.val0 = 0x123456;
//	msg.type = (uint64_t)MgtMsgType::SET;
//	sendMgtMsg(msg);

	msg.mReq.tlbType = 1;
	msg.mReq.caller = m_ioDevId;
	msg.mReq.contextId = 0x42;
	msg.mReq.lineIndex = 42;
	msg.mReq.vAddr = 0x424242;
	msg.mReq.type = (uint64_t)MgtMsgType::MISS;

	sendMgtMsg(msg);
	sectionEnd("MgrTest");
}


void JTAG::handleMgtMsg(MgtMsg msg){
	printHeader("handleMgtMsg");

	std::cout << "Message received! Type: " << unsigned(msg.type) << std::endl;
	std::cout << "Present: " << msg.iRefill.present << std::endl;
	std::cout << "Execute: " << msg.iRefill.execute << std::endl;
	std::cout << "Table: " << msg.iRefill.table << std::endl;
	std::cout << "LineIndex: " << msg.iRefill.lineIndex << std::endl;
	std::cout << "PAddr: " << std::hex << "0x" << msg.iRefill.pAddr << std::endl;

	msg.iRefill.pAddr++;
	sendMgtMsg(msg);
}


Result JTAG::sendMgtMsg(MgtMsg msg){
	IoMsg msg0, msg1;

	msg0.addr = msg1.addr = m_mgtAddr;
	msg0.payload = msg.data.part[0];
	msg1.payload = msg.data.part[1];

	if(!m_fifo_out.Push(msg1)){
		std::cout << "PUSH of msg1 failed" << std::endl;
		return Result::FAILED;
	}

	if(!m_fifo_out.Push(msg0)){
		std::cout << "PUSH of msg0 failed" << std::endl;
		return Result::FAILED;
	}

	std::cout << "PUSH of msg1 and msg0 SUCCESS" << std::endl;
	return Result::SUCCESS;
}

bool JTAG::OnWriteRequestReceived(IODeviceID from, MemAddr address, const IOData& data){
	IoMsg msg;
	msg.addr.devid = from;
	msg.addr.chan = address;
	msg.payload = *((uint64_t*)data.data);

	if(!m_fifo_in.Push(msg)){
		return false;
	}

	m_receiving.Set();

	return true;
}


Object* JTAG::findObject(Object* base, const std::string& name){
	unsigned nrChildren = base->GetNumChildren();

	for(unsigned i=0; i<nrChildren; i++){
		Object* child = base->GetChild(i);
		if(child->GetName() == name){
			return child;
		}
	}
	return NULL;
}

void JTAG::processTester(){
	sectionStart("Process Tester");

	Process* process = GetKernel()->GetActiveProcess();

	if(process == NULL){
		std::cout << std::setw(s_indent * 4) << " " << "GetKernel()->GetActiveProcess() returned NULL poiner" << std::endl;
	}else{
		std::cout << std::setw(s_indent * 4) << " " << " Process name: " << process->GetName() << std::endl;
		std::cout << std::setw(s_indent * 4) << " " << "Process state: " << process->GetState() << std::endl;
	}
	sectionEnd("Process Tester");
}

void JTAG::sectionStart(const std::string &name){
	std::cout << std::setw(s_indent * 4) << " " << "### " << name << " ###" << std::endl;
	s_indent++;
}

void JTAG::sectionEnd(const std::string &name){
	s_indent--;
	std::cout << std::setw(s_indent * 4) << " " << "/// " << name << " \\\\\\" << std::endl;
}

void JTAG::printBanner(const std::string &text){
	std::cout 	<< "\n\n\n"
				<< "================================================================================\n"
				<< "===  " << std::left << std::setw(70) << text 						   << "  ===\n"
				<< "================================================================================\n"
				<< std::endl;
}

void JTAG::printHeader(const std::string &text){
	std::cout << std::setw(s_indent * 4) << " " << "### " << text << " ###" << std::endl;
}

TestNet::TestNet(const std::string& name, Object& parent):
		Object(name, parent)
{}

//void TestNet::mgrPush(const RemoteMessage &msg){
//	assert(msg.type == RemoteMessage::MSG_TLB_MISS_MESSAGE);
//	assert(msg.TlbMissMessage.dest == 42);
//
//	std::cout << std::setw(JTAG::s_indent * 4) << " " << "Relaying message to MgrTester: " << msg.str() << std::endl;
//
//	JTAG::getJTAG().getMgrTester().in_MissMessage(msg);
//}


//void TestNet::netPushRefill(const tlbRefillMsg &msg){
//
//	s_netMsg = tlbRefillMsg(msg);
//
//	std::cout << std::setw(JTAG::s_indent * 4) << " " << "Refill message posted to testing \"network\"" << std::endl;
//}

tlbRefillMsg TestNet::netPopRefill(){

	std::cout << std::setw(JTAG::s_indent * 4) << " " << "Refill message relayed from testing \"network\"" << std::endl;

	return s_netMsg;
}

void TestNet::d$Push(Addr line){
	std::cout << std::setw(JTAG::s_indent * 4) << " " << "TLB wants to inform D$ about progress for line " << line << std::endl;
}

} /* namespace Simulator */
