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
# $Id: Makefile.inc 3 2012-01-15 16:26:07Z aneto $
#
#############################################################
SPB    = DataSources/UDP.x \
	GAMs/AlarmTriggerGAM.x \
	GAMs/PowerMonitorGAM.x \
	GAMs/UltrasonicGAM.x \
	GAMs/WatchdogGAM.x \
	Interfaces/AlarmBroker.x

#MACHINE could have beee only set in the bash but not exported...
MACHINE ?= $(shell uname -m)
ifeq ($(MACHINE), aarch64)
SPB += DataSources/WiringPi.x
endif

MAKEDEFAULTDIR=$(MARTe2_DIR)/MakeDefaults

ROOT_DIR=.
include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

all: $(OBJS) $(SUBPROJ) config check-env
	echo  $(OBJS)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)

config:
	make -C Configurations -f Makefile.cfg clean
	make -C Configurations -f Makefile.cfg

check-env:
ifndef MARTe2_DIR
    $(error MARTe2_DIR is undefined)
endif

