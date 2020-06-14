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

import machine
import time

class Diagnostic(object):

    def __init__(self):
        pass

    def readValue(self):
        pass

    def getDataFormat(self):
        pass

class PIR(Diagnostic): 

    def __init__(self, pirPinNumber, ledDebugPinNumber = -1, ledDebugPinInvert = True):
        self.pirPin = machine.Pin(pirPinNumber, machine.Pin.IN)
        self.debugPin = None
        if (ledDebugPinNumber >= 0):
            self.debugPin = machine.Signal(machine.Pin(ledDebugPinNumber, machine.Pin.OUT), invert = ledDebugPinInvert)

    def getValue(self):
        val = self.pirPin.value()
        if (self.debugPin is not None):
            if (val == 1):
                self.debugPin.on()
            else:
                self.debugPin.off()
        return int(val)

    def getDataFormat(self):
        return 'I'

class Ultrasonic(Diagnostic): 

    def __init__(self, echoPinNumber, triggerPinNumber):
        self.echoPin = machine.Pin(echoPinNumber, machine.Pin.IN)
        self.triggerPin = machine.Pin(triggerPinNumber, machine.Pin.OUT)

        #Could correct for temperature in the future...
        self.SOUND_SPEED_UM_US = 343
        #Max range 400 cm + 100 cm margin => 5000000 um x 2 (fly back)
        MAX_RANGE_UM = 5000000 * 2
        self.pulseLengthTimeout = int(MAX_RANGE_UM / self.SOUND_SPEED_UM_US)
        print('Pulse length timeout is {0}'.format(self.pulseLengthTimeout))

    def getValue(self):
        pulseLength = -1
        self.triggerPin.value(0)
        time.sleep_us(5)
        #Send a 10 us waveform
        self.triggerPin.value(1)
        time.sleep_us(10)
        self.triggerPin.value(0)
        try:
            pulseLength = machine.time_pulse_us(self.echoPin, 1, self.pulseLengthTimeout)
        except OSError as e:
            #Timeout (could check for e.args[0] == 110), but to do what?
            pulseLength = -1

        #Compute distance in us
        distance = -1
        if (pulseLength > -1):
            distance = pulseLength * self.SOUND_SPEED_UM_US / 2
        return int(distance)

    def getDataFormat(self):
        return 'I'

