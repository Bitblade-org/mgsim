#include "Table.h"

#include <cstdint>

#include <sim/config.h>
#include <sim/except.h>

namespace Simulator {
namespace drisc{
namespace mmu{

Table::Table(const std::string& name, Object& parent, Config& config)
	: Object(name, parent),
	m_evictionStrategy (getEvictionStrategy(config.getValue<std::string>(*this, "EvictionStrategy"))),
	m_numEntries (config.getValue<uint64_t>(*this, "NumberOfEntries")),
	m_offsetSize (config.getValue<size_t>(*this, "OffsetSize"))
{}

void Table::Cmd_Info(std::ostream& out, const std::vector<std::string>& /* arguments */) const{
    out << "The Table blablabla\n\n";
    out << "  Eviction strategy: " << m_evictionStrategy << "\n";
    out << "        Offset size: " << m_offsetSize << " bytes\n";
    out << "    Number of lines: " << m_numEntries << "\n\n";
    out << "Supported operations:\n";
    out << "  None implemented yet!" << std::endl;

    //MLDTODO Display statistics
}

void Table::Cmd_Read(std::ostream& out, const std::vector<std::string>& /* arguments */) const{
	out << "No operations implemented yet!" << std::endl;
	//MLDTODO Semi-auto-generated stub
}

EvictionStrategy getEvictionStrategy(std::string name){
	if(name == "PSEUDO_RANDOM") {return EvictionStrategy::PSEUDO_RANDOM;}
	if(name == "ACCESSED") 		{return EvictionStrategy::ACCESSED;		}
	if(name == "LRU")			{return EvictionStrategy::LRU;			}
    throw exceptf<SimulationException>("Unknown eviction strategy");
}

std::ostream& operator<<(std::ostream& os, EvictionStrategy strategy)
{
	switch(strategy) {
         case EvictionStrategy::PSEUDO_RANDOM 	: os << "PSEUDO_RANDOM"; break;
         case EvictionStrategy::ACCESSED 		: os << "ACCESSED"; break;
         case EvictionStrategy::LRU  			: os << "LRU";  break;
         default : os.setstate(std::ios_base::failbit);
    }
    return os;
}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
