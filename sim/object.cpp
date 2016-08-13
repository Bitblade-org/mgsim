#include <cstdio>
#include <iomanip>
#include <sstream>
#include "sim/kernel.h"

namespace Simulator
{

    Object::Object(const std::string& name, Kernel&
#ifndef STATIC_KERNEL
                   k
#endif
        )
        : m_parent(NULL),
          m_name(name),
#ifndef STATIC_KERNEL
          m_kernel(k),
#endif
          m_children()
    {
    }

    Object::Object(const std::string& name, Object& parent)
        : m_parent(&parent),
          m_name(parent.GetName().empty() ? name : (parent.GetName() + '.' + name)),
#ifndef STATIC_KERNEL
          m_kernel(*parent.GetKernel()),
#endif
          m_children()
    {
        // Add ourself to the parent's children array
        parent.m_children.push_back(this);
    }

    Object::~Object()
    {
        if (m_parent != NULL)
        {
            // Remove ourself from the parent's children array
            for (auto p = m_parent->m_children.begin(); p != m_parent->m_children.end(); ++p)
            {
                if (*p == this)
                {
                    m_parent->m_children.erase(p);
                    break;
                }
            }
        }
    }

    void Object::OutputWrite_(const char* msg, ...) const
    {
        va_list args;

        fprintf(stderr, "[%08lld:%s]\t\to ", (unsigned long long)GetKernel()->GetCycleNo(), GetName().c_str());
        va_start(args, msg);
        vfprintf(stderr, msg, args);
        va_end(args);
        fputc('\n', stderr);
    }

    void Object::DeadlockWrite_(const char* msg, ...) const
    {
        va_list args;

        fprintf(stderr, "[%08lld:%s]\t(%s)\td ", (unsigned long long)GetKernel()->GetCycleNo(), GetName().c_str(),
                GetKernel()->GetActiveProcess()->GetName().c_str());
        va_start(args, msg);
        vfprintf(stderr, msg, args);
        va_end(args);
        fputc('\n', stderr);
    }

    void Object::DebugSimWrite_(const char* msg, ...) const
    {
        va_list args;
		static std::stringstream cycleBuffer;
		static unsigned long long cycle;
		static size_t lastHash;
		static unsigned int repeatCount;
		static unsigned int nextRepeatTrigger = 100;
        static unsigned int col0_minw = 25;
        static unsigned int col1_minw = 25;

		if(GetKernel()->GetCycleNo() != cycle){
			std::string contents = cycleBuffer.str();
			std::hash<std::string> hash_fn;
			size_t hash = hash_fn(contents);

			if(hash == lastHash){
				if(repeatCount < nextRepeatTrigger || GetKernel()->GetCycleNo() - 100 > cycle) {
					repeatCount++;
				}else{
					std::cerr << "Last cycles debug output repeated " << repeatCount << " times..." << std::endl;
					repeatCount = 0;
					nextRepeatTrigger = nextRepeatTrigger * 2;
				}
			}else{
				if(repeatCount != 0){
					std::cerr << "Last cycles debug output repeated " << repeatCount << " times..." << std::endl;
					repeatCount = 0;
					nextRepeatTrigger = 100;
				}

				//MLDTODO split contents string
				char buffer[256];
				while(cycleBuffer.getline(buffer, 256)){
					std::cerr << "[" << std::setw(8) << cycle << buffer << std::endl;
				}
			}

			lastHash = hash;
			cycleBuffer.str("");
			cycleBuffer.clear();
			cycle = GetKernel()->GetCycleNo();
		}

		if(GetName().size() >= col0_minw){
			col0_minw = GetName().size() + 4;
		}

		cycleBuffer << ":" << GetName() << ']' << std::setw(col0_minw - GetName().size()) << " ";

        const Process *p = GetKernel()->GetActiveProcess();
		if (p){
			if(p->GetName().size() >= col1_minw){
				col1_minw = p->GetName().size() + 4;
			}
			cycleBuffer << "(" << p->GetName() << ")" << std::setw(col1_minw - p->GetName().size()) << " ";
		}

        va_start(args, msg);
		char buffer[256];
		vsnprintf(buffer, 256, msg, args);
		cycleBuffer << buffer << std::endl;
        va_end(args);
    }

}
