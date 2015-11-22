#include "DCache.h"

#include <arch/drisc/DRISC.h>
#include <sim/log2.h>
#include <sim/config.h>
#include <sim/sampling.h>

#include <cassert>
#include <cstring>
#include <iomanip>
#include <cstdio>

#include "DCache_pre_nov.h"
#include "DCache_naive.h"
using namespace std;


//MLDTODO-DOC MMIO
//MLDTODO Linker plaatst secties van boot.bin op onhandige locaties. Op te lossen door Virt.mem voor boot.bin?



namespace Simulator
{

namespace drisc
{

std::unique_ptr<DCache> DCache::cacheFactory(const std::string cacheType, const std::string& componentName, DRISC& parent, Clock& clock) {
	if(cacheType == "NAIVE"){
		return std::unique_ptr<DCache>(new DCacheNaive(componentName, parent, clock));
	}else if(cacheType == "PRENOV"){
		return std::unique_ptr<DCache>(new DCachePreNov(componentName, parent, clock));
	}else{
        throw exceptf<InvalidArgumentException>(parent, "Unknown cache type: %s", cacheType.c_str());
	}
}

/*MLDTODO Limitations: (Some of which need to be removed!)
 * 		  - Only works for a direct mapped cache. (1-way set associative)
 * 		  - Only works for DIRECT bank selection.
 * 		  - Only works for memory systems that respond to the requesting $ only.
 * 		  Check for limitations
 * 		  MLDTODO-DOC
*/

//MLDTODO-DOC: ContextId is at least 16 bits. But the number of threads running within a single TLB's domain is not going to fill that. A simple lookup table could save an enourmous amount of storage space!

DCache::DCache(const std::string& name, DRISC& parent, Clock& clock)
:   Object(name, parent),
    m_memory(NULL),
    m_mcid(0),
    m_lines(),
    m_data(),
    m_valid(0),

    m_assoc          (GetConf("Associativity", size_t)),
    m_sets           (GetConf("NumSets", size_t)),
    m_lineSize       (GetTopConf("CacheLineSize", size_t)),
    m_selector       (IBankSelector::makeSelector(*this, GetConf("BankSelector", string), m_sets)),
    InitBuffer(m_lookup_responses, clock, "LookupResponsesBufferSize"),
    InitBuffer(m_read_responses, clock, "ReadResponsesBufferSize"),
    InitBuffer(m_write_responses, clock, "WriteResponsesBufferSize"),
    InitBuffer(m_writebacks, clock, "ReadWritebacksBufferSize"),
    InitBuffer(m_outgoing, clock, "OutgoingBufferSize"),
    m_wbstate(),
	m_mmu(&parent.getMMU()),
    InitSampleVariable(numRHits, SVC_CUMULATIVE),//MLDTODO Registreren variabelen voorbeeld
    InitSampleVariable(numDelayedReads, SVC_CUMULATIVE),
    InitSampleVariable(numEmptyRMisses, SVC_CUMULATIVE),
    InitSampleVariable(numInvalidRMisses, SVC_CUMULATIVE),
    InitSampleVariable(numLoadingRMisses, SVC_CUMULATIVE),
    InitSampleVariable(numHardConflicts, SVC_CUMULATIVE),
    InitSampleVariable(numResolvedConflicts, SVC_CUMULATIVE),
    InitSampleVariable(numWAccesses, SVC_CUMULATIVE),
    InitSampleVariable(numWHits, SVC_CUMULATIVE),
    InitSampleVariable(numPassThroughWMisses, SVC_CUMULATIVE),
    InitSampleVariable(numLoadingWMisses, SVC_CUMULATIVE),
    InitSampleVariable(numStallingRMisses, SVC_CUMULATIVE),
    InitSampleVariable(numStallingWMisses, SVC_CUMULATIVE),
    InitSampleVariable(numSnoops, SVC_CUMULATIVE),

    InitProcess(p_LookupResponses, DoLookupResponses),
    InitProcess(p_ReadWritebacks, DoReadWritebacks),
    InitProcess(p_ReadResponses, DoReadResponses),
    InitProcess(p_WriteResponses, DoWriteResponses),
    InitProcess(p_Outgoing, DoOutgoingRequests),

