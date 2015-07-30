#ifndef ARCH_DRISC_MMU_MMU_H_
#define ARCH_DRISC_MMU_MMU_H_

#include <iostream>
#include <string>
#include <vector>

#include <sim/kernel.h>
#include "../../../sim/inspect.h"
#include "../../simtypes.h"
#include "TLB.h"

namespace Simulator {
namespace drisc {
namespace mmu {

class MMU :	public Object, public Inspect::Interface<Inspect::Info>{
public:
    MMU(const std::string& name, Object& parent, AddrWidth netAddrWidth);
    MMU(const MMU &mmu) = delete;

    void initializeIO(IIOBus* iobus);

    TLB& getITLB(){return *m_itlb;}
    TLB& getDTlb(){return *m_dtlb;}

    AddrWidth getPAddrWidth() const {return m_pAddrWidth;}
    AddrWidth pAW() const {return getPAddrWidth();}
    AddrWidth getVAddrWidth() const {return m_vAddrWidth;}
    AddrWidth vAW() const {return getVAddrWidth();}
    AddrWidth getProcAddrWidth() const {return m_procAddrWidth;}
    AddrWidth procAW() const {return getProcAddrWidth();}
    AddrWidth getNetAddrWidth() const {return m_netAddrWidth;}
    AddrWidth netAW() const {return getNetAddrWidth();}

    MMU& operator=(const MMU&) = delete;
    void onInvalidateMsg(RemoteMessage &msg);
    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
private:
    const AddrWidth	m_pAddrWidth;
    const AddrWidth	m_vAddrWidth;
    const AddrWidth	m_procAddrWidth;
    const AddrWidth m_netAddrWidth;
    TLB*			m_dtlb;
    TLB*			m_itlb;

    Object& GetDRISCParent() const { return *GetParent(); } //MLDTODO Could be defined for Object.h
};


} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_MMU_H_ */
