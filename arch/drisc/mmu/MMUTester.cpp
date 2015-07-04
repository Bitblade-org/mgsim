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
	RMAddr p = RMAddr::p(0);
	RMAddr v = RMAddr::v(0x10000);

	DTlbEntry e = DTlbEntry(12);
	e.read = true;
	e.write = true;

	for(int i=0; i<32; i++){
		e.processId.m_value = i;
		e.pAddr = p.truncateLsb(12);
		e.vAddr = v.truncateLsb(12);
		mmu.m_dtlb.store_entry(e);
		p.m_value += 0x1111080000;
		v.m_value += 0x1111080000;
	}

	mmu.m_dtlb.Invalidate(5);
	mmu.m_dtlb.Invalidate(10);
	mmu.m_dtlb.Invalidate(15);
	mmu.m_dtlb.Invalidate(20);
	mmu.m_dtlb.Invalidate(25);
	mmu.m_dtlb.Invalidate(30);

	e.processId.m_value = 0xFF;
	e.pAddr = RMAddr::p(0xfffffffff).truncateLsb(12);
	e.vAddr = RMAddr::v(0xffffffff).truncateLsb(12);

	mmu.m_dtlb.store_entry(e);
	mmu.m_dtlb.store_entry(e);
	mmu.m_dtlb.store_entry(e);
	mmu.m_dtlb.store_entry(e);
	mmu.m_dtlb.store_entry(e);

	assert(mmu.m_dtlb.lookup(RPAddr(0xFF), RMAddr::v(0xffffffff)) != NULL);
}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
