#!/bin/bash
#Arguments -f FILENAME -m MESSAGE [-d cgdb|strace]
#-f FILENAME=MARTe configuration file
#-m MESSAGE=Start message
#-d cgdb=Run with cgdb
#-d strace=Run with strace

#Change to the current directory so that it can be bootstraped from an init file
cd "$(dirname "$0")"

#Run with cgdb or strace?
DEBUG=""

#Consume input arguments
while [[ $# -gt 1 ]]
do
key="$1"

case $key in
    -f|--file)
    FILE="$2"
    shift # past argument
    ;;
    -m|--message)
    MESSAGE="$2"
    shift # past argument
    ;;
    -d|--debug)
    DEBUG="$2"
    shift # past argument
    ;;
    --default)
    DEFAULT=YES
    ;;
    *)
    # unknown option
    ;;
esac
shift # past argument or value
done

if [ -z ${TARGET+x} ]; then echo "Please set the TARGET variable"; exit; fi
if [ -z ${MARTe2_DIR+x} ]; then echo "Please set the MARTe2_DIR environment variable"; exit; fi
if [ -z ${MARTe2_Components_DIR+x} ]; then echo "Please set the MARTe2_Components_DIR environment variable"; exit; fi

#"Compile" the cfg
make -C ../Configurations -f Makefile.cfg clean
make -C ../ -f Makefile.gcc

LD_LIBRARY_PATH=.
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/DataSources/UDP/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/DataSources/WiringPi/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/GAMs/AlarmTriggerGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/GAMs/PowerMonitorGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/GAMs/UltrasonicGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/GAMs/WatchdogGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../Build/$TARGET/Interfaces/AlarmBroker/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_DIR/Build/$TARGET/Core/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/LinuxTimer/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/LoggerDataSource/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/FileDataSource/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/MDSWriter/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/DataSources/RealTimeThreadSynchronisation/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/ConstantGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/ConversionGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/HistogramGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/IOGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/FilterGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/TriggerOnChangeGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/GAMs/WaveformGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/Interfaces/MemoryGate/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/$TARGET/Components/Interfaces/SysLogger/

echo $LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH

#Start with cgdb or with strace
if [ "$DEBUG" = "cgdb" ]
then
    cgdb --args $MARTe2_DIR/Build/$TARGET/App/MARTeApp.ex -l RealTimeLoader -f $FILE -m $MESSAGE
elif [ "$DEBUG" = "strace" ]
then
    strace -s256 -o/tmp/strace.err $MARTe2_DIR/Build/$TARGET/App/MARTeApp.ex -l RealTimeLoader -f $FILE -m $MESSAGE
else
    $MARTe2_DIR/Build/$TARGET/App/MARTeApp.ex -l RealTimeLoader -f $FILE -m $MESSAGE 
fi

