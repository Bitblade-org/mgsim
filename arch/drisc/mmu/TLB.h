#ifndef ARCH_DRISC_MMU_TLB_H_
#define ARCH_DRISC_MMU_TLB_H_

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <sim/inspect.h>
#include <arch/simtypes.h>

#include "Table.h"

namespace Simulator {
namespace drisc {
namespace mmu {

//MLDNOTE Not extending MMIOComponent. TLB is not a memory component itself. Table probably is.
class TLB :	public Object, public Inspect::Interface<Inspect::Info | Inspect::Read> {

//MLDTODO Keep statistics!
public:
    TLB(const std::string& name, Object& parent);
    ~TLB();

    Result lookup(RPAddr const processId, RMAddr const vAddr, int const d$line, bool *r, bool *w, RMAddr *pAddr);
    MWidth getMinOffsetSize(){return m_tables[0]->getOffsetWidth();}

    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void Cmd_Read(std::ostream& out, const std::vector<std::string>& arguments) const override;

private:
    void unlink(DTlbEntry &entry);
    void Invalidate();
    void Invalidate(RPAddr pid);
    void Invalidate(RPAddr pid, RMAddr addr);

    Result store_entry(DTlbEntry &entry);
    Result store_pending_entry(RPAddr processId, RMAddr vAddr, int D$line);

    DTlbEntry* find(RPAddr processId, RMAddr vAddr);

    MMU					&m_mmu;
    uint8_t	const		m_numTables;
    std::vector<Table*>	m_tables;
};

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_TLB_H_ */
