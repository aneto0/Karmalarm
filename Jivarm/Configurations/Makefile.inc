#############################################################
#
# Copyright 2020 Andre Neto
# 
# Licensed under the EUPL, Version 1.1 or - as soon they 
# will be approved by the European Commission - subsequent  
# versions of the EUPL (the "Licence"); 
# You may not use this work except in compliance with the 
# Licence. 
# You may obtain a copy of the Licence at: 
#  
# http://ec.europa.eu/idabc/eupl
#
# Unless required by applicable law or agreed to in 
# writing, software distributed under the Licence is 
# distributed on an "AS IS" basis, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
# express or implied. 
# See the Licence for the specific language governing 
# permissions and limitations under the Licence. 
#
#############################################################

#Named of the unit files to be compiled
OBJSX=Startup.x

#Location of the Build directory where the configuration file will be written to
BUILD_DIR?=.

#Location of the MakeDefaults directory.
#Note that the MARTe2_DIR environment variable
#must have been exported before
MAKEDEFAULTDIR=$(MARTe2_DIR)/MakeDefaults

include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

CFLAGS += -DMARTe2_DIR=${MARTe2_DIR}

ifdef DEBUGGAM
CFLAGS += -DDEBUGGAM
endif

#MACHINE could have beee only set in the bash but not exported...
MACHINE ?= $(shell uname -m)
ifeq ($(MACHINE), aarch64)
CFLAGS += -DWIRING_PI
endif

all: $(OBJS) 
	echo  $(OBJS)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

