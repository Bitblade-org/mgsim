/*
 * MMUTester.cpp
 *
 *  Created on: 22 Jun 2015
 *      Author: nijntje
 */

#include "MMUTester.h"

namespace Simulator {

MMUTester::MMUTester(const std::string& name, JTAG& parent, MMU* mmu):
		Object(name, parent),
		m_mmu(mmu),
		m_dTlb(&(m_mmu->getDTlb()))
{}

void MMUTester::start(){

}

Result MMUTester::doStore(int lineIndex, bool r, bool w, RAddr pAddr, int table){
	return Result::FAILED;
}


} /* namespace Simulator */

