/********************************************************************
Code by IV Liceum Ogolnoksztalcace im. H. Sienkiewicza w Czestochowie
Jan Konopka, Bartlomiej Meller, Galas Milosz, Szymon Zycinski
*********************************************************************/

#include "SparkFun_Si7021_Breakout_Library.h"
#include <Wire.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#define interval 899000                   //delay between each measurement in seconds - 5  minutes - 300 000, 15 minutes 900 000
#define relay_pin 6                       //Heater relay command pin
#define suck_time 3000                    //time to suck air into sensor before returning results

float RH=0;                               //variable to keep realtive humidity
Weather sensor;                           //object that represents Si7021

SoftwareSerial particleSensor(4, 5);      // RX, TX for SDS021
float pm25;                               //2.5um particles detected in ug/m3
float pm10;                               //10um particles detected in ug/m3
unsigned int deviceID;                    //Two byte unique ID set by factor
unsigned long TimeS;                      //current time since last measurement
unsigned long Ttmp;                       //Temporary variable to hold seconds to next measurement

//Initialize Ethernet
EthernetUDP Udp;
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
unsigned int localport = 7708;
IPAddress remoteIP(192,168,1,23);          //CHANGE THIS ONE! To your python host address.
unsigned int remotePort = 5005;

int heaterState=1;                        //default heater on - 1, tu turn off set 0
int renew;                                //DHCP renew result
int no=0;                                 //number of proceded meassurement

void setup()
{
  Serial.begin(9600);                     //open serial over USB at 9600 baud
  Serial.println("Booting UP:");
  Ethernet.begin(mac);                    //Get DHCP IP address
  //Ipaddress ip[]={172,23,198,95};       //set up static IP instead of the DHCP one
  //Ethernet.begin(mac,IP);               //just uncomment this two lines and comment one above
  Serial.print("IP : ");
  Serial.println(Ethernet.localIP());
 
  delay(1000);
  Serial.println("Initialize software serial for SDS021");
  particleSensor.begin(9600);             //SetUp software serial port for SDS021
 
  delay(1000);
  Serial.println("Initialize Si7021");
  sensor.begin();
 
  delay(1000);
  Serial.println("Initialize relay control");
  pinMode(relay_pin, OUTPUT);             //set relay command pin as digital output
  digitalWrite(relay_pin,HIGH);           //turn the heater off

  delay(1000);
  TimeS=millis();                         //get current time since power on
  Ttmp=interval/1000;                     //initial seconds to next measurement
  heaterState=1;                          //let's start with heater on
}

void loop()
{
  //if (millis()<TimeS)                     //Take care of millis() rollover!
  //    TimeS=0;                            //if current time is earlier then last measure there were a rollover
  RH=sensor.getRH();                      //save current relative humidity as RH
  //Serial.print(" RH=");
  //Serial.println(RH);
  if (RH<=50)
  {
    //Serial.println("Heater is OFF");
    heaterState=0;
  }
  else
    if (RH>50&&heaterState==1)
    {
        //Serial.println("Heater is ON");
    }
    else
      if (RH>50&&heaterState==0)
      {
          //Serial.println("Heater is OFF");
      }
   if (RH>=65||sensor.getTemp()<0)
   {
      //Serial.println("Heater is ON");
      heaterState=1;
  }

  if (heaterState==0)
    digitalWrite(relay_pin,HIGH);     //turn OFF
  else
    if (sensor.getTemp()<60)          //Don't OVERHEAT! It can be dangerous!
        digitalWrite(relay_pin,LOW);  //turn ON
 
  renew=Ethernet.maintain();
  switch (renew)
  {
    //case 0 : Serial.println("Nog happend"); break;
    case 1 : Serial.println("Renew failed"); break;
    case 2 : Serial.println("Renew success"); Serial.println(Ethernet.localIP()); break;
    case 3 : Serial.println("Rebind fail"); break;
    case 4 : Serial.println("Rebind sucess"); Serial.println(Ethernet.localIP()); break;
    //default: Serial.println("Dunno what happend");
  }
  //Serial.print("My maintaned IP address is: ");
  //Serial.println(Ethernet.localIP());
 
  if (Ttmp!=((TimeS+interval-millis())/1000))
  {
    Serial.print("Time to wake up: ");
    Serial.print(Ttmp);
    Serial.print(" RH=");
    Serial.print(RH);
    Serial.print(" Heater state=");
    Serial.println(heaterState);

  }
  Ttmp=(TimeS+interval-millis())/1000;
  //Serial.println((TimeS+interval-millis())/1000);
 
  while(particleSensor.available())           //Empty particle sensor buffer
    particleSensor.read();                    //this prevents Soft Serial to hang
 
  if(millis()-TimeS > interval-suck_time)
  {
      Serial.print("Measure no: ");
      Serial.println(++no);
      Serial.println("Read dust:");
      wakeUp();
      delay(suck_time);
      if (dataAvailable()==1)
          {
            Serial.print("Particle Matter [2.5]:");
            Serial.print(pm25, 1);
            Serial.print("ug/m3 [10]:");
            Serial.print(pm10, 1);
            Serial.println("ug/m3");
            Serial.println("Upload Data");
            UploadData(pm25,pm10,heaterState);
          }    
      goToSleep();
      TimeS=millis();  
  }
}

