/**
 * @file UDPReceiver.cpp
 * @brief Source file for class UDPReceiver
 * @date 06/06/2020
 * @author Andre Neto
 *
 * @copyright Copyright 2020 Andre Neto.
 *
 * the Development of Fusion Energy ('Fusion for Energy').
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved
 * by the European Commission - subsequent versions of the EUPL (the "Licence")
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.

 * @details This source file contains the definition of all the methods for
 * the class UDPReceiver (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "AdvancedErrorManagement.h"
#include "UDPReceiver.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
using namespace MARTe;

UDPReceiver::UDPReceiver() :
        MemoryDataSourceI(), EmbeddedServiceMethodBinderI(), executor(*this) {
    stackSize = THREADS_DEFAULT_STACKSIZE * 4u;
    cpuMask = 0xffu;
    udpSocketPort = 0u;
    udpSocketMemory = NULL_PTR(char8 *);
    udpSocketMemoryR = NULL_PTR(char8 *);
    mux.Create();
}

UDPReceiver::~UDPReceiver() {
    if (!executor.Stop()) {
        if (!executor.Stop()) {
            REPORT_ERROR(ErrorManagement::FatalError, "Could not stop SingleThreadService.");
        }
    }
    if (udpSocketMemory != NULL_PTR(char8 *)) {
        delete[] udpSocketMemory;
    }
    if (udpSocketMemoryR != NULL_PTR(char8 *)) {
        delete[] udpSocketMemoryR;
    }
}

bool UDPReceiver::Initialise(StructuredDataI & data) {
    bool ok = MemoryDataSourceI::Initialise(data);
    if (ok) {
        if (!data.Read("CPUs", cpuMask)) {
            REPORT_ERROR(ErrorManagement::Information, "No CPUs defined. Using default = %d", cpuMask);
        }
        if (!data.Read("StackSize", stackSize)) {
            REPORT_ERROR(ErrorManagement::Information, "No StackSize defined. Using default = %d", stackSize);
        }
        if (!data.Read("MulticastAddress", udpMulticastAddress)) {
            REPORT_ERROR(ErrorManagement::Information, "No MulticastAddress defined.");
        }
        udpSocketTimeout = 1000;
        if (!data.Read("SocketTimeout", udpSocketTimeout)) {
            REPORT_ERROR(ErrorManagement::Information, "SocketTimeout not defined. Using default = %d", udpSocketTimeout);
        }
        executor.SetStackSize(stackSize);
        executor.SetCPUMask(cpuMask);
    }
    if (ok) {
        ok = data.Read("Port", udpSocketPort);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Port not specified");
        }
    }
    return ok;
}

bool UDPReceiver::SetConfiguredDatabase(StructuredDataI & data) {
    bool ok = MemoryDataSourceI::SetConfiguredDatabase(data);
    //Check the signal index of the timing signal.
    uint32 nOfSignals = GetNumberOfSignals();
    if (ok) {
        ok = (nOfSignals > 0u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "At least one signal shall be defined");
        }
    }
    if (ok) {
        //Do not allow samples
        uint32 functionNumberOfSignals = 0u;
        uint32 n;
        if (GetFunctionNumberOfSignals(InputSignals, 0u, functionNumberOfSignals)) {
            for (n = 0u; (n < functionNumberOfSignals) && (ok); n++) {
                uint32 nSamples;
                ok = GetFunctionSignalSamples(InputSignals, 0u, n, nSamples);
                if (ok) {
                    ok = (nSamples == 1u);
                }
                if (!ok) {
                    REPORT_ERROR(ErrorManagement::ParametersError, "The number of samples shall be exactly 1");
                }
            }
        }
    }
    uint32 nOfFunctions = GetNumberOfFunctions();
    if (ok) {
        ok = (nOfFunctions == 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "At most one function shall interact with this DataSourceI");
        }
    }
    if (ok) {
        ok = udpSocket.Open();
        if (!ok) {
            REPORT_ERROR(ErrorManagement::FatalError, "Could not open socket");
        }
    }
    if (ok) {
        ok = udpSocket.Listen(udpSocketPort);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::FatalError, "Could not listen in port %d", udpSocketPort);
        }
    }
    if (ok) {
        if (udpMulticastAddress.Size() > 0u) {
            REPORT_ERROR(ErrorManagement::Information, "Setting multicast address %s", udpMulticastAddress.Buffer());
            struct ip_mreq mreq;
            mreq.imr_multiaddr.s_addr = inet_addr(udpMulticastAddress.Buffer());         
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            ok = (setsockopt(udpSocket.GetReadHandle(), IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) >= 0);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::Information, "Failed setting multicast address %s", udpMulticastAddress.Buffer());
            }
        }
    }
    if (ok) {
        REPORT_ERROR(ErrorManagement::Information, "Socket listening in port %d", udpSocketPort);
    }

    return ok;
}

bool UDPReceiver::AllocateMemory() {
    bool ok = MemoryDataSourceI::AllocateMemory();
    if (ok) {
        udpSocketMemory = new char8[totalMemorySize]; 
        udpSocketMemoryR = new char8[totalMemorySize]; 
        MemoryOperationsHelper::Set(udpSocketMemory, 0, totalMemorySize);
        MemoryOperationsHelper::Set(udpSocketMemoryR, 0, totalMemorySize);
    }
    return ok;
}

const char8* UDPReceiver::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
    const char8* brokerName = "";
    if (direction == InputSignals) {
        brokerName = "MemoryMapSynchronisedInputBroker";
    }
    return brokerName;
}

bool UDPReceiver::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    bool ok = true;
    if (executor.GetStatus() == EmbeddedThreadI::OffState) {
        ok = executor.Start();
    }

    return ok;
}

ErrorManagement::ErrorType UDPReceiver::Execute(ExecutionInfo& info) {
    ErrorManagement::ErrorType err = ErrorManagement::NoError;
    if (info.GetStage() != ExecutionInfo::BadTerminationStage) {
        uint32 readSize = totalMemorySize;
        if (udpSocket.Read(udpSocketMemoryR, readSize, udpSocketTimeout)) {
            if(mux.FastTryLock()) {
                (void) MemoryOperationsHelper::Copy(reinterpret_cast<void * const>(udpSocketMemory), reinterpret_cast<const void * const>(udpSocketMemoryR), totalMemorySize);
            }
            mux.FastUnLock();
        }
    }
    return err;
}

uint32 UDPReceiver::GetStackSize() const {
    return stackSize;
}

uint32 UDPReceiver::GetCPUMask() const {
    return cpuMask;
}

bool UDPReceiver::Synchronise() {
   bool ret = true;
   if (mux.FastTryLock()) {
       ret = MemoryOperationsHelper::Copy(reinterpret_cast<void * const>(memory), reinterpret_cast<const void * const>(udpSocketMemory), totalMemorySize);
   }
   mux.FastUnLock();
   return ret;
}

CLASS_REGISTER(UDPReceiver, "1.0")


