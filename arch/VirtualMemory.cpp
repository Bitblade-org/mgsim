#include "VirtualMemory.h"
#include "Memory.h"

#include <sim/except.h>
#include <sim/sampling.h>

#include <cstring>
#include <limits>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <sstream>

using namespace std;

namespace Simulator
{

// We allocate per block, this is the size of each block. Must be a power of two
#define BLOCK_SIZE (1 << 12)

void VirtualMemory::ReportOverlap(MemAddr address, MemSize size) const
{
    ostringstream os;
    VirtualMemory::Cmd_Info(os, vector<string>());
    InvalidArgumentException e = exceptf<InvalidArgumentException>("Overlap in memory reservation (%#016llx, %zd)",
                                                                   (unsigned long long)address, (size_t)size);
    e.AddDetails(os.str());
    throw e;
}

void VirtualMemory::Reserve(MemAddr address, MemSize size, ProcessID pid, int perm)
{
    if (size != 0)
    {
        // Check that there is no overlap
        auto p = m_ranges.lower_bound(address);
        if (p != m_ranges.end())
        {
            if (p->first == address || (address < p->first && address + size > p->first))
            {
                // The range overlaps with an existing range after it
                ReportOverlap(address, size);
            }
        }
        if (p != m_ranges.begin())
        {
            auto q = p; --q;
            assert(q->first < address);
            if (q->first + q->second.size > address)
            {
                // The range overlaps with an existing range before it
                ReportOverlap(address, size);
            }
        }

        Range range;
        range.size        = size;
        range.owner       = pid;
        range.permissions = perm;
        m_ranges.insert(p, make_pair(address, range));
        m_total_reserved += size;
        ++m_number_of_ranges;
    }
}

VirtualMemory::RangeMap::const_iterator VirtualMemory::GetReservationRange(MemAddr address, MemSize size) const
{
    auto p = m_ranges.lower_bound(address);
    if (p != m_ranges.begin() && (p == m_ranges.end() || p->first > address))
    {
        --p;
    }
    return (p != m_ranges.end() &&
            address >= p->first && p->second.size >= size &&
            address <= p->first + (p->second.size - size)) ? p : m_ranges.end();
}

void VirtualMemory::Unreserve(MemAddr address, MemSize size)
{
    auto p = m_ranges.find(address);
    if (p == m_ranges.end())
    {
        throw exceptf<InvalidArgumentException>("Attempting to unreserve non-reserved memory (%#016llx)",
                                                (unsigned long long)address);
    }

    if (size != p->second.size)
    {
        throw exceptf<InvalidArgumentException>("Cannot unreserve %#016llx bytes from %#016llx, reservation was for %#016llx bytes",
                                                (unsigned long long)size,
                                                (unsigned long long)address,
                                                (unsigned long long)p->second.size);
    }
    m_total_reserved -= p->second.size;
    --m_number_of_ranges;
    m_ranges.erase(p);
}

void VirtualMemory::UnreserveAll(ProcessID pid)
{
    // unreserve all ranges belonging to a given process ID

    for (auto p = m_ranges.begin(); p != m_ranges.end(); )
    {
        if (p->second.owner == pid)
        {
            m_total_reserved -= p->second.size;
            --m_number_of_ranges;
            m_ranges.erase(p++); // careful that iterator is invalidated by erase()
        }
        else
            ++p;
    }
}

bool VirtualMemory::CheckPermissions(MemAddr address, MemSize size, int access) const
{
#if MEMSIZE_MAX >= SIZE_MAX
    if (size > SIZE_MAX)
    {
        throw exceptf<InvalidArgumentException>("CheckPerm (%#016llx, %zd): Size argument too big",
                                                (unsigned long long)address, (size_t)size);
    }
#endif

    auto p = GetReservationRange(address, size);
    return (p != m_ranges.end() && (p->second.permissions & access) == access);
}

void VirtualMemory::Read(MemAddr address, void* _data, MemSize size) const
{
#if MEMSIZE_MAX >= SIZE_MAX
    if (size > SIZE_MAX)
    {
        throw exceptf<InvalidArgumentException>("Read (%#016llx, %zd): Size argument too big",
                                                (unsigned long long)address, (size_t)size);
    }
#endif

    MemAddr base   = address & -BLOCK_SIZE;     // Base address of block containing address
    size_t  offset = (size_t)(address - base);      // Offset within base block of address
    char*   data   = static_cast<char*>(_data);     // Byte-aligned pointer to destination

    for (auto pos = m_blocks.lower_bound(base); size > 0;)
    {
        if (pos == m_blocks.end())
        {
            // Rest of address range does not exist, fill with zero
            fill(data, data + size, 0);
            break;
        }

        // Number of bytes to read, initially
        size_t count = min( (size_t)size, (size_t)BLOCK_SIZE);

        if (pos->first > base) {
            // This part of the request does not exist, fill with zero
            fill(data, data + count, 0);
        } else {
            // Read data
            copy(pos->second.data + offset, pos->second.data + offset + count, data);
            ++pos;
        }
        size  -= count;
        data  += count;
        base  += BLOCK_SIZE;
        offset = 0;
    }
}

void VirtualMemory::Write(MemAddr address, const void* _data, const bool* mask, MemSize size)
{
#if MEMSIZE_MAX >= SIZE_MAX
    if (size > SIZE_MAX)
    {
        throw exceptf<InvalidArgumentException>("Write (%#016llx, %zd): Size argument too big",
                                                (unsigned long long)address, (size_t)size);
    }
#endif

    MemAddr     base   = address & -BLOCK_SIZE;                     // Base address of block containing address
    size_t      offset = (size_t)(address - base);          // Offset within base block of address
    const char* data   = static_cast<const char*>(_data);   // Byte-aligned pointer to destination

    while (size > 0)
    {
        // Find or insert the block
        auto ins = m_blocks.insert(make_pair(base, Block()));

        auto& pos = ins.first;
        if (ins.second) {
            // A new element was inserted, allocate and clear memory
            memset(pos->second.data, 0, BLOCK_SIZE);
            m_total_allocated += BLOCK_SIZE;
        }

        // Number of bytes to write, initially
        size_t count = min( (size_t)size, (size_t)BLOCK_SIZE - offset);

        // Write data
        for (size_t i = 0; i < count; ++i)
            if (mask == 0 || mask[i])
                pos->second.data[offset + i] = data[i];

        size  -= count;
        data  += count;
        if (mask != 0)
            mask  += count;
        base  += BLOCK_SIZE;
        offset = 0;
    }
}

void VirtualMemory::SetSymbolTable(SymbolTable& symtable)
{
    m_symtable = &symtable;
}

SymbolTable& VirtualMemory::GetSymbolTable() const
{
    return *m_symtable;
}


VirtualMemory::VirtualMemory(const std::string& name, Object& parent)
    : Object(name, parent),
      m_blocks(),
      m_ranges(),
      InitSampleVariable(total_reserved, SVC_LEVEL),
      InitSampleVariable(total_allocated, SVC_LEVEL),
      InitSampleVariable(number_of_ranges, SVC_LEVEL),
      m_symtable(0)
{
    RegisterStateObject(m_blocks, "blocks");
    RegisterStateObject(m_ranges, "ranges");
}

VirtualMemory::~VirtualMemory()
{
}

void VirtualMemory::Cmd_Info(ostream& out, const vector<string>& /* arguments */) const
{
    out << "Memory range                        | P   | DCAP | Owner PID" << endl
        << "------------------------------------+-----+------+------------" << endl;

    out << hex << setfill('0');

    MemSize total = 0;
    auto p = m_ranges.begin();
    if (p != m_ranges.end())
    {
        // We have at least one range, walk over all ranges and
        // coalesce neighbouring ranges with similar properties.
        MemAddr   begin = p->first;
        ProcessID owner = p->second.owner;
        int       perm  = p->second.permissions;
        MemAddr   size  = 0;

        do
        {
            size  += p->second.size;
            total += p->second.size;
            p++;
            if (p == m_ranges.end() || p->first > begin + size
                || p->second.permissions != perm || p->second.owner != owner)
            {
                // Different block, or end of blocks
                out << setw(16) << begin << " - " << setw(16) << begin + size - 1
                    << " | "
                    << (perm & IMemory::PERM_READ    ? "R" : ".")
                    << (perm & IMemory::PERM_WRITE   ? "W" : ".")
                    << (perm & IMemory::PERM_EXECUTE ? "X" : ".")
                    << " | "
                    << (perm & IMemory::PERM_DCA_READ ? "DR" : "..")
                    << (perm & IMemory::PERM_DCA_WRITE ? "DW" : "..")
                    << " | "
                    << dec << owner << hex
                    << endl;
                if (p != m_ranges.end())
                {
                    // Different block
                    begin = p->first;
                    owner = p->second.owner;
                    perm  = p->second.permissions;
                    size  = 0;
                }
            }
        } while (p != m_ranges.end());
    }

    static const char* Mods[] = { "B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };

    // Print total memory reservation
    assert(m_total_reserved == total);
    int mod;
    for(mod = 0; total >= 1024 && mod < 9; ++mod)
    {
        total /= 1024;
    }
    out << endl << setfill(' ') << dec;
    out << "Total reserved memory:  " << setw(4) << total << " " << Mods[mod] << endl;

    total = m_blocks.size() * BLOCK_SIZE;
    assert(m_total_allocated == total);
    // Print total memory usage
    for (mod = 0; total >= 1024 && mod < 4; ++mod)
    {
        total /= 1024;
    }
    out << "Total allocated memory: " << setw(4) << total << " " << Mods[mod] << endl;
}

}
