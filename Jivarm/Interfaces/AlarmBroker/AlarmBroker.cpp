/**
 * @file AlarmBroker.cpp
 * @brief Source file for class AlarmBroker
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
 * the class AlarmBroker (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "AlarmBroker.h"
#include "CLASSMETHODREGISTER.h"
#include "ConfigurationDatabase.h"
#include "JsonPrinter.h"
#include "ObjectRegistryDatabase.h"
#include "RegisteredMethodsMessageFilter.h"
#include "StreamStructuredData.h"
#include "TCPSocket.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

AlarmBroker::AlarmBroker() :
        MARTe::Object(), MARTe::MessageI() {
    using namespace MARTe;
    ReferenceT<RegisteredMethodsMessageFilter> filter(GlobalObjectsDatabase::Instance()->GetStandardHeap());
    filter->SetDestination(this);
    MessageI::InstallMessageFilter(filter);
    numberOfObjects = 0u;
    monitorObjectsNames = NULL_PTR(StreamString *);
}

AlarmBroker::~AlarmBroker() {
    using namespace MARTe;
    if (monitorObjectsNames != NULL_PTR(StreamString *)) {
        delete [] monitorObjectsNames;
    }
}

MARTe::ErrorManagement::ErrorType AlarmBroker::Trigger() {
    using namespace MARTe;
    ErrorManagement::ErrorType err;
    TCPSocket tcpSocket;
    err.fatalError = !tcpSocket.Open();
    if (err.ErrorsCleared()) {
        err.fatalError = !tcpSocket.Connect(tcpAddress.Buffer(), tcpPort);
        if (!err.ErrorsCleared()) {
            REPORT_ERROR(ErrorManagement::FatalError, "Failed to connect to %s:%d", tcpAddress.Buffer(), tcpPort);
        }
    }
    StreamStructuredData<JsonPrinter> sdata;
    sdata.SetStream(tcpSocket);
    if (err.ErrorsCleared()) {
        err.fatalError = !sdata.GetPrinter()->PrintBegin();
    }
    uint32 i;
    for (i=0u; (err.ErrorsCleared()) && (i<numberOfObjects); i++) {
        ReferenceT<Object> obj = monitorObjects.Get(i);
        err.fatalError = !sdata.CreateAbsolute(obj->GetName());
        if (err.ErrorsCleared()) {
            err.fatalError = !obj->ExportData(sdata);
        }
        if (err.ErrorsCleared()) {
            err.fatalError = !sdata.MoveToAncestor(1u);
        }
    }
    if (err.ErrorsCleared()) {
        err.fatalError = !sdata.GetPrinter()->PrintEnd();
    }
    if (err.ErrorsCleared()) {
        err.fatalError = !tcpSocket.Flush();
    }
    if (err.ErrorsCleared()) {
        err.fatalError = !tcpSocket.Close();
    }
    return err;
}

MARTe::ErrorManagement::ErrorType AlarmBroker::Start() {
    using namespace MARTe;
    uint32 i;
    ErrorManagement::ErrorType err;

    for (i=0u; (err.ErrorsCleared()) && (i<numberOfObjects); i++) {
        ReferenceT<Object> obj = ObjectRegistryDatabase::Instance()->Find(monitorObjectsNames[i].Buffer());
        err.fatalError = !obj.IsValid();
        if (err.ErrorsCleared()) {
            err.fatalError = !monitorObjects.Insert(obj);
        }
        else {
            REPORT_ERROR(ErrorManagement::FatalError, "Object %s does not exist", monitorObjectsNames[i].Buffer());
        }
    }
    return ErrorManagement::NoError;
}

bool AlarmBroker::Initialise(MARTe::StructuredDataI & data) {
    using namespace MARTe;
    bool ok = Object::Initialise(data);
    if (ok) {
        ok = data.Read("TCPAddress", tcpAddress);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "TCPAddress must be specified");
        }
    }
    if (ok) {
        ok = data.Read("TCPPort", tcpPort);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "TCPPort must be specified");
        }
    }
    AnyType arrayDescription = data.GetType("MonitorObjects");
    if (ok) {
        ok = (arrayDescription.GetDataPointer() != NULL_PTR(void *));
    }
    if (ok) {
        numberOfObjects = arrayDescription.GetNumberOfElements(0u);
        ok = (numberOfObjects > 0u);
    }
    if (ok) {
        monitorObjectsNames = new StreamString[numberOfObjects];
        Vector<StreamString> readVector(monitorObjectsNames, numberOfObjects);
        ok = data.Read("MonitorObjects", readVector);
    }
    return ok;
}

CLASS_REGISTER(AlarmBroker, "1.0")
CLASS_METHOD_REGISTER(AlarmBroker, Start)
CLASS_METHOD_REGISTER(AlarmBroker, Trigger)

