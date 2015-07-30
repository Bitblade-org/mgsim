/*
 * ManagerObject.cpp
 *
 *  Created on: 17 Jul 2015
 *      Author: nijntje
 */

#include "MgrTester.h"

#include <iostream>
#include <string>

#include "arch/Address.h"
#include "arch/simtypes.h"
#include "MMUTester.h"



namespace Simulator {


MgrTester::MgrTester(const std::string& name, JTAG& parent):
	Object(name, parent),
	m_builder()
{}

struct pt_t;
union managerReq_t;


void MgrTester::start(){

}


} /* namespace Simulator */
