// -*- c++ -*-
#ifndef PRODCONS_H
#define PRODCONS_H

#include "sim/kernel.h"
#include "sim/buffer.h"
#include "sim/flag.h"

class ExampleConsumer : public Simulator::Object
{
public:
    ExampleConsumer(const std::string& name, Simulator::Object& parent, Simulator::Clock& clock, size_t sz);
    Simulator::Result DoConsume();

    Simulator::Process     p_Consume;
    Simulator::Buffer<int> m_fifo;
};

class ExampleProducer : public Simulator::Object
{
public:
    ExampleProducer(const std::string& name, Simulator::Object& parent, Simulator::Clock& clock, ExampleConsumer& cons);

    Simulator::Result DoProduce();

    int                     m_counter;
    Simulator::Buffer<int>& m_fifo;
    Simulator::Process      p_Produce;
    Simulator::Flag         m_enabled;
};



#endif
