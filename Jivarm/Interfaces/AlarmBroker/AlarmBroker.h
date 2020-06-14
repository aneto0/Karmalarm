/**
 * @file AlarmBroker.h
 * @brief Header file for class AlarmBroker
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

 * @details This header file contains the declaration of the class AlarmBroker
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef ALARMBROKER_H_
#define ALARMBROKER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "MessageI.h"
#include "TCPSocket.h"
#include "StatefulI.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
/**
 * @brief TODO
 */
class AlarmBroker: public MARTe::Object, public MARTe::MessageI {
public:
    CLASS_REGISTER_DECLARATION()
    /**
     * @brief Constructor. NOOP.
     */
    AlarmBroker();

    /**
     * @brief Destructor. NOOP.
     */
    virtual ~AlarmBroker();

    /**
     * @brief TODO 
     */
    MARTe::ErrorManagement::ErrorType Trigger();

    /**
     * @brief TODO 
     */
    MARTe::ErrorManagement::ErrorType Start();

    /**
     * @brief TODO 
     */
    virtual bool Initialise(MARTe::StructuredDataI & data);

private:

    /**
     * The TCP socket to serialize the rpc call
     */
    MARTe::TCPSocket socket;

    /**
     * The target TCP socket address
     */
    MARTe::StreamString tcpAddress;

    /**
     * The target TCP socket port
     */
    MARTe::uint16 tcpPort;

    /**
     * Number of monitor objects
     */
    MARTe::uint32 numberOfObjects;

    /**
     * Monitor objects
     */
    MARTe::ReferenceContainer monitorObjects;
    MARTe::StreamString *monitorObjectsNames;
};

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* ALARMBROKER_H_ */

