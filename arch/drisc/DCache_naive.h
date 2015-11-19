// -*- c++ -*-
#ifndef DCACHE_NAIVE_H
#define DCACHE_NAIVE_H

#include <sim/kernel.h>
#include <sim/inspect.h>
#include <sim/buffer.h>
#include <arch/Memory.h>
#include <arch/drisc/forward.h>
#include <arch/simtypes.h>
#include "arch/drisc/mmu/MMU.h"
#include "DCache.h"

namespace Simulator
{
namespace drisc
{

class DCacheNaive : public DCache
{
public:
	typedef size_t Set;

    DCacheNaive(const std::string& name, DRISC& parent, Clock& clock);
    DCacheNaive(const DCache&) = delete;
    DCacheNaive& operator=(const DCache&) = delete;
    ~DCacheNaive();

    Result Read2 (ContextId contextId, MemAddr address, void* data, MemSize size, RegAddr* reg) override;
    ExtendedResult Write2(ContextId contextId, MemAddr address, void* data, MemSize size, LFID fid, TID tid) override;

private:
	bool getEmptyLine(size_t address, Line* &line);
	bool freeLine(Line &line);
	Line* getEmptyLine(size_t setIndex); //MLDTODO Needs beter name...
	void resetLine(Line* line);

    Result DoLookupResponses() override;
    Result DoReadWritebacks() override;
    Result DoReadResponses() override;
    Result DoWriteResponses() override;
    Result DoOutgoingRequests() override;
};

}
}

#endif
