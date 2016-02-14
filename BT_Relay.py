# -*- coding:utf-8 -*-
import android
import hmac
import hashlib

btAddress = ''
secret = ''

droid = android.Android()

droid.toggleBluetoothState(True)
btResult = droid.bluetoothConnect("00001101-0000-1000-8000-00805F9B34FB")
if btResult.result:
    droid.dialogCreateAlert(u"Enter secret key")
    droid.dialogCreatePassword(secret)
    droid.dialogSetPositiveButtonText(u"Open")
    droid.dialogSetNegativeButtonText(u"Cancel")
    droid.dialogShow()
    response = droid.dialogGetResponse()
    if (response.result['which'] == u'positive'):
        secret = response.result['value']    
        droid.bluetoothWrite("OPEN\r\n")
        sessionTicket = droid.bluetoothReadLine().result
        #print "Session Ticket: " + sessionTicket
        sessionKey = hmac.new(key=secret, msg=sessionTicket, digestmod=hashlib.sha1).hexdigest()
        #print "Session Key: " + sessionKey
        droid.bluetoothWrite("PASS:" + sessionKey + "\r\n")
        responce = droid.bluetoothReadLine().result
        droid.vibrate()
        #print "Responce: " + responce
        droid.dialogCreateAlert(responce)
        droid.dialogSetPositiveButtonText("Done")
        droid.dialogShow()
    droid.bluetoothStop()
else:
    droid.dialogCreateAlert("OUT OF SERVICE")
    droid.dialogSetPositiveButtonText("Done")
    droid.dialogShow()

