#ifndef ARCH_DEV_JTAG_MMUTESTER_H_
#define ARCH_DEV_JTAG_MMUTESTER_H_

#include "../../../sim/kernel.h"
#include "../../../sim/process.h"
#include "../../Address.h"
#include <arch/drisc/mmu/MMU.h>
#include <arch/drisc/mmu/TLB.h>
#include "JTAG.h"
#include "RefillMessage.h"

namespace Simulator {
using drisc::mmu::MMU;
using drisc::mmu::TLB;
class JTAG;

class MMUTester : public Object {
public:
	MMUTester(const std::string& name, JTAG& parent, MMU* mmu);
	MMUTester(const MMUTester&) = delete;

	void start();

	MMUTester& operator=(const MMUTester&) = delete;
	Result doStore(int lineIndex, bool r, bool w, RAddr pAddr, int table);

private:
	MMU* m_mmu;
	TLB* m_dTlb;	//D-TLB



	void mmuFiller();

	void doSet(TlbPropertyMsgType type, TlbType tlbType, Addr value);
	Result doLookup(RAddr const processId, RAddr const vAddr, RAddr& d$lineId, bool& r, bool& w, RAddr& pAddr, bool mayUnlock);
	void doLookupAndStore(RAddr const processId, RAddr const vAddr, bool r, bool w, RAddr pAddr, int table);




};



} /* namespace Simulator */

#endif /* ARCH_DEV_JTAG_MMUTESTER_H_ */
