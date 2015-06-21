#ifndef ARCH_DRISC_MMU_MMU_H_
#define ARCH_DRISC_MMU_MMU_H_

#include <stddef.h>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include <sim/except.h>
#include <sim/inspect.h>
#include <arch/IOBus.h>
#include <arch/simtypes.h>
#include "TLB.h"

namespace Simulator {
namespace drisc {
namespace mmu {


class MMU :	public Object, public Inspect::Interface<Inspect::Info>{
public:
	//MLDTODO Keep statistics
    MMU(const std::string& name, Object& parent); //Lets try this without a clock

    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;

    bool isEnabled() const {return m_enabled;}
    MAddr getTableAddr() const {return m_tableAddr;}
    IODeviceID getManagerAddr() const {return m_managerAddr;}

    void setStatus(bool enabled) {m_enabled = enabled;}
    void setTableAddr(MAddr addr) {m_tableAddr = addr;}
    void setManagerAddr(IODeviceID addr) {m_managerAddr = addr;}

private:
    bool			m_enabled;
    MAddr			m_tableAddr;
    IODeviceID		m_managerAddr; //MLDTODO Verify type

    TLB const		m_dtlb;
    TLB	const		m_itlb;
};


} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_MMU_H_ */
