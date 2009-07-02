#include "ThreadTable.h"
#include "Processor.h"
#include "config.h"
#include "range.h"
#include <cassert>
#include <iomanip>
using namespace std;

namespace Simulator
{

ThreadTable::ThreadTable(Processor& parent, const Config& config)
  : Structure<TID>(&parent, parent.GetKernel(), "threads"),
    m_parent(parent),
    m_threads(config.getInteger<size_t>("NumThreads", 64)),
    m_numThreadsUsed(0)
{
    for (TID i = 0; i < m_threads.size(); ++i)
    {
        m_threads[i].nextMember = i + 1;
        m_threads[i].state      = TST_EMPTY;
    }
    m_threads[m_threads.size() - 1].nextMember = INVALID_TID;

    m_empty.head = 0;
    m_empty.tail = m_threads.size() - 1;
}

TID ThreadTable::PopEmpty()
{
    TID tid = m_empty.head;
    if (tid != INVALID_TID)
    {
        assert(m_threads[tid].state == TST_EMPTY);
        assert(m_numThreadsUsed < m_threads.size());
        COMMIT
        {
            m_empty.head = m_threads[tid].nextMember;
            m_threads[tid].state = TST_WAITING;
            m_numThreadsUsed++;
        }
    }
    return tid;
}

bool ThreadTable::PushEmpty(const ThreadQueue& q)
{
    assert(q.head != INVALID_TID);
    assert(q.tail != INVALID_TID);
    
    COMMIT
    {
        if (m_empty.head == INVALID_TID) {
            m_empty.head = q.head;
        } else {
            m_threads[m_empty.tail].nextMember = q.head;
        }
        m_empty.tail = q.tail;
        m_threads[q.tail].nextMember = INVALID_TID;

        // Admin, set states to empty
        for (TID cur = q.head; cur != INVALID_TID; cur = m_threads[cur].nextMember)
        {
            assert(m_numThreadsUsed > 0);
            DebugSimWrite("Decrementing for T%u", cur);
            m_threads[cur].state = TST_EMPTY;
            m_numThreadsUsed--;
        }
    }
    return true;
}

void ThreadTable::Cmd_Help(ostream& out, const vector<string>& /* arguments */) const
{
    out <<
    "The Thread Table is the storage area in a processor that stores all thread's\n"
    "information. It contains information about the thread's state, execution\n"
    "context, dependencies and much more.\n\n"
    "Supported operations:\n"
    "- read <component> [range]\n"
    "  Reads and displays the used thread table entries. Note that not all\n"
    "  information is displayed; only the most important data.\n"
    "  An optional range argument can be given to only read those threads. The\n"
    "  range is a comma-seperated list of thread ranges. Example ranges:\n"
    "  \"1\", \"1-4,15,7-8\", \"all\"\n";
}

void ThreadTable::Cmd_Read(ostream& out, const vector<string>& arguments) const
{
    static const char* const ThreadStates[] = {
        "", "WAITING", "READY", "ACTIVE", "RUNNING", "SUSPENDED", "UNUSED", "KILLED"
    };
    
    // Read the range
    set<TID> tids;
    if (!arguments.empty()) {
        tids = parse_range<TID>(arguments[0], 0, m_threads.size());
    } else {
        for (TID i = 0; i < m_threads.size(); ++i) {
            if (m_threads[i].state != TST_EMPTY) {
                tids.insert(i);
            }
        }
    }
    
    if (tids.empty())
    {
        out << "No threads selected" << endl;
        return;
    }

    out << "    |         PC         | Fam | Index | Prev | Next | Int. | Flt. | Flags | WR | State" << endl;
    out << "----+--------------------+-----+-------+------+------+------+------+-------+----+----------" << endl;
    for (set<TID>::const_iterator p = tids.begin(); p != tids.end(); ++p)
    {
        out << dec << setw(3) << setfill(' ') << *p << " | ";
        const Thread& thread = m_threads[*p];

        if (thread.state != TST_EMPTY)
        {
            out << setw(18) << setfill(' ') << hex << showbase << thread.pc << " | ";
            out << "F" << setfill('0') << dec << noshowbase << setw(2) << thread.family << " | ";
            out << setw(5) << dec << setfill(' ') << thread.index << " | ";
            if (thread.prevInBlock != INVALID_TID) out << dec << setw(4) << setfill(' ') << thread.prevInBlock; else out << "   -";
            out << " | ";
            if (thread.nextInBlock != INVALID_TID) out << dec << setw(4) << setfill(' ') << thread.nextInBlock; else out << "   -";
            out << " | ";

            for (RegType type = 0; type < NUM_REG_TYPES; ++type)
            {
                if (thread.regs[type].base != INVALID_REG_INDEX)
                    out << setw(4) << setfill('0') << hex << thread.regs[type].base;
                else
                    out << "  - ";
                out << " | ";
            }

            out << (thread.dependencies.prevCleanedUp ? 'P' : '.')
                << (thread.dependencies.killed        ? 'K' : '.')
                << (thread.dependencies.nextKilled    ? 'N' : '.')
                << (thread.isLastThreadInBlock        ? 'L' : '.')
                << "  | "
                << setw(2) << setfill(' ') << thread.dependencies.numPendingWrites
                << " | ";

            out << ThreadStates[thread.state];
        }
        else
        {
            out << "                   |     |       |      |      |      |      |       |    |";
        }
        out << endl;
    }
}

}
