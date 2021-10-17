#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"
#include "Nextion.h"

// Update these with values suitable for your hardware/network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xEE };
IPAddress server(192, 168, 1, 171);

// WiFi card example
char ssid[] = WSSID;    // your SSID
char pass[] = WPSWD;       // your SSID Password

int gblWindDir = 0;
float gblWindSpeed = 0.00;
float gblHumidity = 0.00;
float gblTempF = 0.00;
float gblRainIn = 0.00;
float gblDRainIn = 0.00;
float gblBattLvl = 0.00;
float gblLightLvl = 0.00;


NexGauge WindDir  = NexGauge(0, 15, "z0");
NexText WindSpeed = NexText(0, 20, "t16");
NexText Humidity = NexText(0, 17, "t7");
NexText TempF = NexText(0, 16, "t4");
NexText RainIn = NexText(0, 18, "t9");
NexText DRainIn = NexText(0, 19, "t12");
NexText BattLvl = NexText(0, 22, "t20");
NexText LightLvl = NexText(0, 21, "t19");


int moduloWindDir(int intBearing) {
  if (intBearing > 359) { return(intBearing - 360); }
  else {return(intBearing);}
}

void callback(char* topic, byte* payload, unsigned int length) {
  String tmpTopic = topic;
  char tmpStr[length+1];
  for (int x=0; x<length; x++) {
    tmpStr[x] = (char)payload[x]; // payload is a stream, so we need to chop it by length.
  }
  tmpStr[length] = 0x00; // terminate the char string with a null
  
  if (tmpTopic == "weather/winddir") {int intWindDir = atoi(tmpStr); gblWindDir = (moduloWindDir(intWindDir + 90)); }
  else if (tmpTopic == "weather/windspeedmph") { gblWindSpeed = atof(tmpStr); }
  else if (tmpTopic == "weather/humidity") { gblHumidity = atof(tmpStr); }
  else if (tmpTopic == "weather/tempf") { gblTempF = atof(tmpStr); }
  else if (tmpTopic == "weather/rainin") { gblRainIn = atof(tmpStr); }
  else if (tmpTopic == "weather/dailyrainin") { gblDRainIn = atof(tmpStr); }
  else if (tmpTopic == "weather/batt_lvl") { gblBattLvl = atof(tmpStr); }
  //else if (tmpTopic == "weather/altitude") { gblAltitude = atoi(tmpStr); }
  else if (tmpTopic == "weather/light_lvl") { gblLightLvl = atof(tmpStr); }
  /*else if (tmpTopic == "weather/lat") { gblGpsLat = tmpStr; }
  else if (tmpTopic == "weather/lng") { gblGpsLong = tmpStr; }
  else if (tmpTopic == "weather/sats") { gblGpsSats = tmpStr; }
  else if (tmpTopic == "weather/date") { gblGpsDate = tmpStr; }
  else if (tmpTopic == "weather/time") { gblGpsTime = tmpStr; }

  else if (tmpTopic == "generator/Status/Engine/Engine_State") { gblGenEngState = tmpStr; }
  else if (tmpTopic == "generator/Status/Engine/Switch_State") { gblGenEngSwitch = tmpStr; }
  else if (tmpTopic == "generator/Status/Engine/Battery_Voltage") { gblGenEngBatt = tmpStr; } */

}

void updateDisp() {
  char buffer[10];
  memset(buffer, 0, sizeof(buffer));

  
  WindDir.setValue(gblWindDir);

  dtostrf(gblWindSpeed,5, 2, buffer);
  WindSpeed.setText(buffer);
  
  dtostrf(gblHumidity,5, 2, buffer);
  Humidity.setText(buffer);
  
  dtostrf(gblTempF,5, 2, buffer);
  TempF.setText(buffer);
  
  dtostrf(gblRainIn,5, 2, buffer);
  RainIn.setText(buffer);
  
  dtostrf(gblDRainIn,5, 2, buffer);
  DRainIn.setText(buffer);
  
  dtostrf(gblBattLvl,5, 2, buffer);
  BattLvl.setText(buffer);
  
  dtostrf(gblLightLvl,5, 2, buffer);
  LightLvl.setText(buffer);

}

WiFiClient wClient;
PubSubClient client(wClient);

long lastReconnectAttempt = 0;
unsigned long lastDisplay = 0UL;


boolean reconnect() {
  if (client.connect("arduinoClient2")) {
    client.subscribe("weather/winddir");
    client.subscribe("weather/windspeedmph");
    client.subscribe("weather/humidity");
    client.subscribe("weather/tempf");
    client.subscribe("weather/rainin");
    client.subscribe("weather/dailyrainin");
    client.subscribe("weather/batt_lvl");
    client.subscribe("weather/altitude");
    client.subscribe("weather/light_lvl");
    client.subscribe("weather/lat");
    client.subscribe("weather/lng");
    client.subscribe("weather/sats");
    client.subscribe("weather/date");
    client.subscribe("weather/time");

    client.subscribe("generator/Status/Engine/Engine_State");
    client.subscribe("generator/Status/Engine/Switch_State");
    client.subscribe("generator/Status/Engine/Battery_Voltage");


  }
  return client.connected();
}

void setup()
{
  Serial.begin (9600);

  client.setServer(server, 1883);
  client.setCallback(callback);

  nexInit();

  //WiFi.setPins(53,48,49);
  int status = WiFi.begin(ssid, pass);
  if ( status != WL_CONNECTED) {
    Serial.println("Couldn't get a wifi connection");
    while(true);
  }
  // print out info about the connection:
  else {
    Serial.println("Connected to network");
    IPAddress ip = WiFi.localIP();
    Serial.print("My IP address is: ");
    Serial.println(ip);
  }
  delay(1500);
  lastReconnectAttempt = 0;

  Serial.println("Weather Display online!");

}

void loop()
{
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected

    client.loop();
  }
  if (millis() - lastDisplay > 500) {
    lastDisplay = millis();
    updateDisp();
  }
}
