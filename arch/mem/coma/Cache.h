#ifndef COMA_CACHE_H
#define COMA_CACHE_H

#include "Node.h"
#include "sim/inspect.h"
#include "arch/BankSelector.h"
#include <queue>
#include <set>

class Config;

namespace Simulator
{

class COMA::Cache : public COMA::Node, public Inspect::Interface<Inspect::Read>
{
public:
    enum LineState
    {
        LINE_EMPTY,     ///< Empty, can be used.
        LINE_LOADING,   ///< Allocated, read request sent.
        LINE_FULL,      ///< Allocated and data present.
    };
    
    struct Line
    {
        LineState    state;     ///< State of the line
        MemAddr      tag;       ///< Tag of the line
        char*        data;      ///< Data of the line
        CycleNo      access;    ///< Last access time (for LRU replacement)
        unsigned int tokens;    ///< Number of tokens in this line
        bool         dirty;     ///< Dirty: line has been written to
        unsigned int updating;  ///< Number of REQUEST_UPDATEs pending on this line
        bool         valid[MAX_MEMORY_OPERATION_SIZE]; ///< Validity bitmask
    };

private:    
    struct Request : public MemData
    {
        bool         write;
        bool         consistency;
        bool         mask[MAX_MEMORY_OPERATION_SIZE];
        MemAddr      address;
        unsigned int client;
        LFID         fid;
    };

    IBankSelector&                m_selector;
    size_t                        m_lineSize;
    size_t                        m_assoc;
    size_t                        m_sets;
    CacheID                       m_id;
    std::vector<IMemoryCallback*> m_clients;
    StorageTraceSet               m_storages;
    ArbitratedService<>           p_lines;
    std::vector<Line>             m_lines;
    std::vector<char>             m_data;
    
    // Statistics
    uint64_t                      m_numEmptyRMisses; //hitting an empty line
    uint64_t                      m_numEmptyWMisses;
    uint64_t                      m_numHardRConflicts; //hitting null line
    uint64_t                      m_numHardWConflicts;
    uint64_t                      m_numInjectedEvictions;
    uint64_t                      m_numLoadingRMisses; //hitting loading line
    uint64_t                      m_numLoadingWMisses;
    uint64_t                      m_numMergedEvictions;
    uint64_t                      m_numNetworkRHits;
    uint64_t                      m_numNetworkWHits;
    uint64_t                      m_numRHits;
    uint64_t                      m_numResolvedRConflicts; //hit empty line after eviction
    uint64_t                      m_numResolvedWConflicts;
    uint64_t                      m_numStallingRHits;      //stalled notifying read completion
    uint64_t                      m_numStallingRMessage;   //stalled message insertion 
    uint64_t                      m_numStallingWHits;      //stalled notifying write completion
    uint64_t                      m_numStallingWMessage;   //stalled message insertion
    uint64_t                      m_numStallingEviction;   //stalled cache line eviction
    uint64_t                      m_numWEHits;             //Exclusive Write hit 
    uint64_t                      m_numWSHits;             //Shared Write hit 
    
  
   
    // Processes
    Process p_Requests;
    Process p_In;

    // Incoming requests from the processors
    // First arbitrate, then buffer (models a bus)
    ArbitratedService<PriorityCyclicArbitratedPort> p_bus;
    Buffer<Request>     m_requests;
    Buffer<MemData>     m_responses;
    
    Line* FindLine(MemAddr address);
    Line* AllocateLine(MemAddr address, bool empty_only, MemAddr *ptag = NULL);
    bool  EvictLine(Line* line, const Request& req);

    // Processes
    Result DoRequests();
    Result DoReceive();

    Result OnReadRequest(const Request& req);
    Result OnWriteRequest(const Request& req);
    bool OnMessageReceived(Message* msg);
    bool OnReadCompleted(MemAddr addr, const MemData& data);
public:
    Cache(const std::string& name, COMA& parent, Clock& clock, CacheID id, Config& config);
    
    size_t GetLineSize() const { return m_lineSize; }
    size_t GetNumSets() const { return m_sets; }
    size_t GetAssociativity() const { return m_assoc; }
    
    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const;
    void Cmd_Read(std::ostream& out, const std::vector<std::string>& arguments) const;
    const Line* FindLine(MemAddr address) const;
    
    MCID RegisterClient  (IMemoryCallback& callback, Process& process, StorageTraceSet& traces, Storage& storage);
    void UnregisterClient(MCID id);
    bool Read (MCID id, MemAddr address, MemSize size);
    bool Write(MCID id, MemAddr address, const void* data, MemSize size, LFID fid, const bool* mask, bool consistency);
};

}
#endif
