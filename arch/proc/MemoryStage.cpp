#include "Processor.h"
#include "sim/breakpoints.h"
#include "sim/sampling.h"
#include <cassert>

namespace Simulator
{

Processor::Pipeline::PipeAction Processor::Pipeline::MemoryStage::OnCycle()
{
    PipeValue rcv = m_input.Rcv;

    unsigned inload = 0;
    unsigned instore = 0;
    
    if (m_input.size > 0)
    {
        // It's a new memory operation!
        assert(m_input.size <= sizeof(uint64_t));

        Result result = SUCCESS;
        if (rcv.m_state == RST_FULL)
        {
            // Memory write

            // Check for breakpoints
            GetKernel()->GetBreakPoints().Check(BreakPoints::WRITE, m_input.address, *this);

            // Serialize and store data
            char data[MAX_MEMORY_OPERATION_SIZE];

            uint64_t value = 0;
            switch (m_input.Rc.type) {
            case RT_INTEGER: value = m_input.Rcv.m_integer.get(m_input.Rcv.m_size); break;
            case RT_FLOAT:   value = m_input.Rcv.m_float.toint(m_input.Rcv.m_size); break;
            default: assert(0);
            }

            
            
            SerializeRegister(m_input.Rc.type, value, data, (size_t)m_input.size);

            MMIOInterface& mmio = m_parent.GetProcessor().GetMMIOInterface();
            if (mmio.IsRegisteredWriteAddress(m_input.address, m_input.size))
            {
                result = mmio.Write(m_input.address, data, m_input.size, m_input.fid, m_input.tid);

                if (!result)
                {
                    throw exceptf<VirtualIOException>(*this, "Failed I/O write by %s (F%u/T%u): %#016llx (%zd)",
                                                      GetKernel()->GetSymbolTable()[m_input.pc].c_str(),
                                                      (unsigned)m_input.fid, (unsigned)m_input.tid,
                                                      (unsigned long long)m_input.address, (size_t)m_input.size);
                }
            }
            else
            {
                // Normal request to memory
                if ((result = m_dcache.Write(m_input.address, data, m_input.size, m_input.fid, m_input.tid)) == FAILED)
                {
                    // Stall
                    return PIPE_STALL;
                }
            }

            // Clear the register state so it won't get written to the register file
            rcv.m_state = RST_INVALID;

            // Prepare for count increment
            instore = m_input.size;

            DebugMemWrite("Store by %s (F%u/T%u): *%#016llx <- %#016llx (%zd)",
                          GetKernel()->GetSymbolTable()[m_input.pc].c_str(), (unsigned)m_input.fid, (unsigned)m_input.tid,
                          (unsigned long long)m_input.address, (unsigned long long)value, (size_t)m_input.size);
        }
        // Memory read
        else if (m_input.address >= 4 && m_input.address < 8)
        {
            // Special range. Rather hackish.
            // Note that we exclude address 0 from this so NULL pointers are still invalid.

            // Invalid address; don't send request, just clear register
            rcv = MAKE_EMPTY_PIPEVALUE(rcv.m_size);
        }
        else if (m_input.Rc.valid())
        {
            // Memory read

            // Check for breakpoints
            GetKernel()->GetBreakPoints().Check(BreakPoints::READ, m_input.address, *this);

            char data[MAX_MEMORY_OPERATION_SIZE];
            RegAddr reg = m_input.Rc;

            MMIOInterface& mmio = m_parent.GetProcessor().GetMMIOInterface();
            if (mmio.IsRegisteredReadAddress(m_input.address, m_input.size))
            {
                result = mmio.Read(m_input.address, data, m_input.size, m_input.fid, m_input.tid);

                // We do not support async reads yet with MMIO, so just fail.
                if (!result)
                {
                    throw exceptf<VirtualIOException>(*this, "Failed I/O read by %s (F%u/T%u): %#016llx (%zd)",
                                                      GetKernel()->GetSymbolTable()[m_input.pc].c_str(),
                                                      (unsigned)m_input.fid, (unsigned)m_input.tid,
                                                      (unsigned long long)m_input.address, (size_t)m_input.size);
                }
            }
            else
            {
                // Normal read from memory.

                if ((result = m_dcache.Read(m_input.address, data, m_input.size, m_input.fid, &reg)) == FAILED)
                {
                    // Stall
                    return PIPE_STALL;
                }
            }

            // Prepare for counter increment
            inload = m_input.size;

            rcv.m_size = m_input.Rcv.m_size;
            if (result == SUCCESS)
            {
                // Unserialize and store data
                uint64_t value = UnserializeRegister(m_input.Rc.type, data, (size_t)m_input.size);

                if (m_input.sign_extend)
                {
                    // Sign-extend the value
                    size_t shift = (sizeof(value) - (size_t)m_input.size) * 8;
                    value = (int64_t)(value << shift) >> shift;
                }
                
                rcv.m_state = RST_FULL;
                switch (m_input.Rc.type)
                {
                case RT_INTEGER: rcv.m_integer.set(value, rcv.m_size); break;
                case RT_FLOAT:   rcv.m_float.fromint(value, rcv.m_size); break;
                default:         assert(0);
                }
                DebugMemWrite("Load by %s (F%u/T%u): *%#016llx -> %#016llx (%zd)",
                              GetKernel()->GetSymbolTable()[m_input.pc].c_str(), (unsigned)m_input.fid, (unsigned)m_input.tid,
                              (unsigned long long)m_input.address, (unsigned long long)value, (size_t)m_input.size); 
            }
            else
            {
                // Remember request data
                rcv = MAKE_PENDING_PIPEVALUE(rcv.m_size);
                rcv.m_memory.fid         = m_input.fid;
                rcv.m_memory.next        = reg;
                rcv.m_memory.offset      = (unsigned int)(m_input.address % m_dcache.GetLineSize());
                rcv.m_memory.size        = (size_t)m_input.size;
                rcv.m_memory.sign_extend = m_input.sign_extend;
                DebugMemWrite("Load by %s: *%#016llx -> delayed %s (%zd)",
                              GetKernel()->GetSymbolTable()[m_input.pc].c_str(), 
                              (unsigned long long)m_input.address, 
                              m_input.Rc.str().c_str(), (size_t)m_input.size); 
            }
        }

        if (result == DELAYED)
        {
            // Increase the outstanding memory count for the family
            if (m_input.Rcv.m_state == RST_FULL)
            {
                if (!m_allocator.IncreaseThreadDependency(m_input.tid, THREADDEP_OUTSTANDING_WRITES))
                {
                    return PIPE_STALL;
                }
            }
            else if (!m_allocator.OnMemoryRead(m_input.fid))
            {
                return PIPE_STALL;
            }
        }
    }

    COMMIT
    {
        // Copy common latch data
        (CommonData&)m_output = m_input;
        
        m_output.suspend = m_input.suspend;
        m_output.Rc      = m_input.Rc;
        m_output.Rrc     = m_input.Rrc;
        m_output.Rcv     = rcv;

        // Increment counters
        m_loads += !!inload;
        m_load_bytes += inload;
        m_stores += !!instore;
        m_store_bytes += instore;
    }
    return PIPE_CONTINUE;
}

Processor::Pipeline::MemoryStage::MemoryStage(Pipeline& parent, Clock& clock, const ExecuteMemoryLatch& input, MemoryWritebackLatch& output, DCache& dcache, Allocator& alloc, const Config& /*config*/)
    : Stage("memory", parent, clock),
      m_input(input),
      m_output(output),
      m_allocator(alloc),
      m_dcache(dcache),
      m_loads(0),
      m_stores(0),
      m_load_bytes(0),
      m_store_bytes(0),
      m_nCycleSampleOps(0), 
      m_nOtherSampleOps(0)
{
    RegisterSampleVariableInObject(m_loads, SVC_CUMULATIVE);
    RegisterSampleVariableInObject(m_stores, SVC_CUMULATIVE);
    RegisterSampleVariableInObject(m_load_bytes, SVC_CUMULATIVE);
    RegisterSampleVariableInObject(m_store_bytes, SVC_CUMULATIVE);
    RegisterSampleVariableInObject(m_nCycleSampleOps, SVC_CUMULATIVE);
    RegisterSampleVariableInObject(m_nOtherSampleOps, SVC_CUMULATIVE);
}
    
}