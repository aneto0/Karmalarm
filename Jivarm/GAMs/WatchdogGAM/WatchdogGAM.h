/**
 * @file WatchdogGAM.h
 * @brief Header file for class WatchdogGAM
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

 * @details This header file contains the declaration of the class WatchdogGAM
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef WATCHDOGGAM_H_
#define WATCHDOGGAM_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "GAM.h"
#include "StatefulI.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
/**
 * @brief TODO
 */
class WatchdogGAM: public MARTe::GAM {
public:
    CLASS_REGISTER_DECLARATION()
    /**
     * @brief Constructor. NOOP.
     */
    WatchdogGAM();

    /**
     * @brief Destructor. NOOP.
     */
    virtual ~WatchdogGAM();

    /**
     * @brief Verifies that:
     *  - GetNumberOfInputSignals() == 1 &&
     *  - GetNumberOfOutputSignals() == 1 &&
     *  - GetSignalType(InputSignals, 0) == UnsignedInteger32Bit &&
     *  - GetSignalType(OutputSignals, 0) == UnsignedInteger8Bit
     *  @return true if the conditions above are met.
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
     * Value of the last counter
     */
    MARTe::uint32 lastCounterValue;

    /**
     * Number of cycles without any update
     */
    MARTe::uint32 lateCycles;

    /**
     * Maximum number of updates allowed without the counter being update
     */
    MARTe::uint32 maxLatePackets;

    /**
     * Counter input signal
     */
    MARTe::uint32 *counterSignal;

    /**
     * Trigger output signal
     */
    MARTe::uint8 *triggerSignal;
};

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* WATCHDOGGAM_H_ */

