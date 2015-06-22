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
	//MLDTODO Should probably include IIOBus& iobus and other network stuff later on
    TLB(const std::string& name, Object& parent);

    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void Cmd_Read(std::ostream& out, const std::vector<std::string>& arguments) const override;

    void Invalidate();
    void Invalidate(PAddr pid);
    void Invalidate(PAddr pid, MAddr addr);
    DTlbEntry* lookup(PAddr pid, MAddr addr);
    void store(DTlbEntry &entry);

private:
    uint8_t	const		m_numTables;
    std::vector<Table>	m_tables;
};

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_TLB_H_ */