    p_service       (clock, GetName() + ".p_service")
{

	m_lookup_responses.Sensitive(p_LookupResponses);
    m_writebacks.Sensitive(p_ReadWritebacks);
    m_read_responses.Sensitive(p_ReadResponses);
    m_write_responses.Sensitive(p_WriteResponses);
    m_outgoing.Sensitive(p_Outgoing);

    // These things must be powers of two
    if (m_assoc == 0 || !IsPowerOfTwo(m_assoc))
    {
        throw exceptf<InvalidArgumentException>(*this, "DCacheAssociativity = %zd is not a power of two", (size_t)m_assoc);
    }

    if (m_sets == 0 || !IsPowerOfTwo(m_sets))
    {
        throw exceptf<InvalidArgumentException>(*this, "DCacheNumSets = %zd is not a power of two", (size_t)m_sets);
    }

    if (m_lineSize == 0 || !IsPowerOfTwo(m_lineSize))
    {
        throw exceptf<InvalidArgumentException>(*this, "CacheLineSize = %zd is not a power of two", (size_t)m_lineSize);
    }

    // At least a complete register value has to fit in a line
    if (m_lineSize < 8)
    {
        throw exceptf<InvalidArgumentException>(*this, "CacheLineSize = %zd is less than 8.", (size_t)m_lineSize);
    }

    m_lines.resize(m_sets * m_assoc);
    m_data.resize(m_lines.size() * m_lineSize);
    m_valid = new bool[m_lines.size() * m_lineSize];

    RegisterStateArray(m_valid, m_lines.size() * m_lineSize, "valid"); //MLDTODO Registreren variabelen voorbeeld
    RegisterStateVariable(m_data, "data");//MLDTODO Registreren variabelen voorbeeld

    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        auto &line = m_lines[i];
        line.state  = LINE_EMPTY;
        line.data   = &m_data[i * m_lineSize];
        line.valid  = &m_valid[i * m_lineSize];
        line.create = false;
        RegisterStateObject(line, "line" + to_string(i));
    }

