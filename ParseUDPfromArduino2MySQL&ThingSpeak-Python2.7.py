#!/usr/bin/python

import socket
import sys
from datetime import datetime
import MySQLdb
import urllib
import httplib

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the port
server_address = ('172.23.198.3', 5005)
print('starting up on %s port %s' % server_address)
sock.bind(server_address)

host = 'localhost'
login = 'MySQL login'
password = 'MySQL DB PASSWORD'
database = 'DB NAME'
table = 'TABLE NAME'
API_key = 'THINGSPEAK API WRITE KEY'

while True:
    print('waiting to receive message')
    data, address = sock.recvfrom(1024)
    print('received %s bytes from %s' % (len(data), address))
    print(data)
    print(datetime.now())
    txt1, pm25, txt2, pm10, heater, RH, temp, txt3 = data.split(b':')
    print('pm2.5=   ', str(pm25, 'utf-8'))
    print('pm10 =   ', str(pm10, 'utf-8'))
    print('heater = ', str(heater, 'utf-8'))
    print('RH =     ', str(RH, 'utf-8'))
    print('Temp =   ', str(temp, 'utf-8'))
    strPm25 = str(pm25, 'utf-8')
    strPm10 = str(pm10, 'utf-8')
    strHeater = str(heater, 'utf-8')
    strRH = str(RH, 'utf-8')
    strTemp = str(temp, 'utf-8')
    connection = MySQLdb.connect(host, login, password, database)
    if (connection):
        print("MySQL connect sucessfully")
    else:
        print("Could not connect to MySQL")
    cursor = connection.cursor()
    cursor.execute("insert into readings (pm25,pm10,heater,RH,temp) values (%s,%s,%s,%s,%s)",
                   (strPm25, strPm10, strHeater, strRH, strTemp))
    print("affected rows = {}".format(cursor.rowcount))
    connection.commit()
    connection.close()
    params = urllib.urlencode(
        {'field1': strPm25, 'field2': strPm10, 'field3': strRH, 'key': API_key})
    headers = {"Content-type": "application/x-www-form-urlencoded", "Accept": "text/plain"}
    conn = httplib.HTTPConnection("api.thingspeak.com:80")
    try:
        conn.request("POST", "/update", params, headers)
        response = conn.getresponse()
        print(response.status, response.reason)
        data = response.read()
        print("Data uploaded OK")
        conn.close()
    except:
        print("connection failed")
