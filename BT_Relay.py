# -*- coding:utf-8 -*-
# This script need SL4A installed
import android #SL4A https://github.com/damonkohler/sl4a
import hmac
import hashlib

btAddress = '30:14:10:09:22:10'
secret = 'password'

droid = android.Android()
 

droid.toggleBluetoothState(True)
btResult = droid.bluetoothConnect("00001101-0000-1000-8000-00805F9B34FB", btAddress)
if btResult.result:
    droid.bluetoothWrite("OPEN\r\n")
    sessionTicket = droid.bluetoothReadLine().result
    #print "Session Ticket: " + sessionTicket
    sessionKey = hmac.new(key=secret, msg=sessionTicket, digestmod=hashlib.sha1).hexdigest()
    #print "Session Key: " + sessionKey
    droid.bluetoothWrite("PASS:" + sessionKey + "\r\n")
    responce = droid.bluetoothReadLine().result
    droid.bluetoothStop()
    droid.vibrate()
    #print "Responce: " + responce
    droid.dialogCreateAlert(responce)
    droid.dialogSetPositiveButtonText("OK")
    droid.dialogShow()
else:
    droid.dialogCreateAlert(responce)
    droid.dialogSetPositiveButtonText("OUT OF SERVICE")
    droid.dialogShow()
