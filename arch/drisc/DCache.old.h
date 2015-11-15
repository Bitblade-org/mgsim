// -*- c++ -*-
#ifndef DCACHE_H
#define DCACHE_H

#include <sim/kernel.h>
#include <sim/inspect.h>
#include <sim/buffer.h>
#include <arch/Memory.h>
#include <arch/drisc/forward.h>
#include <arch/simtypes.h>
#include "arch/drisc/mmu/MMU.h"

#define GET_LINE_ID(linePointer) (unsigned int)((Line*)linePointer - &m_lines[0])

namespace Simulator
{
namespace drisc
{

class DCache : public Object, public IMemoryCallback, public Inspect::Interface<Inspect::Read>
{
    friend class Simulator::DRISC;

public:
    /// The state of a cache-line
    enum LineState
    {
        LINE_EMPTY,      ///< Line is empty.
        LINE_LOADING,    ///< Line is being loaded.
        LINE_INVALID,    ///< Line is invalid.
        LINE_FULL        ///< Line is full.
    };

    // 
    // 
// ***** GENERATED CODE - DO NOT MODIFY  *****

struct Line
{
    
   MemAddr tag;
   ContextId contextTag;
   char* data;
   bool* valid;
   CycleNo access;
   RegAddr waiting;
   LineState state;
   bool processing;
   bool create;
   unsigned next;
   size_t tlbOffset;
   MemAddr pTag;
    SERIALIZE(__a) {
    __a & "[cb27";
     __a & tag;
     __a & contextTag;
     __a & access;
     __a & waiting;
     __a & state;
     __a & processing;
     __a & create;
     __a & next;
     __a & tlbOffset;
     __a & pTag;
  __a & "]";
}

    
};
// ***** END GENERATED CODE *****


private:
    // 
// ***** GENERATED CODE - DO NOT MODIFY  *****

struct Request
{
    
   MemData data;
   MemAddr address;
   WClientID wid;
   bool write;
    SERIALIZE(__a) {
    __a & "[eece";
     __a & data;
     __a & address;
     __a & wid;
     __a & write;
  __a & "]";
}

    
};
// ***** END GENERATED CODE *****


    // 
// ***** GENERATED CODE - DO NOT MODIFY  *****

struct LookupResponse
{
    
   CID cid;
   mmu::TlbLineRef tlbLineRef;
   bool present;
    SERIALIZE(__a) {
    __a & "[cbd4";
     __a & cid;
     __a & present;
  __a & "]";
}

    
};
// ***** END GENERATED CODE *****


    // 
// ***** GENERATED CODE - DO NOT MODIFY  *****

struct ReadResponse
{
    
   CID cid;
    SERIALIZE(__a) {
    __a & "[4b7c";
     __a & cid;
  __a & "]";
}

    
};
// ***** END GENERATED CODE *****


    // 
// ***** GENERATED CODE - DO NOT MODIFY  *****

struct WritebackRequest
{
    
    char data[ MAX_MEMORY_OPERATION_SIZE];
   RegAddr waiting;
    SERIALIZE(__a) {
    __a & "[c33f";
     __a & Serialization::binary(data, MAX_MEMORY_OPERATION_SIZE);
     __a & waiting;
  __a & "]";
}

    
};
// ***** END GENERATED CODE *****


    // 
// ***** GENERATED CODE - DO NOT MODIFY  *****

struct WriteResponse
{
    
   WClientID wid;
    SERIALIZE(__a) {
    __a & "[db98";
     __a & wid;
  __a & "]";
}

    
};
// ***** END GENERATED CODE *****


    // Information for multi-register writes
    // 
// ***** GENERATED CODE - DO NOT MODIFY  *****

struct WritebackState
{
    
   uint64_t value;
   RegAddr addr;
   RegAddr next;
   unsigned size;
   unsigned offset;
   LFID fid;
    SERIALIZE(__a) {
    __a & "[78e9";
     __a & value;
     __a & addr;
     __a & next;
     __a & size;
     __a & offset;
     __a & fid;
  __a & "]";
}

    
    WritebackState (
         uint64_t _value, 
         RegAddr _addr, 
         RegAddr _next, 
         unsigned _size, 
         unsigned _offset, 
         LFID _fid) :
      value(_value), 
      addr(_addr), 
      next(_next), 
      size(_size), 
      offset(_offset), 
      fid(_fid){}

    WritebackState():
      value( 0 ) ,
      addr( INVALID_REG ) ,
      next( INVALID_REG ) ,
      size( 0 ) ,
      offset( 0 ) ,
      fid( 0 ) {}
};
// ***** END GENERATED CODE *****

	void splitAddress(MemAddr addr, MemAddr &cacheOffset, MemAddr &cacheIndex, MemAddr *vTag);

