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
#include <arch/drisc/Network.h>
#include "TLB.h"

namespace Simulator {
namespace drisc {
namespace mmu {


class MMU :	public Object, public Inspect::Interface<Inspect::Info>{
public:
    MMU(const std::string& name, Object& parent);
    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;

    TLB &getITLB(){return m_dtlb;}
    TLB &getDTlb(){return m_itlb;}

    void onInvalidateMsg(RemoteMessage &msg);

private:
    TLB 			m_dtlb;
    TLB				m_itlb;
};


} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_MMU_H_ */
