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
import network
import time
import wifi_secrets

#WIFI status
WIFI_DISCONNECTED = -1
WIFI_ALREADY_CONNECTED = 1
WIFI_RECONNECTED = 2

class WIFIManager(object):
    ''' Manages the wifi connection. The user specific SSID and password shall be set in the wifi_secrets.py and named WIFI_ESSID and WIFI_PASS, respectively.
    '''

    def __init__(self, timeout = 5000, ledStatusPin = 2, ledStatusPinInverted = True):
        '''
        Args:
            timeout (uint): maximum time, in milliseconds, to wait for a successful (re)connection
            ledStatusPin (uint): if > 0, the LED identified by the GPIO pin ledStatusPin will be used (by disconnecting it) to signal a successful connection
            ledStatusPinInverted (bool): if ledStatusPin > 0, selects the polarity of the LED
        '''
        self.timeout = timeout
        self.ledStatusPin = None
        self.status = WIFI_RECONNECTED
        if (ledStatusPin >= 0):
            self.ledStatusPin = machine.Signal(machine.Pin(ledStatusPin, machine.Pin.OUT), invert = ledStatusPinInverted)

    def connect(self, forceWifiReset = True):
        '''
        Args:
            timeout (uint): maximum in milliseconds to wait for a connection
        Returns:
            DISCONNECTED if the connection failed, ALREADY_CONNECTED if the connection was already active and is still valid, RECONNECTED if the connection had to be re-established 
        '''
        sta_if = network.WLAN(network.STA_IF)
        maxWaitTime = time.ticks_add(time.ticks_ms(), self.timeout)
        if (forceWifiReset):
            self.status = WIFI_DISCONNECTED
            sta_if.disconnect()
        elif (not sta_if.isconnected()):
            self.status = WIFI_DISCONNECTED

        if (self.status == WIFI_DISCONNECTED):
            if (self.ledStatusPin is not None):
                #Blink to inform that we are going to connect
                for b in range(10):
                    if ((b % 2) == 1):
                        self.ledStatusPin.off()
                    else:
                        self.ledStatusPin.on()
                    time.sleep_ms(100)
            self.ledStatusPin.on()
            ap = network.WLAN(network.AP_IF)
            ap.active(True)
            print('Connecting to network...')
            sta_if.active(True)
            sta_if.connect(wifi_secrets.WIFI_ESSID, wifi_secrets.WIFI_PASS)

            self.status = WIFI_RECONNECTED
            while not sta_if.isconnected():
                time.sleep_ms(100)
                if (time.ticks_diff(time.ticks_ms(), maxWaitTime) > 0):
                    print('Failed to connect to the network...')
                    self.status = WIFI_DISCONNECTED
                    break
            if (self.status == WIFI_RECONNECTED):
                if (self.ledStatusPin is not None):
                    self.ledStatusPin.off()
                print('Network config:', sta_if.ifconfig())
                #Required for multicast to work! 
                ap.active(False)
        elif (self.status == WIFI_RECONNECTED):
            self.status = WIFI_ALREADY_CONNECTED
        return self.status
