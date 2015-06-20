#ifndef ARCH_DRISC_MMU_TABLE_H_
#define ARCH_DRISC_MMU_TABLE_H_

#include <stddef.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <sim/inspect.h>
#include <sim/kernel.h>
#include <sim/config.h>
#include <arch/simtypes.h>

namespace Simulator {
namespace drisc {
namespace mmu {

enum class EvictionStrategy {PSEUDO_RANDOM=0, ACCESSED=1, LRU=2};

//MLDTODO I should probably be extending MMIOComponent here
class Table : public Object, public Inspect::Interface<Inspect::Info | Inspect::Read>
{
	//MLDTODO Keep statistics
public:
    Table(const std::string& name, Object& parent, Config& config); //Lets try this without a clock

    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void Cmd_Read(std::ostream& out, const std::vector<std::string>& arguments) const override;

private:
   	EvictionStrategy 	m_evictionStrategy;
   	unsigned int 		m_numEntries;
   	size_t 				m_offsetSize;
};

EvictionStrategy getEvictionStrategy(std::string name);
std::ostream& operator<<(std::ostream& os, EvictionStrategy strategy);

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */

#endif /* ARCH_DRISC_MMU_TABLE_H_ */
