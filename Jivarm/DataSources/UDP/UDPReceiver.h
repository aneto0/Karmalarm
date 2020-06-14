/**
 * @file UDPReceiver.h
 * @brief Header file for class UDPReceiver
 * @date 06/06/2020
 * @author Andre Neto
 *
 * @copyright Copyright 2020 Andre Neto.
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved
 * by the European Commission - subsequent versions of the EUPL (the "Licence")
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.

 * @details This header file contains the declaration of the class UDPReceiver
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef UDPRECEIVER_H_
#define UDPRECEIVER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "EmbeddedServiceMethodBinderI.h"
#include "EventSem.h"
#include "FastPollingMutexSem.h"
#include "MemoryDataSourceI.h"
#include "SingleThreadService.h"
#include "UDPSocket.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

/**
 * @brief A DataSource which TODO
 * 
 * <pre>
 * +UDPReceiver_1 = {
 *     Class = UDP::Receiver
 *     StackSize = 1048576 //Optional the EmbeddedThread stack size. Default value is THREADS_DEFAULT_STACKSIZE * 4u
 *     CpuMask = 0xff //Optional the affinity of the EmbeddedThread which actually performs the UDP listening
 *     Port = 23456 //Port where to listen
 *     SocketTimeout = 1000 //Timeout on the socket read (default 1000 ms)
 *
 *     Signals = {
 *         //TODO 
 *         ...
 *     }
 * }
 *
 * </pre>
 */
class UDPReceiver: public MARTe::MemoryDataSourceI, public MARTe::EmbeddedServiceMethodBinderI {
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Default constructor. NOOP.
     */
    UDPReceiver();

    /**
     * @brief Destructor. NOOP.
     */
    virtual ~UDPReceiver();

    /**
     * @brief See DataSourceI::GetNumberOfMemoryBuffers.
     * @details Only InputSignals are supported.
     * @return MemoryMapInputBroker.
     */
    virtual const MARTe::char8 *GetBrokerName(MARTe::StructuredDataI &data,
            const MARTe::SignalDirection direction);

    /**
     * @brief See DataSourceI::PrepareNextState. NOOP.
     * @return true.
     */
    virtual bool PrepareNextState(const MARTe::char8 * const currentStateName,
            const MARTe::char8 * const nextStateName);

    /**
     * @brief Loads and verifies the configuration parameters detailed in the class description.
     * @return true if all the mandatory parameters are correctly specified and if the specified optional parameters have valid values.
     */
    virtual bool Initialise(MARTe::StructuredDataI & data);

    /**
     * @brief TODO 
     */
    virtual bool SetConfiguredDatabase(MARTe::StructuredDataI & data);

    /**
     * @brief TODO 
     * @details see MemoryDataSourceI::AllocateMemory
     * @return TODO 
     */
    virtual bool AllocateMemory();

    /**
     * @brief Gets the affinity of the thread which is going to be used to asynchronously read data from the pvac::MonitorSync.
     * @return the the affinity of the thread which is going to be used to asynchronously read data from the pvac::MonitorSync.
     */
    MARTe::uint32 GetCPUMask() const;

    /**
     * @brief Gets the stack size of the thread which is going to be used to asynchronously read data from the pvac::MonitorSync.
     * @return the stack size of the thread which is going to be used to asynchronously read data from the pvac::MonitorSync.
     */
    MARTe::uint32 GetStackSize() const;

    /**
     * @brief Provides the context to execute pvac::MonitorSync::wait.
     * @return ErrorManagement::NoError if the pvac::MonitorSync::wait and pvac::MonitorSync::poll calls return without any error.
     */
    virtual MARTe::ErrorManagement::ErrorType Execute(MARTe::ExecutionInfo & info);

    /**
     * @brief See DataSourceI::Synchronise.
     * @return TODO.
     */
    virtual bool Synchronise();

private:

    /**
     * UDP Socket
     */
    MARTe::UDPSocket udpSocket;

    /**
     * UDP port
     */
    MARTe::uint32 udpSocketPort;

    /**
     * UDP multicast address
     */
    MARTe::StreamString udpMulticastAddress;

    /**
     * UDP socket memories
     */
    MARTe::char8 *udpSocketMemory;
    MARTe::char8 *udpSocketMemoryR;

    /**
     * Size of the udp socket memory.
     */
    MARTe::uint32 udpSocketMemorySize;

    /**
     * Socket timeout in milliseconds
     */
    MARTe::uint32 udpSocketTimeout;

    /**
     * The CPU mask for the executor
     */
    MARTe::uint32 cpuMask;

    /**
     * The stack size
     */
    MARTe::uint32 stackSize;

    /**
     * The EmbeddedThread where the monitor is executed (one thread per record).
     */
    MARTe::SingleThreadService executor;

    /**
     * Synchronise the memory copy operation.
     */
    MARTe::FastPollingMutexSem mux;

};

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* UDPRECEIVER_H_ */

