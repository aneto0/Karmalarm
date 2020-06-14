#/usr/bin/python
_copyright__ = '''
Copyright 2020 Andre C. Neto

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the 'Software'), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
'''
__license__ = 'MIT'
__author__ = 'Andre C. Neto'
__date__ = '12/06/2020'

import argparse
import json
import logging
import logging.handlers
import smtplib, ssl
import socket
import socketserver
import telegram.ext
import threading
import time
import urllib.request

from enum import Enum

#Configure the logging format
dateFormat = '%Y-%m-%d %H:%M:%S'
loggingFormat = logging.Formatter(fmt='[%(asctime)s] [%(levelname)s] [%(process)d] [%(filename)s:%(lineno)d] %(message)s', datefmt=dateFormat)

#Configure the logger
logger = logging.getLogger('{0}'.format(__name__))

#Console handler
consoleHandler = logging.StreamHandler()
consoleHandler.setFormatter(loggingFormat)

#Syslog handler
syslogHandler = logging.handlers.SysLogHandler('/dev/log')
syslogHandler.setFormatter(loggingFormat)

logger.addHandler(consoleHandler)
logger.addHandler(syslogHandler)

latchedAlarmStatus = {}
alarmHandlers = {}
       
#TODO clean...
STATUS_BASE_URL = 'http://localhost:8084/'
statusObjects = ['ObjectBrowse/JivarmApp/Functions/GAMsPunyarms/GAMsPunyarm1/GAMUDPPunyarm1', 'ObjectBrowse/JivarmApp/Functions/GAMsPunyarms/GAMsPunyarm2/GAMUDPPunyarm2', 'ObjectBrowse/JivarmApp/Functions/GAMsPunyarms/GAMsPunyarm3/GAMUDPPunyarm3', 'ObjectBrowse/JivarmApp/Functions/GAMsPowerMonitor/GAMWiringPiInput']

#Template class for alarms
class AlarmHandler(object):

    def __init__(self):
        self.loggingStrings = {logging.NOTSET: 'NOTSET', logging.DEBUG: 'DEBUG', logging.INFO: 'INFO', logging.WARNING: 'WARNING', logging.ERROR: 'ERROR', logging.CRITICAL: 'CRITICAL'}

    def trigger(self, msg, severity):
        #NOOP
        log.critical('Should not be recheable')

    def getLoggingString(self, severity):
        ret = self.loggingStrings[logging.NOTSET]
        if (severity in self.loggingStrings):
            ret = self.loggingStrings[severity]
        return ret