    m_wbstate.size   = 0;
    m_wbstate.offset = 0;
    RegisterStateObject(m_wbstate, "wbstate");//MLDTODO Registreren variabelen voorbeeld
}

bool DCache::hasData(Line* line, size_t offset, size_t size){
	// Check if the data that we want is valid in the line.
	// This happens when the line is FULL, or LOADING and has been
	// snooped to (written to from another core) in the mean time.
	if(line->state == LINE_FULL){ return true; }

	size_t i;
	for (i = 0; i < size; ++i)
	{
		if (!line->valid[offset + i]){ break; }
	}

	return (i == size);
}

void DCache::splitAddress(MemAddr addr, MemAddr &cacheOffset, size_t &setIndex, MemAddr *pTag){
	MemAddr tagBuffer;
	cacheOffset = addr & (m_lineSize - 1); //m_lineSize is guaranteed to be a power of 2
	m_selector->Map(addr / m_lineSize, tagBuffer, setIndex);

	if(pTag != NULL){
		*pTag = tagBuffer;
	}
}

MemAddr DCache::unsplitAddress(MemAddr cacheOffset, size_t setIndex, MemAddr pTag){
	MemAddr addrBuffer = m_selector->Unmap(pTag, setIndex);
	addrBuffer *= m_lineSize;
	addrBuffer += cacheOffset;
	return addrBuffer;
}

DCache::Line* DCache::findLine(size_t setIndex, size_t pTag, Line* ignore){
	Line* line = NULL;

	for (size_t i = 0; i < m_assoc; ++i)
	{
		line = &m_lines[(setIndex * m_assoc) + i];
		if(line == ignore){
			continue;
		}

		// Consider only FULL, LOADING and INVALID lines
		if (line->state == LINE_FULL || line->state == LINE_LOADING || line->state == LINE_INVALID)
		{
			if(line->tag == pTag){
				return line;
			}
		}
	}

	return NULL;
}

Result DCache::Read(MemAddr address, void* data, MemSize size, RegAddr* reg){
//	size_t pos = GetName().find('.');
	//unsigned cpuId = std::stoul(GetName().substr(3, pos-3));
    ContextId contextId = 0; //MLDTODO Figure out where to get contextid from

	COMMIT{
		//if((address >> 63) == 0 ){//|| cpuId > 3){ //Ignore TLS
			//if(cpuId > 3){
    	DebugMemWrite("Read: address: 0x%lX, size:%lu", address, size);
			//}
		//}
	}

	Result result = Read2(contextId, address, data, size, reg);

	COMMIT{
		//if(cpuId > 3){
			if(result == SUCCESS){
				DebugMemWrite("Read: address: 0x%lX, size:%lu, result: %s, data: 0x%lX", address, size, resultStr(result).c_str(), *((uint64_t*)data));
			}else{
				DebugMemWrite("Read: address: 0x%lX, size:%lu, result: %s", address, size, resultStr(result).c_str());
			}
		//}
	}
	return result;
}


ExtendedResult DCache::Write(MemAddr address, void* data, MemSize size, LFID fid, TID tid)
{
//	size_t pos = GetName().find('.');
	//unsigned cpuId = std::stoul(GetName().substr(3, pos-3));
    ContextId contextId = 0; //MLDTODO Figure out where to get contextid from

	COMMIT{
		//if((address >> 63) == 0 ){//|| cpuId > 3){ //Ignore TLS
			//if(cpuId > 3){
    		DebugMemWrite("Write: address: 0x%lX, size:%lu, data: 0x%lX", address, size, *((uint64_t*)data));
			//}
		//}
	}

	ExtendedResult result = Write2(contextId, address, data, size, fid, tid);
//	COMMIT{
//		if((address | 0xff) == 0x801fffffffffffff){
//			std::cout << std::dec << GetKernel()->GetCycleNo() << ": write naar 0x" << std::hex << address << "! Data:0x" << std::hex << *((uint64_t*)data) << ", component:" << GetName() << std::dec << std::endl;
//		}
//	}

	COMMIT{
		//if(cpuId > 3){
			DebugMemWrite("Write: address: 0x%lX, size:%lu, result: %s", address, size, resultStr(result).c_str());
		//}
	}
	return result;
}

//DCache::DCache(const std::string& name, DRISC& parent, Clock& clock)
//:   Object(name, parent),
//    m_memory(NULL),
//    m_mcid(0),
//    m_lines(),
//    m_data(),
//    m_valid(0),
//
//    m_assoc          (GetConf("Associativity", size_t)),
//    m_sets           (GetConf("NumSets", size_t)),
//    m_lineSize       (GetTopConf("CacheLineSize", size_t)),
//    m_selector       (IBankSelector::makeSelector(*this, GetConf("BankSelector", string), m_sets)),
//    InitBuffer(m_lookup_responses, clock, "LookupResponsesBufferSize"),
//    InitBuffer(m_read_responses, clock, "ReadResponsesBufferSize"),
//    InitBuffer(m_write_responses, clock, "WriteResponsesBufferSize"),
//    InitBuffer(m_writebacks, clock, "ReadWritebacksBufferSize"),
//    InitBuffer(m_outgoing, clock, "OutgoingBufferSize"),
//    m_wbstate(),
//	m_mmu(&parent.getMMU()),
//    InitSampleVariable(numRHits, SVC_CUMULATIVE),//MLDTODO Registreren variabelen voorbeeld
//    InitSampleVariable(numDelayedReads, SVC_CUMULATIVE),
//    InitSampleVariable(numEmptyRMisses, SVC_CUMULATIVE),
//    InitSampleVariable(numInvalidRMisses, SVC_CUMULATIVE),
//    InitSampleVariable(numLoadingRMisses, SVC_CUMULATIVE),
//    InitSampleVariable(numHardConflicts, SVC_CUMULATIVE),
//    InitSampleVariable(numResolvedConflicts, SVC_CUMULATIVE),
//    InitSampleVariable(numWAccesses, SVC_CUMULATIVE),
//    InitSampleVariable(numWHits, SVC_CUMULATIVE),
//    InitSampleVariable(numPassThroughWMisses, SVC_CUMULATIVE),
//    InitSampleVariable(numLoadingWMisses, SVC_CUMULATIVE),
//    InitSampleVariable(numStallingRMisses, SVC_CUMULATIVE),
//    InitSampleVariable(numStallingWMisses, SVC_CUMULATIVE),
//    InitSampleVariable(numSnoops, SVC_CUMULATIVE),
//
//    InitProcess(p_LookupResponses, DoLookupResponses),
//    InitProcess(p_ReadWritebacks, DoReadWritebacks),
//    InitProcess(p_ReadResponses, DoReadResponses),
//    InitProcess(p_WriteResponses, DoWriteResponses),
//    InitProcess(p_Outgoing, DoOutgoingRequests),
//
//    p_service       (clock, GetName() + ".p_service")
//{
//
//	m_lookup_responses.Sensitive(p_LookupResponses);
//    m_writebacks.Sensitive(p_ReadWritebacks);
//    m_read_responses.Sensitive(p_ReadResponses);
//    m_write_responses.Sensitive(p_WriteResponses);
//    m_outgoing.Sensitive(p_Outgoing);
//
//    // These things must be powers of two
//    if (m_assoc == 0 || !IsPowerOfTwo(m_assoc))
//    {
//        throw exceptf<InvalidArgumentException>(*this, "DCacheAssociativity = %zd is not a power of two", (size_t)m_assoc);
//    }
//
//    if (m_sets == 0 || !IsPowerOfTwo(m_sets))
//    {
//        throw exceptf<InvalidArgumentException>(*this, "DCacheNumSets = %zd is not a power of two", (size_t)m_sets);
//    }
//
//    if (m_lineSize == 0 || !IsPowerOfTwo(m_lineSize))
//    {
//        throw exceptf<InvalidArgumentException>(*this, "CacheLineSize = %zd is not a power of two", (size_t)m_lineSize);
//    }
//
//    // At least a complete register value has to fit in a line
//    if (m_lineSize < 8)
//    {
//        throw exceptf<InvalidArgumentException>(*this, "CacheLineSize = %zd is less than 8.", (size_t)m_lineSize);
//    }
//
//    m_lines.resize(m_sets * m_assoc);
//    m_data.resize(m_lines.size() * m_lineSize);
//    m_valid = new bool[m_lines.size() * m_lineSize];
//
//    RegisterStateArray(m_valid, m_lines.size() * m_lineSize, "valid"); //MLDTODO Registreren variabelen voorbeeld
//    RegisterStateVariable(m_data, "data");//MLDTODO Registreren variabelen voorbeeld
//
//    for (size_t i = 0; i < m_lines.size(); ++i)
//    {
//        auto &line = m_lines[i];
//        line.state  = LINE_EMPTY;
//        line.data   = &m_data[i * m_lineSize];
//        line.valid  = &m_valid[i * m_lineSize];
//        line.create = false;
//        RegisterStateObject(line, "line" + to_string(i));
//    }
//
//    m_wbstate.size   = 0;
//    m_wbstate.offset = 0;
//    RegisterStateObject(m_wbstate, "wbstate");//MLDTODO Registreren variabelen voorbeeld
//}

//if(!initiateMemoryRequest(true, setIndex, pTag, cacheOffset, data, size)){
//	return FAILED;
//}
//Request request;
//request.write     = true;
//request.address   = unsplitAddress(0, setIndex, pTag);
//request.wid       = tid;

//
//COMMIT{
//	std::copy((char*)data, ((char*)data)+size, request.data.data+cacheOffset);
//	std::fill(request.data.mask, request.data.mask+cacheOffset, false);
//	std::fill(request.data.mask+cacheOffset, request.data.mask+cacheOffset+size, true);
//	std::fill(request.data.mask+cacheOffset+size, request.data.mask+m_lineSize, false);
//}
//
//if (!m_outgoing.Push(request))
//{
//    //++m_numStallingWMisses; MLDTODO Statistics
//    DeadlockWrite("Unable to push request to outgoing buffer");
//    return ExtendedResult::FAILED;
//}

bool DCache::initiateMemoryRequest
(bool write, MemAddr cacheOffset, size_t setIndex, MemAddr pTag, void* data, MemSize size, TID tid){

    Request request;
    request.write     = write;
    request.address   = unsplitAddress(0, setIndex, pTag);

    if(write){
    	//MLDNOTE wid=write id, continuation word opgeslagen (wachten tot alle writes gecommit, optioneel)
    	//MLDNOTE Bij store+TLB Miss --> Threads hebben ook een linkedlist. Threads suspenden. On request completion: Reschedule threads.
    	//MLDNOTE Beginnen met een stall.
    	//MLDTODO-DOC Hoort ook weer in scriptie.
    	request.wid = tid;
    	COMMIT{
    		std::copy((char*)data, ((char*)data)+size, request.data.data+cacheOffset);
    		std::fill(request.data.mask, request.data.mask+cacheOffset, false);
    		std::fill(request.data.mask+cacheOffset, request.data.mask+cacheOffset+size, true);
    		std::fill(request.data.mask+cacheOffset+size, request.data.mask+m_lineSize, false);
    	}
    }

	if (!m_outgoing.Push(request))
	{
//		++m_numStallingRMisses; //MLDTODO Fix statistics
		DeadlockWrite("Unable to push request to outgoing buffer");
		return false;
	}
	return true;
}

void DCache::ConnectMemory(IMemory* memory)
{
    assert(memory != NULL);
    assert(m_memory == NULL); // can't register two times

    m_memory = memory;
    StorageTraceSet traces;
    m_mcid = m_memory->RegisterClient(*this, p_Outgoing, traces, m_read_responses ^ m_write_responses, true);
    p_Outgoing.SetStorageTraces(traces);

}

DCache::~DCache()
{
    delete m_valid;
    delete m_selector;
}

void DCache::PushRegister(Line* line, RegAddr* reg){
    // Data is being loaded, add request to the queue
	if (reg != NULL && reg->valid())
	{
		// We're loading to a valid register, queue it
		RegAddr old   = line->waiting;
		line->waiting = *reg;
		*reg = old;
	}
	else
	{
		line->create  = true;
	}
}

bool DCache::OnTLBLookupCompleted(CID cid, mmu::TlbLineRef tlbLineRef, bool present){ //MLDTODO Modify cid type
    // Push the completion to the back of the queue
    LookupResponse response;
    response.cid   = cid;
    response.tlbLineRef = tlbLineRef;
    response.present = present;

    if (!m_lookup_responses.Push(response))
    {
        DeadlockWrite("Unable to push lookup completion to buffer");
        return false;
    }
    return true;
}

bool DCache::OnMemoryReadCompleted(MemAddr addr, const char* data)
{
	MemAddr cacheOffset, pTag;
	size_t setIndex;
	splitAddress(addr, cacheOffset, setIndex, &pTag);

    // Check if we have the line and if its loading.
    // This method gets called whenever a memory read completion is put on the
    // bus from memory, so we have to check if we actually need the data.
    Line* line = findLine(setIndex, pTag);

    if(line && line->state != LINE_FULL && !line->processing){
    	assert(line->state == LINE_LOADING || line->state == LINE_INVALID);


        // Registers are waiting on this data
        COMMIT
        {
    		/*
                      Merge with pending writes because the incoming line
                      will not know about those yet and we don't want inconsistent
                      content in L1.
                      This is kind of a hack; it's feasibility in hardware in a single cycle
                      is questionable.
            */
            char mdata[m_lineSize];

            std::copy(data, data + m_lineSize, mdata);

            for (Buffer<Request>::const_iterator p = m_outgoing.begin(); p != m_outgoing.end(); ++p)
            {
                if (p->write && p->address == addr)
                {
                    // This is a write to the same line, merge it
                    line::blit(&mdata[0], p->data.data, p->data.mask, m_lineSize);
                }
            }

            // Copy the data into the cache line.
            // Mask by valid bytes (don't overwrite already written data).
            line::blitnot(line->data, mdata, line->valid, m_lineSize);
            line::setifnot(line->valid, true, line->valid, m_lineSize);

            line->processing = true;
        }

        // Push the cache-line to the back of the queue
        ReadResponse response;
        response.cid = getLineId(line);

        DebugMemWrite("Received read completion for %#016llx -> CID %u", (unsigned long long)addr, (unsigned)response.cid);

        if (!m_read_responses.Push(response))
        {
            DeadlockWrite("Unable to push read completion to buffer");
            return false;
        }
    }else{
    	DebugMemWrite("Cannot find a loading/invalid line with address 0x%lX", addr); //MLDTODO Remove after debugging
    }
    return true;
}

bool DCache::OnMemoryWriteCompleted(WClientID wid)
{
    // Data has been written
    if (wid != INVALID_WCLIENTID) // otherwise for DCA
    {
        DebugMemWrite("Received write completion for client %u", (unsigned)wid);

        WriteResponse response;
        response.wid  =  wid;
        if (!m_write_responses.Push(response))
        {
            DeadlockWrite("Unable to push write completion to buffer");
            return false;
        }
    }
    return true;
}

bool DCache::OnMemorySnooped(MemAddr address, const char* data, const bool* mask)
{
	MemAddr cacheOffset, pTag;
	size_t setIndex;
	splitAddress(address, cacheOffset, setIndex, &pTag);
    Line *line = findLine(setIndex, pTag);

    // FIXME: snoops should really either lock the line or access
    // through a different port. Here we cannot (yet) invoke the
    // arbitrator because there is no scaffolding to declare which
    // processes can call OnMemorySnooped via AddProcess().
    /*
    if (!p_service.Invoke())
    {
        DeadlockWrite("Unable to acquire port for D-Cache snoop access (%#016llx, %zd)",
                      (unsigned long long)address, (size_t)data.size);

        return false;
    }
    */

    // Cache coherency: check if we have the same address
    if(line) //MLDTODO INVALID? LOADING? FULL?
    {
        DebugMemWrite("Received snoop request for loaded line %#016llx", (unsigned long long)address);

        COMMIT
        {
            // We do, update the data and mark written bytes as valid
            // Note that we don't have to check against already written data or queued reads
            // because we don't have to guarantee sequential semantics from other cores.
            // This falls within the non-determinism behavior of the architecture.
            line::blit(line->data, data, mask, m_lineSize);
            line::setif(line->valid, true, mask, m_lineSize);

            // Statistics
            ++m_numSnoops;
        }
    }
    return true;
}

bool DCache::OnMemoryInvalidated(MemAddr address)
{
    COMMIT
    {
    	MemAddr cacheOffset, pTag;
    	size_t setIndex;
    	splitAddress(address, cacheOffset, setIndex, &pTag);
        Line *line = findLine(setIndex, pTag);

        if(line)
        {
            DebugMemWrite("Received invalidation request for loaded line %#016llx", (unsigned long long)address);

            // We have the line, invalidate it
            if (line->state == LINE_FULL) {
                // Full lines are invalidated by clearing them. Simple.
                line->state = LINE_EMPTY;
            } else if (line->state == LINE_LOADING) {
                // The data is being loaded. Invalidate the line and it will get cleaned up
                // when the data is read.
                line->state = LINE_INVALID;
            }
        }
    }
    return true;
}

Object& DCache::GetMemoryPeer()
{
    return *GetParent();
}


void DCache::Cmd_Info(std::ostream& out, const std::vector<std::string>& /*arguments*/) const
{
    out <<
    "The Data Cache stores data from memory that has been used in loads and stores\n"
    "for faster access. Compared to a traditional cache, this D-Cache is extended\n"
    "with several fields to support the multiple threads and asynchronous operation.\n\n"
    "Supported operations:\n"
    "- inspect <component>\n"
    "  Display global information such as hit-rate and configuration.\n"
    "- inspect <component> buffers\n"
    "  Reads and display the outgoing request buffer.\n"
    "- inspect <component> lines\n"
    "  Reads and displays the cache-lines.\n";
}

void DCache::Cmd_Read(std::ostream& out, const std::vector<std::string>& arguments) const
{
    auto& regFile = GetDRISC().GetRegisterFile();

    if (arguments.empty())
    {
        out << "Cache type:          ";
        if (m_assoc == 1) {
            out << "Direct mapped" << endl;
        } else if (m_assoc == m_lines.size()) {
            out << "Fully associative" << endl;
        } else {
            out << dec << m_assoc << "-way set associative" << endl;
        }

        out << "L1 bank mapping:     " << m_selector->GetName() << endl
            << "Cache size:          " << dec << (m_lineSize * m_lines.size()) << " bytes" << endl
            << "Cache line size:     " << dec << m_lineSize << " bytes" << endl
            << endl;

        uint64_t numRAccesses = m_numRHits + m_numDelayedReads;

        uint64_t numRRqst = m_numEmptyRMisses + m_numResolvedConflicts;
        uint64_t numWRqst = m_numWAccesses;
        uint64_t numRqst = numRRqst + numWRqst;

        uint64_t numRStalls = m_numHardConflicts + m_numInvalidRMisses + m_numStallingRMisses;
        uint64_t numWStalls = m_numLoadingWMisses + m_numStallingWMisses;
        uint64_t numStalls = numRStalls + numWStalls;

        if (numRAccesses == 0 && m_numWAccesses == 0 && numStalls == 0)
            out << "No accesses so far, cannot provide statistical data." << endl;
        else
        {
            out << "***********************************************************" << endl
                << "                      Summary                              " << endl
                << "***********************************************************" << endl
                << endl << dec
                << "Number of read requests from client:  " << numRAccesses << endl
                << "Number of write requests from client: " << m_numWAccesses << endl
                << "Number of requests to upstream:       " << numRqst      << endl
                << "Number of snoops from siblings:       " << m_numSnoops  << endl
                << "Number of stalled cycles:             " << numStalls    << endl
                << endl;

#define PRINTVAL(X, q) dec << (X) << " (" << setprecision(2) << fixed << (X) * q << "%)"

            float r_factor = 100.0f / numRAccesses;
            out << "***********************************************************" << endl
                << "                      Cache reads                          " << endl
                << "***********************************************************" << endl
                << endl
                << "Number of read requests from client:                " << numRAccesses << endl
                << "Read hits:                                          " << PRINTVAL(m_numRHits, r_factor) << endl
                << "Read misses:                                        " << PRINTVAL(m_numDelayedReads, r_factor) << endl
                << "Breakdown of read misses:" << endl
                << "- to an empty line:                                 " << PRINTVAL(m_numEmptyRMisses, r_factor) << endl
                << "- to a loading line with same tag:                  " << PRINTVAL(m_numLoadingRMisses, r_factor) << endl
                << "- to a reusable line with different tag (conflict): " << PRINTVAL(m_numResolvedConflicts, r_factor) << endl
                << "(percentages relative to " << numRAccesses << " read requests)" << endl
                << endl;

            float w_factor = 100.0f / m_numWAccesses;
            out << "***********************************************************" << endl
                << "                      Cache writes                         " << endl
                << "***********************************************************" << endl
                << endl
                << "Number of write requests from client:                           " << m_numWAccesses << endl
                << "Breakdown of writes:" << endl
                << "- to a loaded line with same tag:                               " << PRINTVAL(m_numWHits, w_factor) << endl
                << "- to a an empty line or line with different tag (pass-through): " << PRINTVAL(m_numPassThroughWMisses, w_factor) << endl
                << "(percentages relative to " << m_numWAccesses << " write requests)" << endl
                << endl;

            float q_factor = 100.0f / numRqst;
            out << "***********************************************************" << endl
                << "                      Requests to upstream                 " << endl
                << "***********************************************************" << endl
                << endl
                << "Number of requests to upstream: " << numRqst  << endl
                << "Read requests:                  " << PRINTVAL(numRRqst, q_factor) << endl
                << "Write requests:                 " << PRINTVAL(numWRqst, q_factor) << endl
                << "(percentages relative to " << numRqst << " requests)" << endl
                << endl;


            if (numStalls != 0)
            {
                float s_factor = 100.f / numStalls;
                out << "***********************************************************" << endl
                    << "                      Stall cycles                         " << endl
                    << "***********************************************************" << endl
                    << endl
                    << "Number of stall cycles:               " << numStalls << endl
                    << "Read-related stalls:                  " << PRINTVAL(numRStalls, s_factor) << endl
                    << "Write-related stalls:                 " << PRINTVAL(numWStalls, s_factor) << endl
                    << "Breakdown of read-related stalls:" << endl
                    << "- read conflict to non-reusable line: " << PRINTVAL(m_numHardConflicts, s_factor) << endl
                    << "- read to invalidated line:           " << PRINTVAL(m_numInvalidRMisses, s_factor) << endl
                    << "- unable to send request upstream:    " << PRINTVAL(m_numStallingRMisses, s_factor) << endl
                    << "Breakdown of write-related stalls:" << endl
                    << "- writes to loading line:             " << PRINTVAL(m_numLoadingWMisses, s_factor) << endl
                    << "- unable to send request upstream:    " << PRINTVAL(m_numStallingWMisses, s_factor) << endl
                    << "(percentages relative to " << numStalls << " stall cycles)" << endl
                    << endl;
            }

        }
        return;
    }
    else if (arguments[0] == "buffers")
    {
        out << endl << "Read responses (CIDs):";
        for (auto& p : m_read_responses)
            out << ' ' << p.cid;
        out << "." << endl

            << endl << "Write responses (TIDs):";
        for (auto& p : m_write_responses)
            out << ' ' << p.wid;
        out << "." << endl

            << endl << "Writeback requests:" << endl;
        for (auto& p : m_writebacks)
        {
            out << "Data: " << hex << setfill('0');
;
            for (size_t x = 0; x < m_lineSize; ++x) {
                if (x && x % sizeof(Integer) == 0) out << ' ';
                out << setw(2) << (unsigned)(unsigned char)p.data[x];
            }
            out << dec << endl
                << "Waiting registers: ";
            RegAddr reg = p.waiting;
            while (reg != INVALID_REG)
            {
                RegValue value;
                regFile.ReadRegister(reg, value, true);

                out << ' ' << reg.str() << " (" << value.str(reg.type) << ')';

                if (value.m_state == RST_FULL || value.m_memory.size == 0)
                {
                    // Rare case: the request info is still in the pipeline, stall!
                    out << " !!";
                    break;
                }

                if (value.m_state != RST_PENDING && value.m_state != RST_WAITING)
                {
                    // We're too fast, wait!
                    out << " !!";
                    break;
                }
                reg = value.m_memory.next;
            }
            out << endl << endl;
        }
        out << endl << "Writeback state: value "
            << hex << showbase << m_wbstate.value << dec << noshowbase
            << " addr " << m_wbstate.addr.str()
            << " next " << m_wbstate.next.str()
            << " size " << m_wbstate.size
            << " offset " << m_wbstate.offset
            << " fid " << m_wbstate.fid
            << endl
            << endl << "Outgoing requests:" << endl
            << "      Address      | Type  | Value (writes)" << endl
            << "-------------------+-------+-------------------------" << endl;
        for (auto &p : m_outgoing)
        {
            out << hex << "0x" << setw(16) << setfill('0') << p.address << " | "
                << (p.write ? "Write" : "Read ") << " |";
            if (p.write)
            {
                out << hex << setfill('0');
                for (size_t x = 0; x < m_lineSize; ++x)
                {
                    if (p.data.mask[x])
                        out << " " << setw(2) << (unsigned)(unsigned char)p.data.data[x];
                    else
                        out << " --";
                }
            }
            out << dec << endl;
        }
        return;
    }

    out << "Set |       Address       |                       Data                      | Waiting Registers" << endl;
    out << "----+---------------------+-------------------------------------------------+-------------------" << endl;
    for (size_t i = 0; i < m_lines.size(); ++i)
    {
        const size_t set = i / m_assoc;
        const Line& line = m_lines[i];

        if (i % m_assoc == 0) {
            out << setw(3) << setfill(' ') << dec << right << set;
        } else {
            out << "   ";
        }

        if (line.state == LINE_EMPTY) {
            out << " |  <<<LINE_EMPTY>>>   |                                                 |";
        } else {
            out << " | "
                << hex << "0x" << setw(16) << setfill('0') << m_selector->Unmap(line.tag, set) * m_lineSize;

            switch (line.state)
            {
                case LINE_LOADING: out << "L"; break;
                case LINE_INVALID: out << "I"; break;
                default: out << "F";
            }
            out << " |";

            // Get the waiting registers
            std::vector<RegAddr> waiting;
            RegAddr reg = line.waiting;
            while (reg != INVALID_REG)
            {
                waiting.push_back(reg);
                RegValue value;
                regFile.ReadRegister(reg, value, true);

                if (value.m_state == RST_FULL || value.m_memory.size == 0)
                {
                    // Rare case: the request info is still in the pipeline, stall!
                    waiting.push_back(INVALID_REG);
                    break;
                }

                if (value.m_state != RST_PENDING && value.m_state != RST_WAITING)
                {
                    // We're too fast, wait!
                    waiting.push_back(INVALID_REG);
                    break;
                }

                reg = value.m_memory.next;
            }

            // Print the data
            out << hex << setfill('0');
            static const int BYTES_PER_LINE = 16;
            const int nLines = (m_lineSize + BYTES_PER_LINE + 1) / BYTES_PER_LINE;
            const int nWaitingPerLine = (waiting.size() + nLines + 1) / nLines;
            for (size_t y = 0; y < m_lineSize; y += BYTES_PER_LINE)
            {
                for (size_t x = y; x < y + BYTES_PER_LINE; ++x) {
                    out << " ";
                    if (line.valid[x]) {
                        out << setw(2) << (unsigned)(unsigned char)line.data[x];
                    } else {
                        out << "  ";
                    }
                }

                out << " |";

                // Print waiting registers for this line
                for (int w = 0; w < nWaitingPerLine; ++w)
                {
                    size_t index = y / BYTES_PER_LINE * nWaitingPerLine + w;
                    if (index < waiting.size())
                    {
                        RegAddr wreg = waiting[index];
                        out << " ";
                        if (wreg == INVALID_REG) out << "[...]"; // And possibly others
                        else out << wreg.str();
                    }
                }

                if (y + BYTES_PER_LINE < m_lineSize) {
                    // This was not yet the last line
                    out << endl << "    |                     |";
                }
            }
        }
        out << endl;
        out << ((i + 1) % m_assoc == 0 ? "----" : "    ");
        out << "+---------------------+-------------------------------------------------+-------------------" << endl;
    }

}

}
}
