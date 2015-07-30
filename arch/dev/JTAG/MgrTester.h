#ifndef ARCH_DEV_JTAG_MGRTESTER_H_
#define ARCH_DEV_JTAG_MGRTESTER_H_

#include "sim/kernel.h"
#include "JTAG.h"
#include "RefillMessage.h"
#include "PTBuilder.h"

namespace Simulator {

class MgrTester: public Object {
public:
	MgrTester(const std::string& name, JTAG& parent);
	void start();

private:
	PTBuilder m_builder;

};


} /* namespace Simulator */

#endif /* ARCH_DEV_JTAG_MGRTESTER_H_ */
