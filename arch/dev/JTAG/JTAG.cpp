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

#include "../../Address.h"
#include "../../drisc/RemoteMessage.h"
#include "MMUTester.h"
#include "MgrTester.h"

namespace Simulator {
using drisc::RemoteMessage;

tlbRefillMsg TestNet::s_netMsg;
int JTAG::s_indent = 0;

JTAG::JTAG(const std::string& name, Object& parent):
		Object(name, parent),
		m_mmuTester(0),
		m_mgrTester(new MgrTester("mgrTester", *this)),
		m_testNet("testNet", *this)
{
	if(&(JTAG::getJTAG()) != NULL){
		throw std::logic_error("What part of \"And one JTAG to rule them all\" don't you understand?");
	}

	getJTAG(this);
}

JTAG& JTAG::getJTAG(JTAG* ref){
	static JTAG* jtag = 0;

	if(ref != NULL){
		jtag = ref;
	}

	return *jtag;
}

void JTAG::start(){
	m_mgrTester->start();

	Object* cpu = findObject(GetParent(), "cpu" + std::to_string(JTAG_CPU));
	assert(cpu != NULL);
	MMU* mmu = (MMU*)findObject(cpu, "cpu" + std::to_string(JTAG_CPU) + ".mmu");
	assert(mmu != NULL);
	m_mmuTester = new MMUTester("mmuTester", *this, mmu);
	m_mmuTester->start();
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

void TestNet::mgrPush(const RemoteMessage &msg){
	assert(msg.type == RemoteMessage::MSG_TLB_MISS_MESSAGE);
	assert(msg.TlbMissMessage.dest == 42);

	std::cout << std::setw(JTAG::s_indent * 4) << " " << "Relaying message to MgrTester: " << msg.str() << std::endl;

	JTAG::getJTAG().getMgrTester().in_MissMessage(msg);
}


void TestNet::netPushRefill(const tlbRefillMsg &msg){

	s_netMsg = tlbRefillMsg(msg);

	std::cout << std::setw(JTAG::s_indent * 4) << " " << "Refill message posted to testing \"network\"" << std::endl;
}

tlbRefillMsg TestNet::netPopRefill(){

	std::cout << std::setw(JTAG::s_indent * 4) << " " << "Refill message relayed from testing \"network\"" << std::endl;

	return s_netMsg;
}

void TestNet::d$Push(Addr line){
	std::cout << std::setw(JTAG::s_indent * 4) << " " << "TLB wants to inform D$ about progress for line " << line << std::endl;
}

} /* namespace Simulator */
