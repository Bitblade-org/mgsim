// -*- c++ -*-
#ifndef ACTIONINTERFACE_H
#define ACTIONINTERFACE_H

#include "old/IOMatchUnit.h"

namespace Simulator
{
namespace drisc
{

class ActionInterface : public MMIOComponent, public Inspect::Interface<Inspect::Info>
{
public:
    ActionInterface(const std::string& name, Object& parent);

    size_t GetSize() const;

    Result Read (MemAddr address, void* data, MemSize size, LFID fid, TID tid, const RegAddr& writeback);
    Result Write(MemAddr address, const void* data, MemSize size, LFID fid, TID tid);
    void Cmd_Info(std::ostream& out, const std::vector<std::string>& arguments) const override;
    void stateUpdate(const unsigned char newState);

    unsigned char currentState;
    uint64_t lastCycleCount;
    uint64_t stateCycleCounter[8];
    uint64_t timerValue;
    uint64_t lastTimerCycleCount;
};

}
}

#endif
