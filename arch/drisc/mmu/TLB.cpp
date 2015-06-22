#include "TLB.h"

#include <stddef.h>

#include <sim/config.h>
#include <algorithm>

namespace Simulator {
namespace drisc {
namespace mmu {

TLB::TLB(const std::string& name, Object& parent)
	: Object(name, parent),
	m_numTables(GetConf("NumberOfTables", size_t))//,

	//The number of Table objects is not known before runtime, so initialisation of each Table
	//object here is not an option

	//m_tables(m_numTables) !!!Calls default constructor which forces a call to a constructor of Object
	//Having a default constructor is not possible since the name needs to be calculated.
{
	for( int i=0; i<m_numTables; i++){
		m_tables.emplace_back(("table" + std::to_string(i)), *this);
		//Vector needs to grow, causing a call to the move constructor of Table, causing a call to
		//the move constructor of Object, which is deleted.

		//Tried a vector of pointers to Table objects created using m_tables.push_back(new ...),
		//but the pointers I retrieved using m_tables.at(i) were == NULL. Can't figure out why.
		//Same happened when using multiple steps, Table *table = new ...; m_tables.push_back(table)

		//Tried a vector of unique_ptr<Table>> objects, but had the same problems as with
		//a vector of pointers...

//		std::unique_ptr<Table> ptr(new Table("table" + std::to_string(i), *this));
//		std::cout << "Table address: " << ptr.get() << std::endl;
//
//		m_tables.push_back(std::move(ptr));
	}
}

void TLB::Invalidate(){
	auto lambda = [](Table &table){table.invalidate();};
	std::for_each(m_tables.begin(), m_tables.end(), lambda);
}

void TLB::Invalidate(PAddr pid){
	auto lambda = [pid](Table &table){table.invalidate(pid);};
	std::for_each(m_tables.begin(), m_tables.end(), lambda);
}

void TLB::Invalidate(PAddr pid, MAddr addr){
	for(Table &table : m_tables){
		MAddr tableAddr = addr.truncateLsb(table.getVAddrWidth());
		table.invalidate(pid, tableAddr);
	}
}

DTlbEntry* TLB::lookup(PAddr pid, MAddr addr){
	for(Table &table : m_tables){
		MAddr tableAddr = addr.truncateLsb(table.getVAddrWidth());
		DTlbEntry *res = table.lookup(pid, tableAddr);

		if(res != NULL){
			return res;
		}
	}
	return NULL;
}

void TLB::store(DTlbEntry &entry){
	std::cout << "TLB::store1" << std::endl;
//	for(int i=0; i<m_numTables; i++){
//		std::unique_ptr<Table> ptr = m_tables.at(i);
//		std::cout << "Table from vector, addr: " << ptr.get() << std::endl;
//	}
	for(Table &table : m_tables){
		std::cout << "Table addr: " << &table << std::endl;
		std::cout << "TLB::store2" << std::endl;
		if(table.getVAddrWidth() == entry.vAddr.m_width){
			std::cout << "TLB::store3" << std::endl;
			table.store(entry);
			return;
		}
		std::cout << "TLB::store4" << std::endl;
	}
	std::cout << "TLB::store5" << std::endl;
	//MLDTODO Throw exception!!!
	std::cerr << "Could not add DTlbEntry, cannot find table with correct addrWidth in TLB::store" << std::endl;
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
