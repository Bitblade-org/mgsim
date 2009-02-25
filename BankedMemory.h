#ifndef BANKEDMEMORY_H
#define BANKEDMEMORY_H

#include <queue>
#include <set>
#include "Memory.h"
#include "kernel.h"
#include "VirtualMemory.h"

namespace Simulator
{

class ArbitratedWriteFunction;

class BankedMemory : public IComponent, public IMemory, public IMemoryAdmin, public VirtualMemory
{
public:
	struct Config
	{
        CycleNo    baseRequestTime;
        CycleNo    timePerLine;
        size_t     sizeOfLine;
        size_t     bufferSize;
        size_t     numBanks;
	};

    class Request
    {
        void release();

    public:
        unsigned long*   refcount;
        IMemoryCallback* callback;
        bool             write;
        MemAddr          address;
        MemData          data;

        Request& operator =(const Request& req);
        Request(const Request& req);
        Request();
        ~Request();
    };
    
    typedef std::multimap<CycleNo, Request> Pipeline;
   
    struct Bank
    {
        bool     busy;
        Request  request;
        CycleNo  done;
        Pipeline incoming;
        Pipeline outgoing;
        
        Bank();
    };

    BankedMemory(Object* parent, Kernel& kernel, const std::string& name, const Config& config, size_t nProcs);
    
    const std::vector<Bank>& GetBanks() const { return m_banks; }

    size_t GetBankFromAddress(MemAddr address) const;
    
    // Component
    Result OnCycleWritePhase(unsigned int stateIndex);

    // IMemory
    void   RegisterListener  (IMemoryCallback& callback);
    void   UnregisterListener(IMemoryCallback& callback);
    Result Read (IMemoryCallback& callback, MemAddr address, void* data, MemSize size, MemTag tag);
    Result Write(IMemoryCallback& callback, MemAddr address, void* data, MemSize size, MemTag tag);
	bool   CheckPermissions(MemAddr address, MemSize size, int access) const;

    // IMemoryAdmin
    void Read (MemAddr address, void* data, MemSize size);
    void Write(MemAddr address, const void* data, MemSize size, int perm = 0);

private:
    Config                      m_config;
    std::set<IMemoryCallback*>  m_caches;
    std::vector<Bank>           m_banks;
    
    void AddRequest(Pipeline& queue, const Request& request, bool data);
};

}
#endif

