/*
 * MMUTester.cpp
 *
 *  Created on: 22 Jun 2015
 *      Author: nijntje
 */

#include "MMUTester.h"

namespace Simulator {
namespace drisc {
namespace mmu {

MMUTester::MMUTester(MMU &mmu) {
	DTlbEntry e1 = DTlbEntry(12);
	e1.pAddr = MAddr(0x0123456789ABCDEF, MAddr::PAddrWidth - 12);
	e1.vAddr = MAddr(0xFEDCBA9876543210, MAddr::VAddrWidth - 12);
	e1.processId = 0x0001;
	e1.read = true;
	e1.write = true;

	mmu.m_dtlb.store(e1);
}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