class TelegramBotHandler(AlarmHandler):

    def __init__(self, token, admin):
        super().__init__()
        self.updater = telegram.ext.Updater(token=token, use_context=True)
        self.dispatcher = self.updater.dispatcher

        monitorHandler = telegram.ext.CommandHandler('monitor', self.monitor, filters = telegram.ext.Filters.user(username=admin))
        self.dispatcher.add_handler(monitorHandler)

        resetHandler = telegram.ext.CommandHandler('reset', self.reset, filters = telegram.ext.Filters.user(username=admin))
        self.dispatcher.add_handler(resetHandler)

        greetHandler = telegram.ext.CommandHandler('greet', self.greet)
        self.dispatcher.add_handler(greetHandler)

        statusHandler = telegram.ext.CommandHandler('status', self.status)
        self.dispatcher.add_handler(statusHandler)

        helpHandler = telegram.ext.CommandHandler('help', self.help)
        self.dispatcher.add_handler(helpHandler)

        unknownHandler = telegram.ext.MessageHandler(telegram.ext.Filters.command, self.unknown)
        self.dispatcher.add_handler(unknownHandler)

        self.chatIds = []
        self.context = None

    def serveForever(self):
        self.updater.start_polling()
        logger.info('Telegram bot started')
        self.updater.idle()

    def greet(self, update, context):
        logger.debug('Telegram bot greet called')
        context.bot.sendMessage(chat_id=update.effective_chat.id, text='Hello there!')

    def status(self, update, context):
        logger.debug('Telegram bot status called')
        statusText = '*Raw*\n'
        statusText += '*MARTe*\n'

        for statusObjAddr in statusObjects:
            urlToQuery = '{0}/{1}?TextMode=0'.format(STATUS_BASE_URL, statusObjAddr)
            statusText += '```\n'
            try:
                parsedjson = {}
                with urllib.request.urlopen(urlToQuery) as response:
                    rawjson = response.read()
                    parsedjson = json.loads(rawjson.decode('ascii'))
                statusText += json.dumps(parsedjson, indent=4, sort_keys=True)
            except Exception as e:
                statusText += 'Failed to retrieve {0} : {1}'.format(urlToQuery, e)
            statusText += '```\n'

        statusText += '\n'
        statusText += '*Latched status*\n'
        statusText += '```\n'
        statusText += json.dumps(latchedAlarmStatus, indent=4, sort_keys=True)
        statusText += '```\n'
        context.bot.sendMessage(chat_id=update.effective_chat.id, text=statusText, parse_mode='MarkdownV2')

    def help(self, update, context):
        helpMsg = '/status\n'
        helpMsg += '/monitor\n'
        helpMsg += '/reset [PREFIX]\n'
        helpMsg += '/greet\n'
        context.bot.sendMessage(chat_id=update.effective_chat.id, text=helpMsg)

    def monitor(self, update, context):
        chatId = update.effective_chat.id
        self.context = context
        if (chatId not in self.chatIds):
            self.chatIds.append(chatId)
        logger.debug('Telegram bot is now going to monitor alarms in chat id {0}'.format(chatId))
        context.bot.sendMessage(chat_id=chatId, text='Monitor is now active for this chat')

    def reset(self, update, context):
        text = update.message.text
        args = text.split(' ')
        print(len(args))
        if (len(args) > 1):
            args = args[1]
        else:
            args = ''
        logger.debug('Reseting alarm status for [{0}]'.format(args))
        if (len(args) > 0):
            resetAlarmStatus(args)
        else:
            resetAlarmStatus()
        context.bot.sendMessage(chat_id=chatId, text='Alarms reset')

    def unknown(self, update, context):
        logger.debug('Telegram bot unknown called')
        context.bot.send_message(chat_id=update.effective_chat.id, text='Sorry, I did not understand that command.')

    def trigger(self, msg, severity):
        if (self.context is not None):
            for chatId in self.chatIds:
                self.context.bot.send_message(chat_id=chatId, text='Alarm: {0}\n\n\n{1}'.format(self.getLoggingString(severity), msg))

class EMailAlarmHandler(AlarmHandler):

    def __init__(self, username, password, destination):
        super().__init__()
        self.emailUsername = username
        self.emailPassword = password
        self.emailDestination = destination
        self.messageSubject = 'Power detector event'
        self.port = 587 
        self.smtpServer = 'smtp.gmail.com'

    
    def trigger(self, msg, severity):
        try:
            #context = ssl.create_default_context()
            #server = smtplib.SMTP_SSL(smtpServer, port, context)
            
            server = smtplib.SMTP(self.smtpServer, self.port)
            server.ehlo()
            server.starttls()
            server.login(self.emailUsername, self.emailPassword)

            message =  '\r\n'.join([
                'From: {0}'.format(self.emailUsername),
                'To: {0}'.format(self.emailDestination.split(',')),
                'Subject: {0} - {1}'.format(self.messageSubject, self.getLoggingString(severity)),
                '',
                '{0}'.format(msg)
            ])
            server.sendmail(self.emailUsername, self.emailDestination.split(','), message)
            server.quit()
        except Exception as e:
            logger.critical('Failed to send e-mail {0}'.format(e))

