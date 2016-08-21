#include "ActionInterface.h"
#include <cctype>
#include <sstream>
#include "DRISC.h"

namespace Simulator
{

namespace drisc
{

/*
 * MGSim control interface.
 *   address bits:
 *                     x A A -> status/action
 *                     0 - - -> don't print status
 *                     1 - - -> print encoded status before action with cycle/cpu/family/thread identity
 *                     - 0 0 -> continue
 *                     - 0 1 -> interrupt
 *                     - 1 0 -> abort
 *                     - 1 1 -> exit
 *    maximum address: 1 1 1
 */

size_t ActionInterface::GetSize() const { return 16 * sizeof(Integer);  }


Result ActionInterface::Read (MemAddr address, void* data, MemSize size, LFID /* fid */ , TID /* tid */, const RegAddr& /* writeback */)
{
    if (address == 0x50)
    {
    	if(size != sizeof(Integer)){
            throw exceptf<>(*this, "Invalid action control: %#016llx", (unsigned long long)address);
    	}

    	Object& dparent = *GetParent();
    	auto& localCpu = static_cast<DRISC&>(dparent);
    	uint64_t deltaC = localCpu.GetCycleNo() - lastTimerCycleCount;
    	lastTimerCycleCount = localCpu.GetCycleNo();

    	if(timerValue > deltaC){
    		timerValue -= deltaC;
    		return FAILED;
    	}

		deltaC -= timerValue;
		timerValue = 0;
	    SerializeRegister(RT_INTEGER, deltaC, data, sizeof(Integer));

    	if(deltaC != 0){
    		std::cout << "Timer overshoot: " << deltaC << "\n";
    	}

	    return SUCCESS;

    }

    UNREACHABLE
}

void ActionInterface::Cmd_Info(std::ostream& out, const std::vector<std::string>&) const{
	Object& dparent = *GetParent();
	auto& localCpu = static_cast<DRISC&>(dparent);
	uint64_t deltaC = localCpu.GetCycleNo() - lastCycleCount;

	for(int i=0; i<8; i++){
		uint64_t count = stateCycleCounter[i];
		if(currentState == i){
			count += deltaC;
		}
		out << "STATE " << i << ": " << count << " cycles\n";
	}
}

void ActionInterface::stateUpdate(const unsigned char newState){
	Object& dparent = *GetParent();
	auto& localCpu = static_cast<DRISC&>(dparent);
	uint64_t deltaC = localCpu.GetCycleNo() - lastCycleCount;
	lastCycleCount = localCpu.GetCycleNo();

//	std::cout << "cycles[" << std::dec << unsigned(currentState) << "] = " << stateCycleCounter[currentState] << " + " << deltaC << " = ";

	stateCycleCounter[currentState] += deltaC;
//	std::cout << stateCycleCounter[currentState] << "; currCycle = " << unsigned(newState) << std::endl;
	currentState = newState;
}

Result ActionInterface::Write(MemAddr address, const void *data, MemSize size, LFID fid, TID tid)
{
	COMMIT{ std::cout << "Write to actioninterface with address " << std::hex << address << std::endl; }
    if (address % sizeof(Integer) != 0 && (address & ~0x4F) != 0)
    {
        throw exceptf<>(*this, "Invalid action control: %#016llx", (unsigned long long)address);
    }

    Integer value = UnserializeRegister(RT_INTEGER, data, size);

    if (address & 0x40){
		Object& dparent = *GetParent();
		auto& localCpu = static_cast<DRISC&>(dparent);

		if(address & 0x10){
			COMMIT{
				lastTimerCycleCount = localCpu.GetCycleNo();
				timerValue = value;
			}
			return SUCCESS;
		}else if(address & 8){
    		COMMIT{
				unsigned char state = address % 8;
				stateUpdate(state);
    		}
    		return SUCCESS;
    	}else{
			Integer cpuId = value;

			auto& remoteCpu = localCpu.getGrid()[cpuId];
			if(remoteCpu->GetName() != std::string("cpu") + std::to_string(cpuId)){
				throw exceptf<>(*this, "Could not find cpu with id %u", (unsigned int)cpuId);
			}

			COMMIT{
				if(address & 1){
					remoteCpu->getMMU().getDTlb().invalidate();
				}

				if(address & 2){
					if(!remoteCpu->GetDCache().invalidate()){
						return FAILED;
					}
				}
			}
			return SUCCESS;
    	}
    }

    address /= sizeof(Integer);

    if (address & 4)
    {
        std::ostringstream msg;
        if ((address & 3) < 3)
        {
            // Anything but EXIT
            Integer imsg = value;
            for (unsigned i = 0; i < sizeof(imsg); ++i, imsg >>= 8)
            {
                char byte = imsg & 0xff;
                if (std::isprint(byte)) msg << byte;
            }
        }
        else
        {
            // EXIT
            msg << (value & 0xff);
        }

        const char *actiontype = 0;
        switch(address & 3)
        {
        case 0: actiontype = "CONTINUE"; break;
        case 1: actiontype = "INTERRUPT"; break;
        case 2: actiontype = "ABORT"; break;
        case 3: actiontype = "EXIT"; break;
        }

        DebugProgWrite("F%u/T%u %s: %s",
                       (unsigned)fid, (unsigned)tid, actiontype, msg.str().c_str());
    }

    COMMIT{
        switch(address & 3)
        {
        case 0: /* nothing, continue */ break;
        case 1:
            GetKernel()->Stop();
            break;
        case 2:
            throw ProgramTerminationException(*this, "Program requested simulator to abort.", 0, true);
        case 3:
        {
            int code = value & 0xff;
            std::ostringstream emsg;
            emsg << "Program requested simulator to exit with code " << code << ".";
            throw ProgramTerminationException(*this, emsg.str(), code, false);
        }
        }
    }

    return SUCCESS;
}

ActionInterface::ActionInterface(const std::string& name, Object& parent)
    : MMIOComponent(name, parent),
	  currentState(0),
	  lastCycleCount(0),
	  stateCycleCounter(),
	  timerValue(0),
	  lastTimerCycleCount(0)
{
}

}
}
