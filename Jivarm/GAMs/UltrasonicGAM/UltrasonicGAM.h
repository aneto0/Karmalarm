/**
 * @file UltrasonicGAM.h
 * @brief Header file for class UltrasonicGAM
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

 * @details This header file contains the declaration of the class UltrasonicGAM
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef ULTRASONICGAM_H_
#define ULTRASONICGAM_H_

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
class UltrasonicGAM: public MARTe::GAM {
public:
    CLASS_REGISTER_DECLARATION()
    /**
     * @brief Constructor. NOOP.
     */
    UltrasonicGAM();

    /**
     * @brief Destructor. NOOP.
     */
    virtual ~UltrasonicGAM();

    /**
     * @brief Verifies that:
     *  - GetNumberOfInputSignals() == 1 &&
     *  - GetNumberOfOutputSignals() == 1 &&
     *  - GetSignalType(InputSignals, 0) == SignedInteger32Bit &&
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
     * Maximum allowed difference between the measurement and its average
     */
    MARTe::float64 maximumDelta;

    /**
     * First order filter alpha (decay of the average)
     */
    MARTe::float64 decayAlpha;

    /**
     * Reset the average to the latest value if -1 is received for N consecutive times
     */
    MARTe::float64 resetAfterNInvalid;

    /**
     * Average measured distance
     */
    MARTe::float64 averageMeasuredDistance;

    /**
     * Counter input signal
     */
    MARTe::int32 *measurementSignal;

    /**
     * Trigger output signal
     */
    MARTe::uint8 *triggerSignal;

    /**
     * Number of consecutive triggers
     */
    MARTe::uint32 consecutiveTriggers;
};

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* ULTRASONICGAM_H_ */

