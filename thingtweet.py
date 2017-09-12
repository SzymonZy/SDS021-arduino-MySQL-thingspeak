#!/usr/bin/python
# coding: UTF-8
#code by IV LO w Czestochowie, Bartlomiej Meller, Milos Galas, Szymon Zycinski
import requests
import MySQLdb

host = 'localhost'
login = 'login'
password = 'password'
database = 'dust'
table = 'readings'

API_KEY="Twitter key"
url="http://api.thingspeak.com/apps/thingtweet/1/statuses/update?api_key="
fill="&status="
url=url+API_KEY+fill

connection = MySQLdb.connect(host, login, password, database)
if (connection):
        print("MySQL connect sucessfully")
else:
        print("Could not connect to MySQL")
cursor = connection.cursor()
cursor.execute("select round(avg(pm25),2) as 'PM2.5', round(avg(pm10),2) as PM10 from readings where time>now()-INTERVAL 1 HOUR;")
row=cursor.fetchone();
connection.close()
tweet="Your text here\nPM2.5: "+str(row[0])+"\nPM10: "+str(row[1])+"\nyour text here"
url=url+tweet
print("Sending data ")
response=requests.get(url)
if response.status_code==200:
    print("Tweet sent")
else:
    print("Problem occured. Error code=")
print(response.status_code)
