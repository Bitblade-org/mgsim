#include "DCache_pre_nov.h"

#include <arch/drisc/DRISC.h>
#include <sim/log2.h>
#include <sim/config.h>
#include <sim/sampling.h>

#include <cassert>
#include <cstring>
#include <iomanip>
#include <cstdio>

using namespace std;


//MLDTODO-DOC MMIO
//MLDTODO Linker plaatst secties van boot.bin op onhandige locaties. Op te lossen door Virt.mem voor boot.bin?



namespace Simulator
{

namespace drisc
{

/*MLDTODO Limitations:
 * 		  - Only works for a direct mapped cache. (1-way set associative)
 * 		  - Only works for DIRECT bank selection.
 * 		  - Only works for memory systems that respond to the requesting $ only.
 * 		  Check for limitations
 * 		  MLDTODO-DOC
*/

//MLDTODO-DOC: ContextId is at least 16 bits. But the number of threads running within a single TLB's domain is not going to fill that. A simple lookup table could save an enourmous amount of storage space!

DCachePreNov::DCachePreNov(const std::string& name, DRISC& parent, Clock& clock)
:DCache(name, parent, clock)
{}

DCachePreNov::~DCachePreNov()
{}
//
////MLDTODO Create reverse lookup directory
//Result DCache::ReverseFindLine(MemAddr pAddr, Line* &line)
//{
//	if(pAddr == 0x480000ul){
//		bp();
//	}
//	//MLDTOOD Flush cache on tlb state change
//	return FindLine(pAddr, 0, line, true, true);
//
//
////    size_t offset = (size_t)(pAddr % m_lineSize);
////	MemAddr pTag = pAddr - offset;
////
////    // Find the line
////    for (size_t i = 0; i < m_assoc * m_sets; ++i)
////    {
////        line = &m_lines[i];
////
////        if (line->pTag == pTag && line->state != LINE_EMPTY)
////        {
////			return SUCCESS;
////        }
////    }
////
////    return FAILED;
//
//}

/*
 * MLDTODO Ombouwen naar VIPT+PID
 * Index op basis van (V+P)addr
 * CID tag
 * Paddr tag
 *
 * Read: Pipeline ---|---D$ vAddr lookup--CID tag comp----|--tlb pAddr compare----perm compare--->
 *                   |---TLB lookup-----------------------|
 *
 *
 * 48 bit vAddr:
 * 64 byte cache lines (6 bits addressing)
 * 4 Kbyte page offset (12 bits addressing)
 * Dus:
 * 		[52:12]				[11:6]				[5:0]
 * 		address				d$ index			d$ offset
 */

/*
 * Splits the address in the following values:
 * 	cacheOffset: The part of the address that falls within the cache line
 * 	cacheIndex:  The part of the address which does not fall within the cache line and
 * 				 does fall within the smallest page. Only these bits can be used for indexing
 * 	vTag:		 The part of the address that does not fall within the smallest page.
 *
 * 	For example:
 * 	Linesize:		64 Bytes
 * 	Smallest page:	4  KiB
 * 		[52:12]				[11:6]				[5:0]
 * 		 vTag		|      d$ max index	|     d$ offset
 * 		 vTag       |               page offset
 */
void DCachePreNov::splitAddress(MemAddr addr, MemAddr &cacheOffset, MemAddr &cacheIndex, MemAddr *vTag){
	//MLDTODO Merge with map function (selector)?
	//MLDTODO m_linesize must be 2^x
	//size_t indexBits = ilog2(this->m_lineSize);

	cacheOffset = addr & (this->m_lineSize - 1);
	cacheIndex = addr - cacheOffset;
	cacheIndex &= (1 << this->m_mmu->getDTlb().getMinOffsetWidth()) - 1;

	if(vTag != NULL){
		*vTag = addr - cacheOffset - cacheIndex;
	}
}

DCache::Line& DCachePreNov::fetchLine(MemAddr address){
	assert(m_assoc == 1); // Associativity must be 1

	size_t offsetBits, indexBits;
	splitAddress(address, offsetBits, indexBits, NULL);

	//MLDTODO-DOC Als fetchLine altijd hetzelfde setIndex geeft voor zowel pAddr als vAddr, is geen reverse lookup nodig!
	//MLDTODO Meerdere "selectors" beschikbaar. Omdat ik enkel de indexBits geef kunnen ze allen gebruikt worden, maar is dat nuttig?
    MemAddr vTag;
    size_t setindex;
    m_selector->Map(indexBits / this->m_lineSize, vTag, setindex);
    DebugMemWrite("Fetchline 0x%lX: vTag=0x%lX, index=%lu", address, vTag, setindex);

    return m_lines[setindex];
}

bool DCachePreNov::comparePTag(Line &line, MemAddr pTag){
	return line.tag == pTag;
}

bool DCachePreNov::freeLine(Line &line){
	// Invalid lines may not be touched or considered
	if (line.state != LINE_EMPTY && line.state != LINE_FULL)
	{
        DeadlockWrite("Cache-line %lu cannot be freed", getLineId(&line));
		return false;
	}

	// Reset the line
	COMMIT
	{
		line.processing = false;
		line.waiting    = INVALID_REG;
		std::fill(line.valid, line.valid + m_lineSize, false);
	}

    return true;
}

//Result DCache::FindLine(MemAddr address, ContextId contextId, Line* &line, bool check_only, bool ignore_tags)
//{
//	size_t offsetBits, indexBits;
//	splitAddress(address, offsetBits, indexBits, NULL);
//
//	//MLDTODO-DOC Als FindLine altijd hetzelfde setIndex geeft voor zowel pAddr als vAddr, is geen reverse lookup nodig!
//	//MLDTODO Meerdere "selectors" beschikbaar. Gebruik "DIRECT"
//	//MLDTODO Take contextId into account
//    MemAddr tag;
//    size_t setindex;
//    m_selector->Map(indexBits, tag, setindex);
//    DebugMemWrite("Findline 0x%X: tag=0x%X, index=%d", address, tag, setindex);
//
//    const size_t  set  = setindex * m_assoc;
//
//    // Find the line
//    Line* empty   = NULL;
//    Line* replace = NULL;
//    for (size_t i = 0; i < m_assoc; ++i)
//    {
//        line = &m_lines[set + i];
//
//        // Invalid lines may not be touched or considered
//        if (line->state == LINE_EMPTY)
//        {
//            // Empty, unused line, remember this one
//            empty = line;
//        }
//        else if (ignore_tags || (line->tag == tag && line->contextTag == contextId)) //MLDTODO Will do for now. Correct?
//        {
//            // The wanted line was in the cache
//            return SUCCESS;
//        }
//        else if (line->state == LINE_FULL && (replace == NULL || line->access < replace->access))
//        {
//            // The line is available to be replaced and has a lower LRU rating,
//            // remember it for replacing
//            replace = line;
//        }
//    }
//
//    // The line could not be found, allocate the empty line or replace an existing line
//    line = (empty != NULL) ? empty : replace;
//    if (line == NULL)
//    {
//        // No available line
//        if (!check_only)
//        {
//            DeadlockWrite("Unable to allocate a free cache-line in set %u", (unsigned)(set / m_assoc) );
//        }
//        return FAILED;
//    }
//
//    if (!check_only)
//    {
//        // Reset the line
//        COMMIT
//        {
//            line->processing = false;
//            line->tag        = tag;
//            line->contextTag = contextId;
//            line->waiting    = INVALID_REG;
//            std::fill(line->valid, line->valid + m_lineSize, false);
//        }
//    }
//
//    return DELAYED;
//}

Result DCachePreNov::Read2(ContextId contextId, MemAddr address, void* data, MemSize size, RegAddr* reg)
{//CASE L1/L2/L3/L4/L5/L6/L7/L8/P/F
	size_t offsetBits, indexBits, vTag;
	splitAddress(address, offsetBits, indexBits, &vTag);

	//std::cout << "OB:" << offsetBits << ", SZ:" << size << ", LS:" << m_lineSize << std::endl;
	if (offsetBits + size > m_lineSize)
    {
        throw exceptf<InvalidArgumentException>(*this, "Read (%#016llx, %zd): Address range crosses over cache line boundary",
                                                (unsigned long long)address, (size_t)size);
    }

#if MEMSIZE_MAX >= SIZE_MAX
    if (size > SIZE_MAX)
    {
        throw exceptf<InvalidArgumentException>(*this, "Read (%#016llx, %zd): Size argument too big",
                                                (unsigned long long)address, (size_t)size);
    }
#endif

    if (!p_service.Invoke())
    {
        DeadlockWrite("Unable to acquire port for D-Cache read access (%#016llx, %zd)",
                      (unsigned long long)address, (size_t)size);

        return FAILED;
    }

    if (!m_mmu->getDTlb().invoke()){
        DeadlockWrite("Unable to acquire port for D-TLB read access (%#016llx, %zd)",
                      (unsigned long long)address, (size_t)size);

        return FAILED;
    }

    Line& line = fetchLine(address);
    mmu::TLBResultMessage tlbData;
    Result tlbResult = m_mmu->getDTlb().lookup(contextId, address, false, tlbData);

    if(tlbResult == FAILED)
    {//Case F
    	DebugMemWrite("CASE F");
        DeadlockWrite("dTLB lookup for (%u, %#016llx) failed!", contextId, (unsigned long long)address);
    	return FAILED; //MLDTODO Temporary
    }
    else if(tlbResult == DELAYED)
    {// Case L5/L6/L7/L8
    	if(!freeLine(line))
    	{//Case L5/L6/L7
        	DebugMemWrite("Case L5, L6 or L7");
    		//MLDTODO statistics
        	return FAILED;
    	}
    	else
    	{//Case L8
    		DebugMemWrite("CASE L8");
    		COMMIT{
    			m_mmu->getDTlb().setDCacheReference(getLineId(&line));
    			line.tlbOffset = offsetBits + indexBits;
    		}
        	COMMIT {
        		//MLDTODO Statistics
        		if (line.state == LINE_EMPTY){
        			++m_numEmptyRMisses;
        		}else{
        			++m_numResolvedConflicts;
        		}

        		line.state = LINE_LOADING;
        		PushRegister(&line, reg);

        		++m_numDelayedReads;
        	}

        	return DELAYED;
    	}
    }
    else if(tlbResult == SUCCESS)
    {//Case L1/L2/L3/L4/P
    	if(!tlbData.read)
    	{//Case P
			throw exceptf<SecurityException>(*this, "Read (%#016llx, %zd): Attempting to read from non-readable memory",
											 (unsigned long long)address, (size_t)size);
    	}

        //Compare tags
        if(comparePTag(line, tlbData.pAddr))
        {//Case L1/L2

            // Update last line access
            COMMIT{ line.access = GetDRISC().GetCycleNo(); } //MLDTODO Call on L2?
        	// Check if the data that we want is valid in the line.
        	// This happens when the line is FULL, or LOADING and has been
        	// snooped to (written to from another core) in the mean time.
        	size_t i;
        	for (i = 0; i < size; ++i)
        	{
        		if (!line.valid[offsetBits + i])
        		{
        			break;
        		}
        	}

        	if (i == size)
        	{//Case L1
				DebugMemWrite("CASE L1");

				COMMIT
				{
					// Data is entirely in the cache, copy it
					memcpy(data, line.data + offsetBits, (size_t)size);
					++m_numRHits; //MLDTODO Statistics
				}
				return SUCCESS;
        	}
        	else
        	{// Case L2
           		DebugMemWrite("CASE L2");
            	COMMIT{
           			//MLDTODO statistics
            		++m_numLoadingRMisses;
            		++m_numDelayedReads;

            		PushRegister(&line, reg);
            	}

            	return DELAYED;
        	}
        }
        else
        {//Case L3/L4
        	if(freeLine(line))
        	{//Case L4
        		DebugMemWrite("CASE L4");

        		Request request;
        		request.write     = false;
        		request.address   = tlbData.pAddr - offsetBits; //MLDTODO pAddr

        		if (!m_outgoing.Push(std::move(request)))
        		{
        			++m_numStallingRMisses;//MLDTODO Statistics
        			DeadlockWrite("Unable to push request to outgoing buffer");
        			return FAILED;
        		}
        		COMMIT{
        			line.tag = tlbData.pAddr - offsetBits;
        		}
				COMMIT {
					if (line.state == LINE_EMPTY){
						++m_numEmptyRMisses;//MLDTODO Statistics
					}else{
						++m_numResolvedConflicts;//MLDTODO Statistics
					}

					line.state = LINE_LOADING;
					PushRegister(&line, reg);

					++m_numDelayedReads; //MLDTODO Statistics
				}
				return DELAYED;
        	}
        	else
        	{//Case L3
            	DebugMemWrite("CASE L3");

            	++m_numHardConflicts; //MLDTODO Statistics
                //DeadlockWrite() is done in freeLine
                return FAILED;
        	}
        }
    }
    UNREACHABLE
}

ExtendedResult DCachePreNov::Write2(ContextId contextId, MemAddr address, void* data, MemSize size, LFID fid, TID tid)
{
	size_t offsetBits, indexBits, vTag;
	splitAddress(address, offsetBits, indexBits, &vTag);

    assert(fid != INVALID_LFID);
    assert(tid != INVALID_TID);

    if (offsetBits + size > m_lineSize)
    {
        throw exceptf<InvalidArgumentException>(*this, "Write (%#016llx, %zd): Address range crosses over cache line boundary",
                                                (unsigned long long)address, (size_t)size);
    }

#if MEMSIZE_MAX >= SIZE_MAX
    if (size > SIZE_MAX)
    {
        throw exceptf<InvalidArgumentException>(*this, "Write (%#016llx, %zd): Size argument too big",
                                                (unsigned long long)address, (size_t)size);
    }
#endif

    if (!p_service.Invoke())
    {
        DeadlockWrite("Unable to acquire port for D-Cache write access (%#016llx, %zd)",
                      (unsigned long long)address, (size_t)size);
        return ExtendedResult::FAILED;
    }

    if (!m_mmu->getDTlb().invoke()){
        DeadlockWrite("Unable to acquire port for D-TLB read access (%#016llx, %zd)",
                      (unsigned long long)address, (size_t)size);

        return ExtendedResult::FAILED;
    }

    Line& line = fetchLine(address);
    mmu::TLBResultMessage tlbData;
    Result tlbResult = m_mmu->getDTlb().lookup(contextId, address, false, tlbData);

    if(tlbResult == DELAYED){
    	//MLDTODO Statistics
    	m_mmu->getDTlb().setDCacheReference(INVALID_CID);
    	return ExtendedResult::DELAYED;
    }

    if (comparePTag(line, address))
    {
        assert(line.state != LINE_EMPTY);

		if (!tlbData.write)
		{
			throw exceptf<SecurityException>(*this, "Write (%#016llx, %zd): Attempting to write to non-writable memory",
											 (unsigned long long)address, (size_t)size);
		}

        if (line.state == LINE_LOADING || line.state == LINE_INVALID)
        {
        	//MLDNOTE STORE, MISS, WAITING, *
        	//MLDTODO Handle TLB request result
            // We cannot write into a loading line or we might violate the
            // sequential semantics of a single thread because pending reads
            // might get the later write's data.
            // We cannot ignore the line either because new reads should get the new
            // data and not the old.
            // We cannot invalidate the line so new reads will generate a new
            // request, because read completion goes on address, and then we would have
            // multiple lines of the same address.
            //

            // So for now, just stall the write
            ++m_numLoadingWMisses;
            DeadlockWrite("Unable to write into loading cache line");
            return ExtendedResult::FAILED;
        }
        else
        {
            // Update the line
            assert(line.state == LINE_FULL);
            COMMIT{
            	//MLDNOTE STORE, HIT
                std::copy((char*)data, (char*)data + size, line.data + offsetBits);
                std::fill(line.valid + offsetBits, line.valid + offsetBits + size, true);

                // Statistics
                ++m_numWHits;
            }
        }
    }
    else
    {
        COMMIT{ ++m_numPassThroughWMisses; }
    }

    //MLDNOTE STORE, MISS, AVAIL
    //MLDTODO Handle TLB request result
    Request request;
    request.write     = true;
    request.address   = tlbData.pAddr - offsetBits;
    request.wid       = tid;//MLDNOTE wid=write id, continuation word opgeslagen (wachten tot alle writes gecommit, optioneel)
    //MLDNOTE Bij store+TLB Miss --> Threads hebben ook een linkedlist. Threads suspenden. On request completion: Reschedule threads.
    //MLDNOTE Beginnen met een stall.
    //MLDTODO-DOC Hoort ook weer in scriptie.

    COMMIT{
    std::copy((char*)data, ((char*)data)+size, request.data.data+offsetBits);
    std::fill(request.data.mask, request.data.mask+offsetBits, false);
    std::fill(request.data.mask+offsetBits, request.data.mask+offsetBits+size, true);
    std::fill(request.data.mask+offsetBits+size, request.data.mask+m_lineSize, false);
    }

    if (!m_outgoing.Push(std::move(request)))
    {
        ++m_numStallingWMisses;
        DeadlockWrite("Unable to push request to outgoing buffer");
        return ExtendedResult::FAILED;
    }

    COMMIT{ ++m_numWAccesses; }

    return ExtendedResult::DELAYED;
}

Result DCachePreNov::DoLookupResponses(){
	assert(!m_lookup_responses.Empty());

    if (!p_service.Invoke())
    {
        DeadlockWrite("Unable to acquire port for D-Cache access in lookup completion");
        return FAILED;
    }

    auto& response = m_lookup_responses.Front();

    if(response.cid == INVALID_CID)
    {//Case (L5)/S5/S7/S8
    	UNREACHABLE //MLDTODO Handle
    }

    //Valid CID, this must be the callback for cases L6 or L8!
    assert(response.present); //MLDTODO Handle

    //We need to restart the memory req.
    Line& line = m_lines[response.cid];
    assert(line.state == LINE_LOADING);

    DebugMemWrite("Processing lookup completion for Line ID %u", response.cid);

    mmu::TLBResultMessage tlbData = m_mmu->getDTlb().getLine(response.tlbLineRef);

	if(!tlbData.read)
	{//Case P
		throw exceptf<SecurityException>(*this, "Read (%#016llx): Attempting to read from non-readable physical address",
										 (unsigned long long)tlbData.pAddr);
	}

    //// DO REQUEST

    size_t pOffset = (size_t)(tlbData.pAddr % m_lineSize);

    DebugTLBWrite("TLBLookupComplete: address: 0x%lX", tlbData.pAddr);
    Request request;
    request.write     = false;
    request.address   = tlbData.pAddr - pOffset; //MLDTODO pAddr

	if (!m_outgoing.Push(std::move(request)))
	{
		++m_numStallingRMisses; //MLDTODO Fix statistics
		DeadlockWrite("Unable to push request to outgoing buffer");
		return FAILED;
	}
	COMMIT {
		line.tag = tlbData.pAddr - pOffset;

		if (line.state == LINE_EMPTY){
			++m_numEmptyRMisses; //MLDTODO Fix statistics
		}else{
			++m_numResolvedConflicts; //MLDTODO Fix statistics
		}

		line.state = LINE_LOADING; //MLDTODO Fix statistics
		//PushRegister(line, reg); //MLDTODO Fix register linked list

		++m_numDelayedReads; //MLDTODO Fix statistics
	}

    ////END DO REQUEST

    m_lookup_responses.Pop();

    return SUCCESS;
}

Result DCachePreNov::DoReadResponses()
{
    assert(!m_read_responses.Empty());

    if (!p_service.Invoke())
    {
        DeadlockWrite("Unable to acquire port for D-Cache access in read completion");
        return FAILED;
    }

    // Process a waiting register
    auto& response = m_read_responses.Front();
    Line& line = m_lines[response.cid];

    assert(line.state == LINE_LOADING || line.state == LINE_INVALID);

    DebugMemWrite("Processing read completion for CID %u", (unsigned)response.cid);

    // If bundle creation is waiting for the line data, deliver it
    if (line.create)
    {
        DebugMemWrite("Signalling read completion to creation process");
        auto& alloc = GetDRISC().GetAllocator();
        alloc.OnDCachelineLoaded(line.data);
        COMMIT { line.create = false; }
    }

    if (line.waiting.valid())
    {
        // Push the cache-line to the back of the queue
        WritebackRequest req;
        COMMIT{std::copy(line.data, line.data + m_lineSize, req.data);}
        req.waiting = line.waiting;

        DebugMemWrite("Queuing writeback request for CID %u starting at %s", (unsigned)response.cid, req.waiting.str().c_str());

        if (!m_writebacks.Push(std::move(req)))
        {
            DeadlockWrite("Unable to push writeback request to buffer");
            return FAILED;
        }
    }

    COMMIT {
        line.waiting = INVALID_REG;
        line.state = (line.state == LINE_INVALID) ? LINE_EMPTY : LINE_FULL;
    }
    m_read_responses.Pop();
    return SUCCESS;
}

Result DCachePreNov::DoReadWritebacks()
{
    assert(!m_writebacks.Empty());

    // Process a waiting register
    auto& req = m_writebacks.Front();

    WritebackState state = m_wbstate;
    if (!state.next.valid() && state.offset == state.size)
    {
        // New request
        assert(req.waiting.valid());
        state.next = req.waiting;
    }

    auto& regFile = GetDRISC().GetRegisterFile();

    if (state.offset == state.size)
    {
        // Starting a new multi-register write

        // Write to register
        if (!regFile.p_asyncW.Write(state.next))
        {
            DeadlockWrite("Unable to acquire port to write back %s", state.next.str().c_str());
            return FAILED;
        }

        // Read request information
        RegValue value;
        if (!regFile.ReadRegister(state.next, value))
        {
            DeadlockWrite("Unable to read register %s", state.next.str().c_str());
            return FAILED;
        }

        if (value.m_state == RST_FULL || value.m_memory.size == 0)
        {
            // Rare case: the request info is still in the pipeline, stall!
            DeadlockWrite("Register %s is not yet written for read completion", state.next.str().c_str());
            return FAILED;
        }

        if (value.m_state != RST_PENDING && value.m_state != RST_WAITING)
        {
            // We're too fast, wait!
            DeadlockWrite("Memory read completed before register %s was cleared", state.next.str().c_str());
            return FAILED;
        }

        // Ignore the request if the family has been killed
        state.value = UnserializeRegister(state.next.type, &req.data[value.m_memory.offset], value.m_memory.size);

        if (value.m_memory.sign_extend)
        {
            // Sign-extend the value
            assert(value.m_memory.size < sizeof(Integer));
            int shift = (sizeof(state.value) - value.m_memory.size) * 8;
            state.value = (int64_t)(state.value << shift) >> shift;
        }

        state.fid    = value.m_memory.fid;
        state.addr   = state.next;
        state.next   = value.m_memory.next;
        state.offset = 0;

        // Number of registers that we're writing (must be a power of two)
        state.size = (value.m_memory.size + sizeof(Integer) - 1) / sizeof(Integer);
        assert((state.size & (state.size - 1)) == 0);
    }
    else
    {
        // Write to register
        if (!regFile.p_asyncW.Write(state.addr))
        {
            DeadlockWrite("Unable to acquire port to write back %s", state.addr.str().c_str());
            return FAILED;
        }
    }

    assert(state.offset < state.size);

    // Write to register file
    RegValue reg;
    reg.m_state = RST_FULL;

#if ARCH_ENDIANNESS == ARCH_BIG_ENDIAN
    // LSB goes in last register
    const Integer data = state.value >> ((state.size - 1 - state.offset) * sizeof(Integer) * 8);
#else
    // LSB goes in first register
    const Integer data = state.value >> (state.offset * sizeof(Integer) * 8);
#endif

    switch (state.addr.type) {
    case RT_INTEGER: reg.m_integer       = data; break;
    case RT_FLOAT:   reg.m_float.integer = data; break;
    default: UNREACHABLE;
    }

    DebugMemWrite("Completed load: %#016llx -> %s %s",
                  (unsigned long long)data, state.addr.str().c_str(), reg.str(state.addr.type).c_str());

    if (!regFile.WriteRegister(state.addr, reg, true))
    {
        DeadlockWrite("Unable to write register %s", state.addr.str().c_str());
        return FAILED;
    }

    // Update writeback state
    state.offset++;
    state.addr.index++;

    if (state.offset == state.size)
    {
        // This operand is now fully written
        auto& alloc = GetDRISC().GetAllocator();
        if (!alloc.DecreaseFamilyDependency(state.fid, FAMDEP_OUTSTANDING_READS))
        {
            DeadlockWrite("Unable to decrement outstanding reads on F%u", (unsigned)state.fid);
            return FAILED;
        }

        if (!state.next.valid())
        {
            COMMIT {
                state.value = 0;
                state.addr = INVALID_REG;
                state.size = 0;
                state.offset = 0;
                state.fid = 0;
            }
            m_writebacks.Pop();
        }
    }
    COMMIT{ m_wbstate = state; }

    return SUCCESS;
}

Result DCachePreNov::DoWriteResponses()
{
    assert(!m_write_responses.Empty());
    auto& response = m_write_responses.Front();

    auto& alloc = GetDRISC().GetAllocator();
    if (!alloc.DecreaseThreadDependency((TID)response.wid, THREADDEP_OUTSTANDING_WRITES))
    {
        DeadlockWrite("Unable to decrease outstanding writes on T%u", (unsigned)response.wid);
        return FAILED;
    }

    DebugMemWrite("T%u completed store", (unsigned)response.wid);

    m_write_responses.Pop();
    return SUCCESS;
}

Result DCachePreNov::DoOutgoingRequests()
{
    assert(m_memory != NULL);
    assert(!m_outgoing.Empty());
    const Request& request = m_outgoing.Front();

    if (request.write)
    {
        if (!m_memory->Write(m_mcid, request.address, request.data, request.wid))
        {
            DeadlockWrite("Unable to send write to 0x%016llx to memory", (unsigned long long)request.address);
            return FAILED;
        }
    }
    else
    {
        if (!m_memory->Read(m_mcid, request.address))
        {
            DeadlockWrite("Unable to send read to 0x%016llx to memory", (unsigned long long)request.address);
            return FAILED;
        }
    }

    DebugMemWrite("F%d queued outgoing %s request for %.*llx",
                  (request.write ? (int)request.wid : -1), (request.write ? "store" : "load"),
                  (int)(sizeof(MemAddr)*2), (unsigned long long)request.address);

    m_outgoing.Pop();
    return SUCCESS;

}

}
}
