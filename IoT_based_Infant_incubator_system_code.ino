/*
=====================================================
Project: IoT Based Infant Incubator Monitoring System
Description:
- Monitors Temperature, Humidity, Pulse & Gas levels
- Displays on LCD
- Sends data to ThingSpeak via ESP8266
=====================================================
*/

// ---------------- LIBRARIES ----------------
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <dht11.h>

// ---------------- LCD SETUP ----------------
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// ---------------- PIN DEFINITIONS ----------------
#define smoke A1
#define RX 9
#define TX 10
#define dht_apin 11   // DHT11 sensor pin

// ---------------- SENSOR OBJECT ----------------
dht11 dhtObject;

// ---------------- VARIABLES ----------------
int Signal;
int const PULSE_SENSOR_PIN = 0;
int Threshold = 510;

// ---------------- WIFI CONFIG ----------------
String AP = "elegant";       // WiFi Name
String PASS = "smartwork";   // WiFi Password
String API = "A6XK08TCIOPG5CLE";   // ThingSpeak API Key
String HOST = "api.thingspeak.com";
String PORT = "80";

// ---------------- CONTROL VARIABLES ----------------
int countTrueCommand;
int countTimeCommand;
boolean found = false;
int valSensor = 1;

// ---------------- ESP8266 SERIAL ----------------
SoftwareSerial esp8266(RX, TX);

// ---------------- SETUP ----------------
void setup() 
{
  Serial.begin(9600);
  esp8266.begin(115200);

  pinMode(smoke, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  lcd.begin(16, 2);
  lcd.clear();

  // Welcome Display
  lcd.setCursor(4, 0);
  lcd.clear();
  lcd.print("WELCOME");
  lcd.print("INFANT INCUBATOR");
  lcd.setCursor(0, 1);
  lcd.print("MONITORING IoT");

  // ESP8266 Initialization
  sendCommand("AT", 5, "OK");
  sendCommand("AT+CWMODE=1", 5, "OK");
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");

  delay(1000);
}

// ---------------- MAIN LOOP ----------------
void loop() 
{
  String getData = "GET /update?api_key=" + API
                   + "&field1=" + getTemperatureValue()
                   + "&field2=" + getHumidityValue()
                   + "&field3=" + getsmokeValue()
                   + "&field4=" + getpluse();

  sendCommand("AT+CIPMUX=1", 5, "OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
  sendCommand("AT+CIPSEND=0," + String(getData.length() + 4), 4, ">");

  esp8266.println(getData);
  delay(1500);

  countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0", 5, "OK");
}

// ---------------- TEMPERATURE FUNCTION ----------------
String getTemperatureValue() 
{
  dhtObject.read(dht_apin);

  Serial.print("Temperature(C)= ");

  lcd.setCursor(0, 1);
  lcd.print("Temperature=");

  float temp = dhtObject.temperature;

  Serial.println(temp);
  lcd.print(temp);

  delay(500);

  return String(temp);
}

// ---------------- HUMIDITY FUNCTION ----------------
String getHumidityValue() 
{
  dhtObject.read(dht_apin);

  Serial.print(" Humidity in %=");

  lcd.clear();
  lcd.print("Humidity%=");

  float humidity = dhtObject.humidity;

  Serial.println(humidity);
  lcd.print(humidity);

  delay(500);

  return String(humidity);
}

// ---------------- PULSE FUNCTION ----------------
String getpluse()  
{
  lcd.clear();

  Signal = analogRead(PULSE_SENSOR_PIN);

  int pulse;

  Serial.println(Signal);

  lcd.print("pluse rate:");
  lcd.print(Signal);

  lcd.setCursor(0, 1);
  lcd.print("PULSE:");
  lcd.print(Signal - 445);

  pulse = Signal - 445;

  if (Signal > Threshold)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
  }

  delay(1000);

  return String(pulse);
}

// ---------------- ESP8266 COMMAND FUNCTION ----------------
void sendCommand(String command, int maxTime, char readReplay[]) 
{
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");

  while (countTimeCommand < (maxTime * 1))
  {
    esp8266.println(command);

    if (esp8266.find(readReplay))
    {
      found = true;
      break;
    }

    countTimeCommand++;
  }

  if (found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
}

// ---------------- SMOKE SENSOR FUNCTION ----------------
String getsmokeValue() 
{
  float adcValue = 0;

  for (int i = 0; i < 10; i++)
  {
    adcValue += analogRead(smoke);
    delay(10);
  }

  float v = (adcValue / 10) * (5.0 / 1024.0);
  float mgL = 0.67 * v;

  Serial.print("smoke:");
  Serial.print(mgL);
  Serial.print(" mg/L");

  lcd.setCursor(0, 0);
  lcd.print("GAS: ");
  lcd.print(mgL, 4);
  lcd.print(" g/g        ");

  delay(2000);

  return String(mgL);
}