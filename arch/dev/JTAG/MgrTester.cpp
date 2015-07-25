/*
 * ManagerObject.cpp
 *
 *  Created on: 17 Jul 2015
 *      Author: nijntje
 */

#include "MgrTester.h"

#include <iostream>
#include <string>

#include "../../../sim/kernel.h"
#include "../../Address.h"
#include "../../simtypes.h"
#include "MMUTester.h"



namespace Simulator {


MgrTester::MgrTester(const std::string& name, JTAG& parent):
	Object(name, parent),
	m_builder()
{}

struct pt_t;
union managerReq_t;


void MgrTester::start(){
	//m_builder.demoBuild();

	//firstPt((pt_t*)m_builder.getStart());
}

int MgrTester::mgrTick(){
	//int result = handleReq();
//	if(result == 0){
//		std::cout << "(Result: 0)" << std::endl;
//	}else if(result != 1){
//		std::cout << "Unexpected result from manager: " << result << std::endl;
//	}
//	return result;
}

void MgrTester::in_MissMessage(const RemoteMessage &msg){
	mr_miss cMsg;
	cMsg.dest = msg.TlbMissMessage.dest;
	cMsg.isInvalidate = 0;
	cMsg.lineIndex = msg.TlbMissMessage.lineIndex;
	cMsg.processId = msg.TlbMissMessage.processId;
	cMsg.tlbType = msg.TlbMissMessage.tlb == TlbType::DTLB ? 1 : 0;
	cMsg.vAddr = msg.TlbMissMessage.addr;

	int result = 0;//in_netMsg((managerReq_t*)&cMsg);
	if(result < 0){
		std::cout << "Manager cannot queue miss message, return code " << result << std::endl;
	}else if(result == 0){
		std::cout << "Manager cannot queue miss message, queue is full" << std::endl;
	}else{
		std::cout << "Manager queued the miss message. Activating Manager until queue is empty" << std::endl;
		while(result > 0){
			std::cout << "Tick...: ";
			result = mgrTick();
		}
	}

	tlbRefillMsg response = TestNet::netPopRefill();
	std::cout << "Forwarding message from manager to MMUTester" << std::endl;
//	if(!response.base.present){
//		std::cout << "Scratch that, message has !p, not implemented yet" << std::endl;
//	}else if(!response.base.dTLB){
//		std::cout << "Scratch that, message is for iTlb, not implemented yet" << std::endl;
//	}else{
//		((JTAG*)GetParent())->getMMUTester().doStore(response.d.lineIndex, response.d.read, response.d.write, RAddr(response.d.pAddr, 52), 2 - response.d.table);
//	}
}
extern "C"{
	void out_RefillMessage(const tlbRefillMsg *msg){
		TestNet::netPushRefill(*msg);
	}
}

} /* namespace Simulator */
