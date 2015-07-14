#ifndef ARCH_DRISC_MMU_TLB_H_
#define ARCH_DRISC_MMU_TLB_H_

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cassert>

#include <sim/inspect.h>
#include <cli/argument.h>
#include <arch/simtypes.h>
#include <arch/IOBus.h>
#include <arch/drisc/Network.h>

#include "Table.h"


namespace Simulator {
namespace drisc {
namespace mmu {

//MLDNOTE Not extending MMIOComponent. TLB is not a memory component itself. Table probably is.
class TLB :	public Object, public Inspect::Interface<Inspect::Info | Inspect::Write> {

//MLDTODO Keep statistics!
	/*
	 * - Number of hits
	 * - Number of misses
	 * 		- line ==, etc
	 * - Number of flushes
	 * - Number of PID invalidates
	 * - Number of vAddr+PID invalidates
	 *
	 *
	 *
	 * - Per lookup: Manager round trip time
	 */
public:
    TLB(const std::string& name, Object& parent);
    ~TLB();

    Result lookup(RAddr const processId, RAddr const vAddr, RAddr &d$line, bool &r, bool &w, RAddr &pAddr, bool mayUnlock);
    AddrWidth getMinOffsetSize(){return m_tables[0]->getOffsetWidth();}

    Result onInvalidateMsg(RemoteMessage &msg);
    Result onPropertyMsg(RemoteMessage &msg);
    Result onStoreMsg(RemoteMessage &msg);

    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void Cmd_Write(std::ostream& out, const std::vector<std::string>& arguments) override;
    void Cmd_Usage(std::ostream& out) const;


private:
    void unlink(Line &line);
    void invalidate();
    void invalidate(RAddr pid);
    void invalidate(RAddr pid, RAddr addr);

    Result store_entry(Line &line);
    Result store_pending_entry(RAddr processId, RAddr vAddr, int D$line);

    Line* doLookup(RAddr processId, RAddr vAddr, LineTag tag);

    uint8_t	const		m_numTables;
    std::vector<Table*>	m_tables;
    bool				m_enabled;
    RAddr				m_tableAddr;
    RAddr				m_managerAddr;

    Object& GetDRISCParent() const { return *GetParent()->GetParent(); }
   	MMU&	getMMU() const { return (MMU&)*GetParent(); }
};

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_TLB_H_ */