	//Result FindLine(MemAddr address, ContextId contextId, Line* &line, bool check_only, bool ignore_tags);
	Line& fetchLine(MemAddr address);
	bool comparePTag(Line &line, MemAddr pTag);
	bool compareCTag(Line &line, CID cid);
	bool getEmptyLine(MemAddr address, Line* &line);
	bool freeLine(Line &line);

    //Result ReverseFindLine(MemAddr pAddr, Line* &line);

    IMemory*             m_memory;          	///< Memory
    MCID                 m_mcid;            	///< Memory Client ID
    std::vector<Line>    m_lines;           	///< The cache-lines.
    std::vector<char>    m_data;            	///< The data in the cache lines.
    bool*                m_valid;           	///< The valid bits.
    size_t               m_assoc;           	///< Config: Cache associativity.
    size_t               m_sets;            	///< Config: Number of sets in the cace.
    size_t               m_lineSize;        	///< Config: Size of a cache line, in bytes.
    IBankSelector*       m_selector;        	///< Mapping of cache line addresses to tags and set indices.
    Buffer<LookupResponse> m_lookup_responses; 	///< Incoming buffer for lookup responses from TLB.
    Buffer<ReadResponse>  m_read_responses; 	///< Incoming buffer for read responses from memory bus.
    Buffer<WriteResponse> m_write_responses;	///< Incoming buffer for write acknowledgements from memory bus.
    Buffer<WritebackRequest> m_writebacks;  	///< Incoming buffer for register writebacks after load.
    Buffer<Request>      m_outgoing;        	///< Outgoing buffer to memory bus.
    WritebackState       m_wbstate;         	///< Writeback state
    mmu::MMU*			 m_mmu;					///< Memory Management Unit


    // Statistics

    DefineSampleVariable(uint64_t, numRHits);
    DefineSampleVariable(uint64_t, numDelayedReads);
    DefineSampleVariable(uint64_t, numEmptyRMisses);
    DefineSampleVariable(uint64_t, numInvalidRMisses);
    DefineSampleVariable(uint64_t, numLoadingRMisses);
    DefineSampleVariable(uint64_t, numHardConflicts);
    DefineSampleVariable(uint64_t, numResolvedConflicts);

    DefineSampleVariable(uint64_t, numWAccesses);
    DefineSampleVariable(uint64_t, numWHits);
    DefineSampleVariable(uint64_t, numPassThroughWMisses);
    DefineSampleVariable(uint64_t, numLoadingWMisses);

    DefineSampleVariable(uint64_t, numStallingRMisses);
    DefineSampleVariable(uint64_t, numStallingWMisses);

    DefineSampleVariable(uint64_t, numSnoops);


    void PushRegister(Line* line, RegAddr* reg);
    Line* getLineById(size_t id) { return &m_lines[id]; }
    size_t getLineId(Line* line) { return (size_t)(line - &m_lines[0]) / sizeof(Line); }

    Result DoLookupResponses();
    Result DoReadWritebacks();
    Result DoReadResponses();
    Result DoWriteResponses();
    Result DoOutgoingRequests();

    Object& GetDRISCParent() const { return *GetParent(); }

public:
    DCache(const std::string& name, DRISC& parent, Clock& clock);
    DCache(const DCache&) = delete;
    DCache& operator=(const DCache&) = delete;
    ~DCache();
    void ConnectMemory(IMemory* memory);

    // Processes
    Process p_LookupResponses;
    Process p_ReadWritebacks;
    Process p_ReadResponses;
    Process p_WriteResponses;
    Process p_Outgoing;

    ArbitratedService<> p_service;

    // Public interface
    Result Read (MemAddr address, void* data, MemSize size, RegAddr* reg);
    Result Write(MemAddr address, void* data, MemSize size, LFID fid, TID tid);

    Result Read2 (ContextId contextId, MemAddr address, void* data, MemSize size, RegAddr* reg);
    Result Write2(ContextId contextId, MemAddr address, void* data, MemSize size, LFID fid, TID tid);

    size_t GetLineSize() const { return m_lineSize; }

    // TLB callbacks
    bool OnTLBLookupCompleted(CID cid, mmu::TlbLineRef tlbLineRef, bool present);

    // Memory callbacks
    bool OnMemoryReadCompleted(MemAddr addr, const char* data) override;
    bool OnMemoryWriteCompleted(TID tid) override;
    bool OnMemoryReadCompleted2(MemAddr addr, const char* data);
    bool OnMemoryWriteCompleted2(TID tid);
    bool OnMemorySnooped(MemAddr addr, const char* data, const bool* mask) override;
    bool OnMemoryInvalidated(MemAddr addr) override;

    Object& GetMemoryPeer() override;


    // Debugging
    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void Cmd_Read(std::ostream& out, const std::vector<std::string>& arguments) const override;

    size_t GetAssociativity() const { return m_assoc; }
    size_t GetNumLines()      const { return m_lines.size(); }
    size_t GetNumSets()       const { return m_sets; }

    const Line& GetLine(size_t i) const { return m_lines[i];  }
};

}
}

#endif
