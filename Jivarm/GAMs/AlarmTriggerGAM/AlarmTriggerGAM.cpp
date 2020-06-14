/**
 * @file AlarmTriggerGAM.cpp
 * @brief Source file for class AlarmTriggerGAM
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
 * the class AlarmTriggerGAM (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "AlarmTriggerGAM.h"
#include "MessageI.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

AlarmTriggerGAM::AlarmTriggerGAM() :
        MARTe::GAM() {
    using namespace MARTe;
    alarmHysteresis = 0u;
    numberOfAlarmInputSignals = 0u;
    alarmInputSignals = NULL_PTR(uint8 **);
    alarmStatus = NULL_PTR(bool *);
    consecutiveAlarmCounter = NULL_PTR(uint32 *);

}

AlarmTriggerGAM::~AlarmTriggerGAM() {
    using namespace MARTe;
    if (alarmInputSignals != NULL_PTR(uint8 **)) {
        delete [] alarmInputSignals;
    }
    if (alarmStatus != NULL_PTR(bool *)) {
        delete [] alarmStatus;
    }
    if (consecutiveAlarmCounter != NULL_PTR(uint32 *)) {
        delete [] consecutiveAlarmCounter;
    }
}

bool AlarmTriggerGAM::Setup() {
    using namespace MARTe;
    numberOfAlarmInputSignals = GetNumberOfInputSignals();
    bool ok = (numberOfAlarmInputSignals > 0u);
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "GetNumberOfInputSignals() must be > 0u");
    }
    if (ok) {
        alarmInputSignals = new uint8*[numberOfAlarmInputSignals];
        alarmStatus = new bool[numberOfAlarmInputSignals];
        consecutiveAlarmCounter = new uint32[numberOfAlarmInputSignals];
    }
    if (ok) {
        uint32 i;
        for (i = 0u; i<numberOfAlarmInputSignals; i++) {
            alarmInputSignals[i] = reinterpret_cast<uint8 *>(GetInputSignalMemory(i));
            alarmStatus[i] = false;
            consecutiveAlarmCounter[i] = 0u;
        }
    }
    return ok;
}

bool AlarmTriggerGAM::Initialise(MARTe::StructuredDataI & data) {
    using namespace MARTe;
    bool ok = GAM::Initialise(data);
    if (ok) {
        ok = data.Read("AlarmHysteresis", alarmHysteresis);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "AlarmHysteresis must be specified");
        }
    }
    if (ok) {
        ok = (Size() == 1);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "At least one child message should had been added");
        }
    }
    if (ok) {
        messageTrigger = Get(0);
        ok = messageTrigger.IsValid();
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Element in position 0 is not a message");
        }
    }
    return ok;
}

bool AlarmTriggerGAM::Execute() {
    using namespace MARTe;
    uint32 i;
    for (i = 0u; i<numberOfAlarmInputSignals; i++) {
        uint8 alarmSignal = *(alarmInputSignals[i]);
        if (alarmSignal > 0u) {
            if (!alarmStatus[i]) {
                consecutiveAlarmCounter[i]++;
                if (consecutiveAlarmCounter[i] == alarmHysteresis) {
                    alarmStatus[i] = true;
                    StreamString signalName;
                    (void) GetSignalName(InputSignals, i, signalName);
                    REPORT_ERROR(ErrorManagement::Warning, "Alarm triggered for signal %s (idx: %d)", signalName.Buffer(), i);
                    ErrorManagement::ErrorType msgError = MessageI::SendMessage(messageTrigger, this);
                    if (!msgError.ErrorsCleared()) {
                        REPORT_ERROR(ErrorManagement::ParametersError, "Error while sending message to destination %s with function %s", messageTrigger->GetDestination().GetList(), messageTrigger->GetFunction().GetList());
                    }

                }
            }
        }
        else {
            if (alarmStatus[i]) {
                consecutiveAlarmCounter[i]--;
                if (consecutiveAlarmCounter[i] == 0) {
                    alarmStatus[i] = false;
                    StreamString signalName;
                    (void) GetSignalName(InputSignals, i, signalName);
                    REPORT_ERROR(ErrorManagement::Debug, "Alarm reset for signal %s (idx: %d)", signalName.Buffer(), i);

                }
            }
        }
    }
    return true;
}

CLASS_REGISTER(AlarmTriggerGAM, "1.0")