class BrokerTCPHandler(socketserver.BaseRequestHandler):

    def handle(self):
        logger.debug('Received TCP request from {0}'.format(self.client_address[0]))
        jsonRawFrame = ''
        try:
            waitForFrame = True
            while(waitForFrame):
                msgBytes = self.request.recv(4096)
                waitForFrame = (len(msgBytes) > 0)
                if (waitForFrame):
                    msgStr = msgBytes.decode('ascii')
                    jsonRawFrame = jsonRawFrame + msgStr
                    pass
        except Exception as e:
            logger.critical('TCP exception: {0}'.format(e))
        try:
            logger.debug('Received raw json: {0}'.format(jsonRawFrame))
            jsonFrame = json.loads(jsonRawFrame)
            logger.debug('json is valid: {0}'.format(jsonFrame))
            triggeredAlarmStatus = {}
            alarmsToTrigger = []
            alarmGAMSignals = jsonFrame['GAMAlarmTrigger']['InputSignals']
            for t in alarmGAMSignals:
                if (t in latchedAlarmStatus):
                    triggeredAlarmStatus[t] = int(alarmGAMSignals[t])
                else:
                    logger.critical('Signal {0} is not defined'.format(t))
            for t in triggeredAlarmStatus:
                if (triggeredAlarmStatus[t] == 1):
                    if (triggeredAlarmStatus[t] != latchedAlarmStatus[t]):
                        for alarmHandler in alarmHandlers[t]:
                            if (alarmHandler not in alarmsToTrigger):
                                alarmsToTrigger.append(alarmHandler)
                        latchedAlarmStatus[t] = triggeredAlarmStatus[t]
            for a in alarmsToTrigger:
                logger.debug('Triggering alarm {0}'.format(a))
                a.trigger(json.dumps(jsonFrame, indent=4, sort_keys=True), logging.CRITICAL)
        except Exception as e:
            logger.critical('Received an invalid json: {0}'.format(e))

def tcpServeWaitForever(brokerPort):
    TCP_IP = '0.0.0.0'
    tcpBrokerServer = socketserver.ThreadingTCPServer((TCP_IP, brokerPort), BrokerTCPHandler)
    tcpBrokerServer.serve_forever()

def resetAlarmStatus(prefix = 'P'):
    for k in latchedAlarmStatus:
        if (k.startswith(prefix)):
            latchedAlarmStatus[k] = 0

#... ugly... todo clean
def initGlobal(defaultAlarmTypes):
    punyPrefix = 'Punyarm{0}AlarmTriggers.{1}'
    for i in range(1, 4):
        for at in ['Watchdog', 'PIR', 'Ultrasonic']:
            punyPrefixId = punyPrefix.format(i, at)
            logger.info('Setting alarms for {0}'.format(punyPrefixId))
            alarmHandlers[punyPrefixId] = defaultAlarmTypes
            latchedAlarmStatus[punyPrefixId] = 0
   
    powerMonitorId = 'PowerMonitorTrigger'
    logger.info('Setting alarms for {0}'.format(powerMonitorId))
    alarmHandlers[powerMonitorId] = defaultAlarmTypes
    latchedAlarmStatus[powerMonitorId] = 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description = 'Measure the power on the defined ADC and trigger an alarm if the power is lower than a given value')
    logLevels = ['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL']
    parser.add_argument('-bp', '--broker_port', type=int, default=8888, help='Broker TCP port')
    parser.add_argument('-ll', '--log_level', type=str, default='INFO', help='Log level', choices = logLevels)
    parser.add_argument('-eu', '--email_user', type=str, required=True, help='email username')
    parser.add_argument('-ep', '--email_pass', type=str, required=True, help='email password')
    parser.add_argument('-ed', '--email_dest', type=str, required=True, help='email destination')
    parser.add_argument('-tt', '--telegram_bot_token', type=str, required=True, help='Telegram bot token')
    parser.add_argument('-ta', '--telegram_bot_admin', type=str, required=True, help='Telegram bot admin username')
   
    args = parser.parse_args()
    loggingCriticalities = {'DEBUG': logging.DEBUG, 'INFO': logging.INFO, 'WARNING': logging.WARNING, 'ERROR': logging.ERROR, 'CRITICAL': logging.CRITICAL}
    logger.setLevel(loggingCriticalities[args.log_level])
    consoleHandler.setLevel(loggingCriticalities[args.log_level])
    syslogHandler.setLevel(loggingCriticalities[args.log_level])

    emailAlarmHandler = EMailAlarmHandler(args.email_user, args.email_pass, args.email_dest)
    telegramBotHandler = TelegramBotHandler(args.telegram_bot_token, args.telegram_bot_admin)

    initGlobal([emailAlarmHandler, telegramBotHandler])
    resetAlarmStatus()

    tcpListenThread = threading.Thread(target=tcpServeWaitForever, args=(args.broker_port, ))
    tcpListenThread.start()

    telegramBotHandler.serveForever()
