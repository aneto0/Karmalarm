/**
 * @file PowerMonitorGAM.cpp
 * @brief Source file for class PowerMonitorGAM
 * @date 11/06/2020
 * @author Andre Neto
 *
 * @copyright Copyright 2020 Andre Neto.
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
 * the class PowerMonitorGAM (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "PowerMonitorGAM.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

PowerMonitorGAM::PowerMonitorGAM() :
        MARTe::GAM() {
    using namespace MARTe;
    adcToVolts = 0.F;
    voltageThreshold = 0.5F;
    adc0 = NULL_PTR(uint32 *);
    triggerSignal = NULL_PTR(uint8 *);
}

PowerMonitorGAM::~PowerMonitorGAM() {
    using namespace MARTe;

}

bool PowerMonitorGAM::Setup() {
    using namespace MARTe;
    bool ok = (GetNumberOfInputSignals() == 1u);
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "GetNumberOfInputSignals() != 1u");
    }
    if (ok) {
        ok = (GetNumberOfOutputSignals() == 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "GetNumberOfOutputSignals() != 1u");
        }
    }
    if (ok) {
        ok = (GetSignalType(InputSignals, 0u) == UnsignedInteger32Bit);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "GetSignalType(InputSignals, 0u) != UnsignedInteger32Bit");
        }
    }
    if (ok) {
        ok = (GetSignalType(OutputSignals, 0u) == UnsignedInteger8Bit);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "GetSignalType(OutputSignals, 0u) != UnsignedInteger8Bit");
        }
    }
    if (ok) {
        adc0 = static_cast<uint32 *>(GetInputSignalMemory(0u));
        triggerSignal = static_cast<uint8 *>(GetOutputSignalMemory(0u));
    } 
    return ok;
}

bool PowerMonitorGAM::Initialise(MARTe::StructuredDataI & data) {
    using namespace MARTe;
    bool ok = GAM::Initialise(data);
    if (ok) {
        ok = data.Read("ADCToVolts", adcToVolts);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "ADCToVolts must be specified");
        }
    }
    if (ok) {
        ok = data.Read("VoltageThreshold", voltageThreshold);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "VoltageThreshold must be specified");
        }
    }
    return ok;
}

bool PowerMonitorGAM::Execute() {
    using namespace MARTe;
    float32 adc0F = *adc0 * adcToVolts;
    if (adc0F < voltageThreshold) {
        if (*triggerSignal == 0) {
            REPORT_ERROR(ErrorManagement::Warning, "PowerMonitor sensor triggered %f < %f)", adc0F, voltageThreshold);
        }
        *triggerSignal = 1u;
    }
    else {
        *triggerSignal = 0u;
    }
    return true;
}

CLASS_REGISTER(PowerMonitorGAM, "1.0")

