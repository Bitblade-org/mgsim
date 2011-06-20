#ifndef IONMUX_H
#define IONMUX_H

#ifndef PROCESSOR_H
#error This file should be included in Processor.h
#endif

class IOBusInterface;

class IONotificationMultiplexer : public Object
{
private:
    RegisterFile&                   m_regFile;
    Allocator&                   m_allocator;

    std::vector<Register<RegAddr>*> m_writebacks;

    StorageTraceSet GetInterruptRequestTraces() const;
    StorageTraceSet GetNotificationTraces() const;

public:
    std::vector<bool>               m_mask;
    std::vector<SingleFlag*>        m_interrupts;
    std::vector<Buffer<Integer>*>   m_notifications;

private:
    size_t                          m_lastNotified;

public:
    IONotificationMultiplexer(const std::string& name, Object& parent, Clock& clock, RegisterFile& rf, Allocator& alloc, size_t numChannels, Config& config);
    ~IONotificationMultiplexer();

    // sent by device select upon an I/O read from the processor
    bool SetWriteBackAddress(IONotificationChannelID which, const RegAddr& addr);
    bool ConfigureChannel(IONotificationChannelID which, Integer mode);

    // triggered by the IOBusInterface
    bool OnInterruptRequestReceived(IONotificationChannelID which);
    bool OnNotificationReceived(IONotificationChannelID which, Integer tag);

    Process p_IncomingNotifications;
    
    // upon interrupt received
    Result DoReceivedNotifications();
};


#endif
