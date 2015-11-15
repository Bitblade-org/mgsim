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

class DCachePreNov : public DCache
{

public:
    DCachePreNov(const std::string& name, DRISC& parent, Clock& clock);
    DCachePreNov(const DCache&) = delete;
    DCachePreNov& operator=(const DCache&) = delete;
    ~DCachePreNov();

    Result Read2 (ContextId contextId, MemAddr address, void* data, MemSize size, RegAddr* reg) override;
    Result Write2(ContextId contextId, MemAddr address, void* data, MemSize size, LFID fid, TID tid) override;

private:
	void splitAddress(MemAddr addr, MemAddr &cacheOffset, MemAddr &cacheIndex, MemAddr *vTag);

	//Result FindLine(MemAddr address, ContextId contextId, Line* &line, bool check_only, bool ignore_tags);
	Line& fetchLine(MemAddr address) override;
	bool comparePTag(Line &line, MemAddr pTag) override;
	bool compareCTag(Line &line, CID cid) override;
	bool getEmptyLine(MemAddr address, Line* &line) override;
	bool freeLine(Line &line) override;

    //Result ReverseFindLine(MemAddr pAddr, Line* &line);

    Result DoLookupResponses() override;
    Result DoReadWritebacks() override;
    Result DoReadResponses() override;
    Result DoWriteResponses() override;
    Result DoOutgoingRequests() override;
};

}
}

#endif
