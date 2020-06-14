/**
 * @file UltrasonicGAM.cpp
 * @brief Source file for class UltrasonicGAM
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
 * the class UltrasonicGAM (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "UltrasonicGAM.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

UltrasonicGAM::UltrasonicGAM() :
        MARTe::GAM() {
    using namespace MARTe;
    maximumDelta = 0.F;
    decayAlpha = 0.F;
    resetAfterNInvalid = 0;
    consecutiveTriggers = 0u;
    averageMeasuredDistance = 0.F;
    measurementSignal = NULL_PTR(int32 *);
    triggerSignal = NULL_PTR(uint8 *);
}

UltrasonicGAM::~UltrasonicGAM() {
    using namespace MARTe;

}

bool UltrasonicGAM::Setup() {
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
        ok = (GetSignalType(InputSignals, 0u) == SignedInteger32Bit);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "GetSignalType(InputSignals, 0u) != SignedInteger32Bit");
        }
    }
    if (ok) {
        ok = (GetSignalType(OutputSignals, 0u) == UnsignedInteger8Bit);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "GetSignalType(OutputSignals, 0u) != UnsignedInteger8Bit");
        }
    }
    if (ok) {
        measurementSignal = static_cast<int32 *>(GetInputSignalMemory(0u));
        triggerSignal = static_cast<uint8 *>(GetOutputSignalMemory(0u));
    } 
    return ok;
}

bool UltrasonicGAM::Initialise(MARTe::StructuredDataI & data) {
    using namespace MARTe;
    bool ok = GAM::Initialise(data);
    if (ok) {
        ok = data.Read("MaximumDelta", maximumDelta);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "MaximumDelta must be specified");
        }
    }
    if (ok) {
        ok = data.Read("DecayAlpha", decayAlpha);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "DecayAlpha must be specified");
        }
    }
    if (ok) {
        ok = data.Read("ResetAfterNInvalid", resetAfterNInvalid);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "ResetAfterNInvalid must be specified");
        }
    }
    if (ok) {
        ok = (resetAfterNInvalid > 0);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "ResetAfterNInvalid must be > 0");
        }

    }
    if (ok) {
        consecutiveTriggers = resetAfterNInvalid;
    }
    return ok;
}

bool UltrasonicGAM::Execute() {
    using namespace MARTe;
    float64 measurementDelta = 999999999;
    int32 measuredDistanceI = *measurementSignal;
    int32 measuredDistance = static_cast<float64>(measuredDistanceI);
    if (measuredDistanceI != -1) {
        //Reset
        if (consecutiveTriggers == resetAfterNInvalid) {
            averageMeasuredDistance = measuredDistance;
            consecutiveTriggers = 0u;
            REPORT_ERROR(ErrorManagement::Warning, "Ultrasonic average sensor measurement reset %f", averageMeasuredDistance);
        }
        //Compute average with filter
        else {
            averageMeasuredDistance = averageMeasuredDistance * (decayAlpha);
            averageMeasuredDistance += measuredDistance * (1 - decayAlpha);
            measurementDelta = abs(averageMeasuredDistance - measuredDistance);
        }
    }
    if (measurementDelta > maximumDelta) {
        *triggerSignal = 1u;
        if (consecutiveTriggers <= resetAfterNInvalid) {
            REPORT_ERROR(ErrorManagement::Warning, "Ultrasonic sensor triggered %f > abs(avg: %f - last: %d)", measurementDelta, averageMeasuredDistance, measuredDistance);
            consecutiveTriggers++;
        }
    }
    else {
        *triggerSignal = 0u;
        consecutiveTriggers = 0u;
    }
    return true;
}

CLASS_REGISTER(UltrasonicGAM, "1.0")

