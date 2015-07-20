#ifndef ARCH_DEV_JTAG_MGRTESTER_H_
#define ARCH_DEV_JTAG_MGRTESTER_H_

#include "../../../sim/kernel.h"
#include <arch/drisc/RemoteMessage.h>
#include "JTAG.h"
#include "RefillMessage.h"
#include "PTBuilder.h"

namespace Simulator {
class DRISC;

using drisc::RemoteMessage;

struct mr_miss{	// Will be 8-byte aligned, totals 16 bytes.
	uint16_t 	isInvalidate: 1; //Must be 0
	uint16_t	_padding1	: 14; //Must be 0
	uint16_t 	tlbType 	: 1;
	uint16_t	lineIndex;
	uint16_t 	processId;
	uint16_t 	dest;
	uint64_t	vAddr;
};

class MgrTester: public Object {
public:
	MgrTester(const std::string& name, JTAG& parent);
	void start();
	int mgrTick();

	void in_MissMessage(const RemoteMessage &msg);

private:
	PTBuilder m_builder;

};

extern "C" void out_RefillMessage(const tlbRefillMsg *msg);


} /* namespace Simulator */

#endif /* ARCH_DEV_JTAG_MGRTESTER_H_ */
