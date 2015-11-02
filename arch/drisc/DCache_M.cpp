 #include <arch/drisc/DCache.h>
#include <arch/drisc/DRISC.h>
#include <sim/log2.h>
#include <sim/config.h>
#include <sim/sampling.h>

#include <cassert>
#include <cstring>
#include <iomanip>
#include <cstdio>
using namespace std;

namespace Simulator
{

namespace drisc
{

// For monitoring
//MLDTODO Remove after testing
Result DCache::Read(MemAddr address, void* data, MemSize size, RegAddr* reg){
	size_t pos = GetName().find('.');
	unsigned cpuId = std::stoul(GetName().substr(3, pos-3));
    ContextId contextId = 0; //MLDTODO Figure out where to get contextid from

	COMMIT{
		//if((address >> 63) == 0 ){//|| cpuId > 3){ //Ignore TLS
			if(cpuId > 3){
				DebugTLBWrite("Read: address: 0x%lX, size:%lu", address, size);
			}
		//}
	}

	Result result = Read2(contextId, address, data, size, reg);

	COMMIT{
		if(cpuId > 3){
			if(result == SUCCESS){
				DebugTLBWrite("Read: address: 0x%lX, size:%lu, result: %s, data: 0x%lX", address, size, resultStr(result).c_str(), *((uint64_t*)data));
			}else{
				DebugTLBWrite("Read: address: 0x%lX, size:%lu, result: %s", address, size, resultStr(result).c_str());
			}
		}
	}
	return result;
}


Result DCache::Write(MemAddr address, void* data, MemSize size, LFID fid, TID tid)
{
	size_t pos = GetName().find('.');
	unsigned cpuId = std::stoul(GetName().substr(3, pos-3));
    ContextId contextId = 0; //MLDTODO Figure out where to get contextid from

	COMMIT{
		//if((address >> 63) == 0 ){//|| cpuId > 3){ //Ignore TLS
			if(cpuId > 3){
				DebugTLBWrite("Write: address: 0x%lX, size:%lu, data: 0x%lX", address, size, *((uint64_t*)data));
			}
		//}
	}

	Result result = Write2(contextId, address, data, size, fid, tid);
	if(address == 0x801fffffffffff80){
		std::cout << "Write naar probleemadres! Data:0x" << std::hex << address << ", component:" << GetName() << std::endl;
	}

	COMMIT{
		if(cpuId > 3){
			DebugTLBWrite("Write: address: 0x%lX, size:%lu, result: %s", address, size, resultStr(result).c_str());
		}
	}
	return result;
}

bool DCache::OnMemoryReadCompleted(MemAddr addr, const char* data)
{
	//MLDTODO Remove after testing
	size_t pos = GetName().find('.');
	unsigned cpuId = std::stoul(GetName().substr(3, pos-3));

	bool result = OnMemoryReadCompleted2(addr, data);

	COMMIT{
		if(cpuId > 3){
			DebugTLBWrite("OnMemoryReadCompleted: address: 0x%lX, data: 0x%lX, result:%d", addr, *((uint64_t*)data), result);
		}
	}

	return result;
}

bool DCache::OnMemoryWriteCompleted(WClientID wid)
{

	//MLDTODO Remove after testing
	size_t pos = GetName().find('.');
	unsigned cpuId = std::stoul(GetName().substr(3, pos-3));

	bool result = OnMemoryWriteCompleted2(wid);

	COMMIT{
		if(cpuId > 3){
			DebugTLBWrite("OnMemoryWriteCompleted: wid:%u, result:%d", wid, result);
		}
	}
	return result;
}

}}
