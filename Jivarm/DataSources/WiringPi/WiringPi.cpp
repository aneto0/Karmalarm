/**
 * @file WiringPi.cpp
 * @brief Source file for class WiringPi
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
 * the class WiringPi (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include <wiringPi.h>
#include "WiringPi.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
using namespace MARTe;

WiringPi::WiringPi() :
        MemoryDataSourceI() {
    adcPin = 1u;
}

WiringPi::~WiringPi() {
}

bool WiringPi::Initialise(StructuredDataI & data) {
    bool ok = MemoryDataSourceI::Initialise(data);
    if (ok) {
        ok = data.Read("ADCPin", adcPin);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "ADCPin not specified");
        }
    }
    return ok;
}

bool WiringPi::SetConfiguredDatabase(StructuredDataI & data) {
    bool ok = MemoryDataSourceI::SetConfiguredDatabase(data);
    //Check the signal index of the timing signal.
    uint32 nOfSignals = GetNumberOfSignals();
    if (ok) {
        ok = (nOfSignals == 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Only one signal shall be defined");
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
        //Always return 0... see API
        (void) wiringPiSetup();
        REPORT_ERROR(ErrorManagement::Information, "Called wiringPiSetup");
    }

    return ok;
}

const char8* WiringPi::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
    const char8* brokerName = "";
    if (direction == InputSignals) {
        brokerName = "MemoryMapSynchronisedInputBroker";
    }
    return brokerName;
}

bool WiringPi::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    return true;
}

bool WiringPi::Synchronise() {
    using namespace MARTe;
    bool ret = true;
    *(reinterpret_cast<uint32 *>(memory)) = analogRead(adcPin);
    return ret;
}

CLASS_REGISTER(WiringPi, "1.0")