bool UploadData(float PM25, float PM10, int heater)
{
 Udp.begin(localport);
 if (Udp.beginPacket(remoteIP, remotePort)==1)
 {
  Udp.print("PM25:");
  Udp.print(PM25);
  Udp.print(":PM10:");
  Udp.print(PM10);
  Udp.print(":");
  Udp.print(heater);
  Udp.print(":");
  Udp.print(sensor.getRH());
  Udp.print(":");
  Udp.print(sensor.getTemp());
  Udp.print(":");
  Serial.println("beginPacket OK");
  if (Udp.endPacket()==1)
    Serial.println("Pakiet zaakceptowany");
  else
    Serial.println("Pakiet niezaakceptowany");
  Udp.stop();
  return 1;
 }
 Udp.stop();
 return 0;
}


void getFirmwareVersion(void)
{
  sendCommand(7, 0, 0); //Command number is 7, no databytes
}

//Tell the module to go to sleep
void goToSleep(void)
{
  sendCommand(6, 1, 0); //Command number is 6, set mode = 1, sleep = 0
}

//Tell module to start working!
void wakeUp(void)
{
  sendCommand(6, 1, 1); //Command number is 6, set mode = 1, work = 1
}

//Scans for incoming packet
//Times out after 1500 miliseconds
boolean dataAvailable(void)
{
  //Spin until we hear meassage header byte
  long startTime = millis();

  while (1)
  {
    while (!particleSensor.available())
    {
      delay(1);
      if (millis() - startTime > 1500) return (false); //Timeout error
    }

    if (particleSensor.read() == 0xAA) break; //We have the message header
  }

  //Read the next 9 bytes
  byte sensorValue[10];
  for (byte spot = 1 ; spot < 10 ; spot++)
  {
    startTime = millis();
    while (!particleSensor.available())
    {
      delay(1);
      if (millis() - startTime > 1500) return (false); //Timeout error
    }

    sensorValue[spot] = particleSensor.read();
  }

  //Check CRC
  byte crc = 0;
  for (byte x = 2 ; x < 8 ; x++) //DATA1+DATA2+...+DATA6
    crc += sensorValue[x];
  if (crc != sensorValue[8])
    return (false); //CRC error

  if (sensorValue[1] == 0xC0) //This is just a normal reading
  {
    //Update the global variables
    pm25 = ((float)sensorValue[3] * 256 + sensorValue[2]) / 10;
    pm10 = ((float)sensorValue[5] * 256 + sensorValue[4]) / 10;

    deviceID = sensorValue[6] * 256 + sensorValue[7];
  }
  else if (sensorValue[1] == 0xC5) //Response to command
  {
    Serial.println();
    Serial.println("Response to command found");

    if (sensorValue[2] == 7) //Firmware response
    {
      Serial.print("Firmware version Y/M/D: ");
      Serial.print(sensorValue[3]);
      Serial.print("/");
      Serial.print(sensorValue[4]);
      Serial.print("/");
      Serial.print(sensorValue[5]);
      Serial.println();
    }
    else if (sensorValue[2] == 6) //Query/Set work and sleep modes
    {
      if (sensorValue[3] == 1) //Response to set mode
      {
        Serial.print("Sensor is going to ");
        if (sensorValue[4] == 0) Serial.println("sleep");
        else if (sensorValue[4] == 1) Serial.println("work");
      }
    }

    Serial.println();
  }

  Serial.print("Raw data:");
  for (int x = 1 ; x < 10 ; x++)
  {
    Serial.print(" ");
    Serial.print(x);
    Serial.print(":0x");
    Serial.print(sensorValue[x], HEX);
  }
  Serial.println();

  return (true); //We've got a good reading!
}

//Send a command packet to the module
//Requires the command number and two setting bytes
//Calculates CRC and attaches all header/ender bytes
//Assumes you are only talking to one sensor
void sendCommand(byte commandNumber, byte dataByte2, byte dataByte3)
{
  byte packet[19]; //It's 19 bytes big
  packet[0] = 0xAA; //Message header
  packet[1] = 0xB4; //Packet type = Command
  packet[2] = commandNumber; //Type of command we want to do
  packet[3] = dataByte2; //These are specific to each command
  packet[4] = dataByte3;

  for (byte x = 5; x < 15 ; x++)
    packet[x] = 0; //Reserved bytes

  packet[15] = 0xFF; //Talk to whatever sensor we are connected to. No specific device ID.
  packet[16] = 0xFF; //Talk to whatever sensor we are connected to. No specific device ID.

  //packet[15] = 0xA4; //Talk to specific sensor
  //packet[16] = 0xE6; //Talk to specific sensor

  //Caculate CRC
  byte crc = 0;
  for (byte x = 2 ; x < 17 ; x++)
    crc += packet[x];

  packet[17] = crc;
  packet[18] = 0xAB; //Tail

  //Display the contents of the command packet for debugging
  /*Serial.print("Command packet:");
    for(int x = 0 ; x < 19 ; x++)
    {
    Serial.print(" ");
    Serial.print(x);
    Serial.print(":0x");
    Serial.print(packet[x], HEX);
    }
    Serial.println();*/

  //The sensor seems to fail to respond to the first 2 or 3 times we send a command
  //Hardware serial doesn't have this issue but software serial does.
  //Sending 10 throw away characters at it gets the units talking correctly
  for (byte x = 0 ; x < 20 ; x++)
    particleSensor.write('!'); //Just get the software serial working

  //Send command packet
  for (byte x = 0 ; x < 19 ; x++)
    particleSensor.write(packet[x]);

  //Now look for response
  dataAvailable();

}
