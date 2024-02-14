
String myAPIkey = "ORL6VB0LONBY7CN1";
#include"dht.h"
#include <SoftwareSerial.h>
SoftwareSerial ESP8266(10, 11); // Rx,  Tx
#include <LiquidCrystal595.h>       // include the library
 LiquidCrystal595 lcd(4, 3, 2);  // datapin, latchpin, clockpin
 
#define dht_dpin A0
dht DHT;
int humi, tem;

long writingTimer = 20; 
long startTime = 0;
long waitTime = 0;

unsigned char check_connection=0;
unsigned char times_check=0;
boolean error;

//***********************************************************************************
void setup()
{
  Serial.begin(9600); 
  ESP8266.begin(115200); 
  lcd.begin(16, 2);  lcd.clear();
  lcd.home (); lcd.setLED2Pin(HIGH);
  
  startTime = millis(); 
  ESP8266.println("AT+RST");
  delay(2000);
  Serial.println("Connecting to Wifi");
  while(check_connection==0)
  {
   Serial.print(".");
   ESP8266.print("AT+CWJAP=\"vivo 1938\",\"divya2001\"\r\n");
   ESP8266.setTimeout(5000);
   if(ESP8266.find("WIFI CONNECTED\r\n")==1)
    {
     Serial.println("WIFI CONNECTED");
     break;
    }
   times_check++;
    if(times_check>3) 
    {
     times_check=0;
     Serial.println("Trying to Reconnect..");
    }
  }
}

//***********************************************************************************
void loop()
{
  waitTime = millis()-startTime;   
  if (waitTime > (writingTimer*1000)) 
  {
    readSensors();
    writeThingSpeak();
    startTime = millis();   
  }
}

//***********************************************************************************
void readSensors(void)
{
  DHT.read11(dht_dpin);
  humi = DHT.humidity;
  tem = DHT.temperature;
  lcd.setCursor(0, 0);  lcd.print("Humidity:");
  lcd.print(humi);  lcd.print(" %   ");
  lcd.setCursor(0, 1);  lcd.print("Temperature:");
  lcd.print(tem); lcd.write(39);  lcd.print("C ");
}
//**************************************************************************************************************************



//***********************************************************************************
void writeThingSpeak(void)
{
  startThingSpeakCmd();
  // preparacao da string GET
  String getStr = "GET /update?api_key=";
  getStr += myAPIkey;
  getStr +="&field1=";
  getStr += String(tem);
  getStr +="&field2=";
  getStr += String(humi);
  getStr += "\r\n\r\n";
  GetThingspeakcmd(getStr); 
}

//***********************************************************************************
void startThingSpeakCmd(void)
{
  ESP8266.flush();
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com IP address
  cmd += "\",80";
  ESP8266.println(cmd);
  Serial.print("Start Commands: ");
  Serial.println(cmd);

  if(ESP8266.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
}

//***********************************************************************************
String GetThingspeakcmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ESP8266.println(cmd);
  Serial.println(cmd);

  if(ESP8266.find(">"))
  {
    ESP8266.print(getStr);
    Serial.println(getStr);
    delay(500);
    String messageBody = "";
    while (ESP8266.available()) 
    {
      String line = ESP8266.readStringUntil('\n');
      if (line.length() == 1) 
      { 
        messageBody = ESP8266.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    ESP8266.println("AT+CIPCLOSE");     
    Serial.println("AT+CIPCLOSE"); 
  } 
}
//***********************************************************************************
//***********************************************************************************
