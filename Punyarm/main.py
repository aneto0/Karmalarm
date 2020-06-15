#/usr/bin/python
_copyright__ = '''
Copyright 2020 Andre C. Neto

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the 'Software'), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
'''
__license__ = 'MIT'
__author__ = 'Andre C. Neto'
__date__ = '06/06/2020'

import diagnostics
import machine
import socket
import struct
import time
import wifi

class Packet(object):
    def __init__(self, dataFormat):
        #Header -> counter, cycle time, execution time
        self.fmt = 'IIIII'
        muid = machine.unique_id()
        self.uid = muid[0] << 24 | muid[1] << 16 | muid[2] << 8 | muid[3]
        self.fmt = self.fmt + dataFormat
        self.data = []

    def update(self, counter, cycleTime, executionTime, wifiFailureCounter, values):
        self.data = [self.uid]
        self.data.extend([counter, cycleTime, executionTime, wifiFailureCounter])
        self.data.extend(values)

    def serialise(self):
        return struct.pack(self.fmt, *(self.data))


class MainApp(object):

    def __init__(self, loopPeriod, masterAddress, masterPort, diagnosticList, ledStatusPinNumber, ledStatusPinInvert = True, wifiTimeout = 5000, wifiFailureSleep = 30000, blinkUpdatePeriod = 1000, wifiFailureBlinkUpdatePeriod = 250):
        print('MainApp instantiated')
        self.wifi = wifi.WIFIManager(wifiTimeout, ledStatusPinNumber, ledStatusPinInvert)
        self.loopPeriod = loopPeriod
        self.masterAddress = masterAddress
        self.masterPort = masterPort
        self.wifiFailureSleep = wifiFailureSleep
        self.diagnosticList = diagnosticList
        dataFormat = ''
        for d in self.diagnosticList:
            dataFormat = dataFormat + d.getDataFormat()
        self.packet = Packet(dataFormat)
        self.ledStatusPin = machine.Signal(machine.Pin(ledStatusPinNumber, machine.Pin.OUT), invert = ledStatusPinInvert)
        self.blinkUpdatePeriod = blinkUpdatePeriod
        self.wifiFailureBlinkUpdatePeriod = wifiFailureBlinkUpdatePeriod

    def sendUDPPacket(self):
        ok = True
        try:
            self.sock.sendto(self.packet.serialise(), (self.masterAddress, self.masterPort))
        except Exception as e:
            print('Could not send UDP packet to {0}:{1} - {2}'.format(self.masterAddress, self.masterPort, e))
            ok = False
        return ok

    def connectToWifi(self, forceWifiReset):
        ok = False
        print('Connecting to WIFI')
        wifiStatus = self.wifi.connect(forceWifiReset)
        self.sock = None
        if (wifiStatus == wifi.WIFI_RECONNECTED):
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 
            print('Successfully connected to WIFI')
            ok = True
        print('Returning {0}'.format(ok))
        return ok

    def loop(self):
        self.connectToWifi(True) 
        loopPeriodUS = self.loopPeriod * 1000 
        nextSleepWakeTime = time.ticks_add(time.ticks_us(), loopPeriodUS)
        lastCycleStartTime = time.ticks_us()
        counter = 0
        lastCycleExecutionTime = 0
        counterUpdateCount = self.blinkUpdatePeriod / self.loopPeriod
        wifiFailureCounter = 0 
        packetFailureConsecutiveCounter = 0 

        while (True):
            #Compute cycle time and associated statistics
            timeToSleep = time.ticks_diff(nextSleepWakeTime, time.ticks_us())
            if (timeToSleep > 0):
                time.sleep_us(timeToSleep)
            cycleStartTime = time.ticks_us()
            actualExecutionPeriod = (cycleStartTime - lastCycleStartTime)
            lastCycleStartTime = cycleStartTime
            periodError = actualExecutionPeriod - loopPeriodUS
            if (abs(periodError) > (loopPeriodUS / 2)):
                periodError = 0
            nextSleepWakeTime = time.ticks_add(cycleStartTime, loopPeriodUS - periodError)

            #Get data from all the connected diagnostics
            diagnosticValues = []
            for d in self.diagnosticList:
                diagnosticValues.append(d.getValue())

            self.packet.update(counter, actualExecutionPeriod, lastCycleExecutionTime, wifiFailureCounter, diagnosticValues)
            counter = counter + 1
            if(self.sendUDPPacket()):
                if ((counter % counterUpdateCount) == 0):
                    self.ledStatusPin.on()
                else:
                    self.ledStatusPin.off()
                packetFailureConsecutiveCounter = 0
            else:
                packetFailureConsecutiveCounter = packetFailureConsecutiveCounter + 1
                forceWifiReset = (packetFailureConsecutiveCounter == 3)
                forceMicroReset = (packetFailureConsecutiveCounter == 12)
                print('Failed to send packet. Reconnecting to network: {0}'.format(packetFailureConsecutiveCounter))
                if (forceMicroReset):
                    print('Giving up and rebooting... bye')
                    packetFailureConsecutiveCounter = 0
                    machine.reset()

                if(not self.connectToWifi(forceWifiReset)):
                    wifiFailureCounter = wifiFailureCounter + 1
                    print('wifiFailureCounter = {0}'.format(wifiFailureCounter))
                    #Wait before trying again. This looks serious...
                    wifiFailedTime = time.ticks_ms()
                    wifiFailedTryAgain = time.ticks_add(wifiFailedTime, self.wifiFailureSleep)
                    while(time.ticks_diff(wifiFailedTryAgain, time.ticks_ms()) > 0):
                        #Blink at a faster rate
                        time.sleep_ms(int(self.wifiFailureBlinkUpdatePeriod / 2))
                        self.ledStatusPin.on()
                        time.sleep_ms(int(self.wifiFailureBlinkUpdatePeriod / 2))
                        self.ledStatusPin.off()
            lastCycleExecutionTime = time.ticks_diff(time.ticks_us(), cycleStartTime)


if __name__ == "__main__":
    UDP_IP = '192.168.0.21'
    #UDP_IP = '239.1.1.5'
    UDP_PORT = 44441
    PERIOD_MS = 100

    LED_STATUS_GPIO_PIN = 2

    PIR_GPIO_PIN = 0
    PIR_GPIO_DEBUG_PIN = 16

    ULTRASONIC_ECHO_GPIO_PIN = 5
    ULTRASONIC_TRIGGER_GPIO_PIN = 4

    diagnosticList = [diagnostics.PIR(PIR_GPIO_PIN, PIR_GPIO_DEBUG_PIN), diagnostics.Ultrasonic(ULTRASONIC_ECHO_GPIO_PIN, ULTRASONIC_TRIGGER_GPIO_PIN)]
    app = MainApp(PERIOD_MS, UDP_IP, UDP_PORT, diagnosticList, LED_STATUS_GPIO_PIN)
    app.loop()
 

