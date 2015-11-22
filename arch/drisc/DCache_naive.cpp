#include "DCache_naive.h"

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
 * 		  - Only works for DIRECT bank selection.
 * 		  Check for limitations
 * 		  MLDTODO-DOC
*/

//MLDTODO-DOC: ContextId is at least 16 bits. But the number of threads running within a single TLB's domain is not going to fill that. A simple lookup table could save an enourmous amount of storage space!

DCacheNaive::DCacheNaive(const std::string& name, DRISC& parent, Clock& clock)
:DCache(name, parent, clock)
{}

DCacheNaive::~DCacheNaive()
{}

/*
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
/*
 *
 */

bool DCacheNaive::freeLine(Line &line){
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

bool DCacheNaive::getEmptyLine(size_t /*address*/, Line* &/*line*/){
	throw "myFunction is not implemented yet.";
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
//MLDTODO-DOC Instead of [V|P]IPT, UIPT (universal)


void DCacheNaive::resetLine(Line* line){
	COMMIT
	{	//MLDTODO Does not reset line->state due to statistics...
		line->processing = false;
		line->tag       = 0;
		line->waiting    = INVALID_REG;
		std::fill(line->valid, line->valid + m_lineSize, false);
	}
}

DCache::Line* DCacheNaive::getEmptyLine(size_t setIndex){
	Line* replace = NULL;
	for (size_t i = 0; i < m_assoc; ++i)
	{
		Line* line = &m_lines[(setIndex * m_assoc) + i];

		// Invalid and loading lines may not be touched or considered
		if (line->state == LINE_EMPTY)
		{
			resetLine(line);
			return line;
		}
		else if (line->state == LINE_FULL && (replace == NULL || line->access < replace->access))
		{
			// The line is available to be replaced and has a lower LRU rating,
			// remember it for replacing
			replace = line;
		}
	}

	if(replace == NULL){
		DeadlockWrite("Unable to allocate a free cache-line in set %u", (unsigned)(setIndex) );
	}else{
		resetLine(replace);
	}

	return replace;
}

/*
 * Cases:
 * 	Case 1: TLB HIT, PERM OK, D$ HIT, LINE FULL
 * 	Case 2: TLB HIT, PERM OK, D$ HIT, LINE !FULL
 * 	CASE 3: TLB HIT, PERM OK, D$ MISS, LINE AVAILABLE
 * 	CASE 4: TLB HIT, PERM OK, D$ MISS, SET LINES OCCUPIED
 * 	CASE 5: TLB HIT, PERM !OK
 * 	CASE 6: TLB PENDING, LINE AVAILABLE
 * 	CASE 7: TLB PENDING, SET LINES OCCUPIED
 * 	CASE 8: TLB FAILED, LINE AVAILABLE
 * 	CASE 9: TLB FAILED, SET LINES OCCUPIED
 */
Result DCacheNaive::Read2(ContextId contextId, MemAddr address, void* data, MemSize size, RegAddr* reg)
{ //MLDTODO It seems that the original D$ does not store size information when delaying a request.
	MemAddr cacheOffset;
	size_t setIndex;
	splitAddress(address, cacheOffset, setIndex, NULL);

	if (cacheOffset + size > m_lineSize)
    {
        throw exceptf<InvalidArgumentException>(*this, "Read (%#016llx, %zd): Address range crosses over cache line boundary", (unsigned long long)address, (size_t)size);
    }

#if MEMSIZE_MAX >= SIZE_MAX
    if (size > SIZE_MAX)
    {
        throw exceptf<InvalidArgumentException>(*this, "Read (%#016llx, %zd): Size argument too big", (unsigned long long)address, (size_t)size);
    }
#endif

    if (!p_service.Invoke())
    {
        DeadlockWrite("Unable to acquire port for D-Cache read access (%#016llx, %zd)", (unsigned long long)address, (size_t)size);
        return FAILED;
    }

    if (!m_mmu->getDTlb().invoke()){
        DeadlockWrite("Unable to acquire port for D-TLB read access (%#016llx, %zd)", (unsigned long long)address, (size_t)size);
        return FAILED;
    }

    mmu::TLBResultMessage tlbData;
    Result lookupResult = m_mmu->getDTlb().lookup(contextId, address, false, tlbData);

    if(lookupResult == SUCCESS){
    	Line* line;
    	MemAddr pTag;
    	MemAddr pAddr = tlbData.pAddr << tlbData.tlbOffsetWidth;
    	pAddr |= ((1 << tlbData.tlbOffsetWidth) - 1) & address;

    	splitAddress(pAddr, cacheOffset, setIndex, &pTag);

    	if(!tlbData.read)
    	{ //Case 5
    		DebugMemWrite("D-Cache state 5");
			throw exceptf<SecurityException>(*this, "Read (%#016llx, %zd): Attempting to read from non-readable memory", (unsigned long long)address, (size_t)size);
    	}

    	line = findLine(setIndex, pTag);
    	if(line && line->state != LINE_INVALID){
            COMMIT{ line->access = GetDRISC().GetCycleNo(); } //MLDTODO Call on L2?


    		if(hasData(line, cacheOffset, size))
    		{ //Case 1
    			DebugMemWrite("D-Cache state 1");
				COMMIT
				{
					memcpy(data, line->data + cacheOffset, (size_t)size);

					//MLDTODO Statistics
//					++m_numRHits;
				}
				return SUCCESS;

    		}else
    		{ //Case 2
    			DebugMemWrite("D-Cache state 2");
            	COMMIT{
            		PushRegister(line, reg);

           			//MLDTODO statistics
//            		++m_numLoadingRMisses;
//            		++m_numDelayedReads;
            	}
            	return DELAYED;
    		}
    	}else{
    		line = getEmptyLine(setIndex);
    		if(line)
    		{ //Case 3 MLDTODO
    			DebugMemWrite("D-Cache state 3");
    			if(!initiateMemoryRequest(cacheOffset, setIndex, pTag)){ //MLDTODO Will only work for smallest page size
    				return FAILED;
    			}
        		COMMIT{
        			line->tag = pTag;
//					if (line.state == LINE_EMPTY){
//						++m_numEmptyRMisses;//MLDTODO Statistics
//					}else{
//						++m_numResolvedConflicts;//MLDTODO Statistics
//					}

					line->state = LINE_LOADING;
					PushRegister(line, reg);

//					++m_numDelayedReads; //MLDTODO Statistics
				}
				return DELAYED;

    		}else
    		{ //Case 4 MLDTODO
    			DebugMemWrite("D-Cache state 4");
//            	COMMIT {++m_numHardConflicts;} //MLDTODO Statistics
    	        DeadlockWrite("DCache write for (%u, %#016llx) failed! [CASE 4]", contextId, (unsigned long long)address);
    	        return FAILED;
    		}
    	}
    }else if(lookupResult == DELAYED){
		Line* line = getEmptyLine(setIndex);
		if(line)
		{ //Case 6
			DebugMemWrite("D-Cache state 6");
			COMMIT{
				m_mmu->getDTlb().setDCacheReference(getLineId(line));
				line->tlbOffset = address && ~(UINT64_MAX << m_mmu->getDTlb().getMaxOffsetWidth());
				line->state = LINE_LOADING;
				PushRegister(line, reg);
//        		//MLDTODO Statistics
//     			++m_numEmptyRMisses;
//        		++m_numDelayedReads;
			}
			return DELAYED;
		}else
		{ //Case 7
			DebugMemWrite("D-Cache state 7");
	        DeadlockWrite("DCache write for (%u, %#016llx) failed! [CASE 7]", contextId, (unsigned long long)address);
	        return FAILED;
		}
    }else
	{ //Case 8 | 9
    	DebugMemWrite("D-Cache state 8|9");
		DeadlockWrite("dTLB lookup for (%u, %#016llx) failed! [CASE 8|9]", contextId, (unsigned long long)address);
		return FAILED;
    }
}

ExtendedResult DCacheNaive::Write2(ContextId contextId, MemAddr address, void* data, MemSize size, LFID fid, TID tid)
{
    assert(fid != INVALID_LFID);
    assert(tid != INVALID_TID);

	MemAddr cacheOffset;
	size_t setIndex;
	splitAddress(address, cacheOffset, setIndex, NULL);

    if (cacheOffset + size > m_lineSize)
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

    mmu::TLBResultMessage tlbData;
    Result tlbResult = m_mmu->getDTlb().lookup(contextId, address, false, tlbData);

    if(tlbResult == DELAYED)
    { //Case 8, 9
    	COMMIT{ m_mmu->getDTlb().setDCacheReference(INVALID_CID); }
    	DeadlockWrite("TLB delayed on write");
    	return ExtendedResult::ACTIVE;
    }else if(tlbResult != SUCCESS)
    { //Case 6, 7
    	DeadlockWrite("TLB miss on write");
    	return ExtendedResult::FAILED;
    }

	if (!tlbData.write)
	{
		throw exceptf<SecurityException>(*this, "Write (%#016llx, %zd): Attempting to write to non-writable memory", (unsigned long long)address, (size_t)size);
	}
	MemAddr pAddr = tlbData.pAddr << tlbData.tlbOffsetWidth;
	pAddr |= ((1 << tlbData.tlbOffsetWidth) - 1) & address;

	//std::cout << "offs:" << tlbData.tlbOffsetWidth << ", " << std::hex << pAddr << std::dec << std::endl;

    MemAddr pTag;
	splitAddress(pAddr, cacheOffset, setIndex, &pTag);

	Line* line = findLine(setIndex, pTag);
	if(line)
	{ //Case 1, 2
		if (line->state == LINE_LOADING || line->state == LINE_INVALID)
		{
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
			//++m_numLoadingWMisses; MLDTODO Statistics
			DeadlockWrite("Unable to write into loading cache line");
			return ExtendedResult::FAILED;
		}
		else
		{
			// Update the line
			assert(line->state == LINE_FULL);
			COMMIT{
				std::copy((char*)data, (char*)data + size, line->data + cacheOffset);
				std::fill(line->valid + cacheOffset, line->valid + cacheOffset + size, true);
				//++m_numWHits;	//MLDTODO Statistics
			}
		}
	}else
	{ //Case 3, 4
//		COMMIT{ ++m_numPassThroughWMisses; } MLDTODO Statistics
	}

	if(!initiateMemoryRequest(true, cacheOffset, setIndex, pTag, data, size, tid)){
		return ExtendedResult::FAILED;
	}

    return ExtendedResult::DELAYED;
}

Result DCacheNaive::DoLookupResponses(){
	assert(!m_lookup_responses.Empty());

    if (!p_service.Invoke())
    {
        DeadlockWrite("Unable to acquire port for D-Cache access in lookup completion");
        return FAILED;
    }

    auto& response = m_lookup_responses.Front();

	if(!response.present)
	{ //Case 12
		throw exceptf<SecurityException>(*this, "Page fault %p, %lu", (void *)response.tlbLineRef.m_line, (unsigned long)response.tlbLineRef.m_table); //MLDTODO Add information
	}

    if(response.cid == INVALID_CID)
    { //Case 10
    	//Do nothing...
		DebugMemWrite("'Handling' lookup completion for 'magic' CID");
        m_lookup_responses.Pop();
        return SUCCESS;
    }

    mmu::TLBResultMessage tlbData = m_mmu->getDTlb().getLine(response.tlbLineRef);
	MemAddr pAddr = tlbData.pAddr << tlbData.tlbOffsetWidth;
	//std::cout << "offs:" << tlbData.tlbOffsetWidth << std::endl;
	assert(!tlbData.pending);

    //Unreachable by writes, so this must be case 1, 2, 3 or 5 (read)
    Line* line = &m_lines[response.cid];
    assert(line->state == LINE_LOADING);

	pAddr |= ((1 << tlbData.tlbOffsetWidth) - 1) & line->tlbOffset;
	//MLDTODO Add offset bits
	DebugMemWrite("Handling lookup completion for CID %u, (%#016llx)", (unsigned)response.cid, (unsigned long long)pAddr);

	if(!tlbData.read)
	{ //Case 5
		throw exceptf<SecurityException>(*this, "Read (%#016llx): Attempting to read from physical address through non-readable virtual address", (unsigned long long)pAddr);
	}

	MemAddr cacheOffset, pTag;
	size_t setIndex;
	splitAddress(pAddr, cacheOffset, setIndex, &pTag);

	Line* otherLine = findLine(setIndex, pTag, line);
	if(otherLine)
	{ //Case 1 or 2
		COMMIT{
			PushRegister(otherLine, &line->waiting);
			line->waiting = INVALID_REG;
			line->state = LINE_EMPTY;
		}

		if(hasData(otherLine, 0, m_lineSize))
		{ //Case 1
		    if(!initiateWriteback(otherLine)){
		    	return FAILED; //Deadlockwrite done in initiateWriteback
		    }
		}
	}else
	{ //Case 3
		if(!initiateMemoryRequest(cacheOffset, setIndex, pTag)){ //MLDTODO In case of write?
			return FAILED;
		}

		COMMIT {
			line->tag = pTag;

			if (line->state == LINE_EMPTY){
	//			++m_numEmptyRMisses; //MLDTODO Fix statistics
			}else{
	//			++m_numResolvedConflicts; //MLDTODO Fix statistics
			}

			line->state = LINE_LOADING;
			//PushRegister(line, reg); //MLDTODO Fix register linked list

	//		++m_numDelayedReads; //MLDTODO Fix statistics
		}
	}

    m_lookup_responses.Pop();
    return SUCCESS;
}

Result DCacheNaive::DoReadResponses()
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

    if(!initiateWriteback(&line)){
    	return FAILED; //Deadlockwrite done in initiateWriteback
    }

    COMMIT {
        line.waiting = INVALID_REG;
        line.state = (line.state == LINE_INVALID) ? LINE_EMPTY : LINE_FULL;
    }
    m_read_responses.Pop();
    return SUCCESS;
}

bool DCacheNaive::initiateWriteback(Line* line){
	// If bundle creation is waiting for the line data, deliver it
	if (line->create)
	{
		DebugMemWrite("Signalling read completion to creation process");
		auto& alloc = GetDRISC().GetAllocator();
		alloc.OnDCachelineLoaded(line->data);
		COMMIT { line->create = false; }
	}

	if (line->waiting.valid())
	{
		// Push the cache-line to the back of the queue
		WritebackRequest req;
		std::copy(line->data, line->data + m_lineSize, req.data);
		req.waiting = line->waiting;

		DebugMemWrite("Queuing writeback request for CID %u starting at %s", (unsigned)getLineId(line), req.waiting.str().c_str());

		if (!m_writebacks.Push(req))
		{
			DeadlockWrite("Unable to push writeback request to buffer");
			return false;
		}
	}
	return true;
}

Result DCacheNaive::DoReadWritebacks()
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

Result DCacheNaive::DoWriteResponses()
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

Result DCacheNaive::DoOutgoingRequests()
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
