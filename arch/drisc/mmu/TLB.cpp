#include "TLB.h"

#include <stddef.h>

#include <sim/config.h>
#include <algorithm>
#include <assert.h>

namespace Simulator {
namespace drisc {
namespace mmu {

//MLDTODO-DOC On reserve of +L-P entry: What to do if the pickDestination algo returns a locked entry? Nothing for now...

TLB::TLB(const std::string& name, Object& parent)
	: Object(name, parent),
	m_mmu(parent),
	m_numTables(GetConf("NumberOfTables", size_t)),
	m_tables(0) 	//MLDTODO Must be a way to initialise to the right size.
{
	if(m_numTables == 0){
        throw exceptf<InvalidArgumentException>("%s must have at least one table.", name);
	}

	for( int i=0; i<m_numTables; i++){

		Table *table = new Table("table" + std::to_string(i), *this);
		if(i > 0 && table->getOffsetWidth() <= m_tables[i-1]->getOffsetWidth()){
            throw exceptf<InvalidArgumentException>("TLB tables must have ascending offset sizes.");
		}
		m_tables.emplace_back(table);
	}
}

TLB::~TLB(){
	auto lambda = [](Table *table){delete table;};
	std::for_each(m_tables.begin(), m_tables.end(), lambda);
}

// SUCCESS: Address in TLB
// DELAYED: Address not in TLB, expecting refill
// FAILED:  Stalled or Address not in TLB and unable to transmit refill request
// domain_error: TLB is disabled
Result TLB::lookup(RPAddr const processId, RMAddr const vAddr, int const d$line, bool *r, bool *w, RMAddr *pAddr){
	if(!m_mmu.isEnabled()){
		throw exceptf<std::domain_error>("TLB cannot handle lookups while disabled");
	}

	if(!processId.isValid()){
		throw exceptf<std::invalid_argument>("Process ID is not valid"); //MLDTODO see if using std::exceptions is ok.
	}

	if(!vAddr.isValid()){
		throw exceptf<std::invalid_argument>("Virtual Address is not valid"); //MLDTODO see if using std::exceptions is ok.
	}

	if(vAddr.m_width != m_tables[0]->getVAddrWidth()){
		throw exceptf<std::invalid_argument>("Virtual Address should be of width %d", m_tables[0]->getVAddrWidth()); //MLDTODO see if using std::exceptions is ok.
	}

	DTlbEntry *entry = find(processId, vAddr);
	if(entry == NULL){
		DTlbEntry dest = m_tables[0]->pickDestination();

		if(dest.locked){
			return Result::FAILED;
		}

		m_tables[0]->invalidate(dest);

		dest.processId = processId;
		dest.vAddr = vAddr;
		dest.


		dest.d$line =

	}
}

DTlbEntry* TLB::find(RPAddr processId, RMAddr vAddr){
	DTlbEntry *discoveredEntry = NULL; //MLDTODO Remove after debugging

	for(Table *table : m_tables){
		RMAddr truncatedAddr = vAddr.truncateLsb(table->getVAddrWidth());
		DTlbEntry *entry = *table->lookup(processId, truncatedAddr);
		if(entry != NULL){
			if(discoveredEntry != NULL){ //MLDTODO Remove after debugging
				throw exceptf<std::domain_error>("vAddr %lX exists in multiple tables!", vAddr);
			}
			discoveredEntry = entry;
		}
	}

	return discoveredEntry;
}

void TLB::Invalidate(){
	auto lambda = [](Table *table){table->invalidate();};
	std::for_each(m_tables.begin(), m_tables.end(), lambda);
}

void TLB::Invalidate(RPAddr pid){
	auto lambda = [pid](Table *table){table->invalidate(pid);};
	std::for_each(m_tables.begin(), m_tables.end(), lambda);
}

void TLB::Invalidate(RPAddr pid, RMAddr addr){
	for(Table *table : m_tables){
		RMAddr tableAddr = addr.truncateLsb(table->getVAddrWidth());
		table->invalidate(pid, tableAddr);
	}
}

DTlbEntry* TLB::lookup(RPAddr pid, RMAddr addr){ //MLDTODO Rename to processId and vAddr
	assert(addr.m_width == RMAddr::VirtWidth);

	for(Table *table : m_tables){
		RMAddr tableAddr = addr.truncateLsb(table->getOffsetWidth());
		DTlbEntry *res = table->lookup(pid, tableAddr);

		if(res != NULL){
			return res;
		}
	}
	return NULL;
}

//MLDTODO See if offset is already stored in D$
//MLDTODO Find the correct type for D$line
Result TLB::store_pending_lookup(RPAddr processId, RMAddr vAddr, RMAddr offset, int D$line){
	for(Table *table : m_tables){
		if(table->getVAddrWidth() == entry.vAddr.m_width){
			return table->store(entry);
		}
	}

	throw exceptf<InvalidArgumentException>(*this, "Invalid address length"); //MLDTODO Rewrite after change to msg
}

//MLDTODO Use store message with line-id
//SUCCESS: Successfully stored.
//FAIL	 : Destination entry locked
Result TLB::store_entry(DTlbEntry &entry){
	for(Table *table : m_tables){
		if(table->getVAddrWidth() == entry.vAddr.m_width){
			return table->store(entry);
		}
	}

	throw exceptf<InvalidArgumentException>(*this, "Invalid address length"); //MLDTODO Rewrite after change to msg
}

void TLB::Cmd_Info(std::ostream& out, const std::vector<std::string>& /* arguments */) const{
    out << "The TLB blablabla\n\n";
    out << "  Number of tables: " << unsigned(m_numTables) << "\n\n";
    out << "Supported operations:\n";
    out << "  None implemented yet!" << std::endl;
    //MLDTODO Display statistics
}

void TLB::Cmd_Read(std::ostream& out, const std::vector<std::string>& /* arguments */) const{
	out << "No operations implemented yet!" << std::endl;
	//MLDTODO Implement flush operation
	//MLDTODO Implement PID-flush operation
	//MLDTODO Implement invalidate operation
	//MLDTODO Implement lookup operation (local, auto, force_remote)
}

} /* namespace mmu */
} /* namespace drisc */
} /* namespace Simulator */
