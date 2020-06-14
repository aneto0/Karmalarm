/**
 * @file AlarmTriggerGAM.h
 * @brief Header file for class AlarmTriggerGAM
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

 * @details This header file contains the declaration of the class AlarmTriggerGAM
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef ALARMTRIGGER_H_
#define ALARMTRIGGER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "GAM.h"
#include "Message.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
/**
 * @brief TODO
 */
class AlarmTriggerGAM: public MARTe::GAM {
public:
    CLASS_REGISTER_DECLARATION()
    /**
     * @brief Constructor. NOOP.
     */
    AlarmTriggerGAM();

    /**
     * @brief Destructor. NOOP.
     */
    virtual ~AlarmTriggerGAM();

    /**
     * @brief TODO
     * @return true if the conditions above are met.
     */
    virtual bool Setup();

    /**
     * @brief The configuration data detailed in the class description
     * @return true if all the compulsory parameters are set.
     */
    virtual bool Initialise(MARTe::StructuredDataI & data);

    /**
     * @brief TODO 
     * @return true.
     */
    virtual bool Execute();

private:
    /**
     * Alarm hysteresis
     */
    MARTe::uint32 alarmHysteresis;

    /**
     * Number of alarm input signals
     */
    MARTe::uint32 numberOfAlarmInputSignals;

    /**
     * Alarm input signals
     */
    MARTe::uint8 **alarmInputSignals;

    /**
     * Current alarm status of every signal
     */
    bool *alarmStatus;

    /**
     * Consecutive alarm counters
     */
    MARTe::uint32 *consecutiveAlarmCounter;

    /**
     * Message to send when an alarm is triggered
     */
    MARTe::ReferenceT<MARTe::Message> messageTrigger;
};

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* ALARMTRIGGER_H_ */

