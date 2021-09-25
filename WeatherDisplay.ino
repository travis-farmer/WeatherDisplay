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



NexGauge WindDir  = NexGauge(0, 1, "z0");
NexText WindSpeed = NexText(0, 8, "t4");
NexText Humidity = NexText(0, 12, "t9");
NexText TempF = NexText(0, 9, "t7");
NexText RainIn = NexText(0, 14, "t12");
NexText DRainIn = NexText(0, 18, "t16");
NexText BattLvl = NexText(0, 21, "t19");
NexText Altitude = NexText(0, 25, "t23");
NexText LightLvl = NexText(0, 32, "t30");
NexText GpsLat = NexText(0, 28, "t26");
NexText GpsLong = NexText(0, 30, "t28");
NexText GpsSats = NexText(0, 31, "t29");
NexText GpsDate = NexText(0, 22, "t20");
NexText GpsTime = NexText(0, 23, "t21");

NexText genEngState = NexText(0, 36, "t34");
NexText genEngSwitch = NexText(0, 38, "t36");
NexText genEngBatt  = NexText(0, 34, "t32");

int moduloWindDir(int intBearing) {
  if (intBearing > 359) return(intBearing - 360);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String tmpTopic = topic;
  char tmpStr[length];
  for (int x=0; x<length; x++) {
    tmpStr[x] = (char)payload[x]; // payload is a stream, so we need to chop it by length.
  }

  if (tmpTopic == "weather/winddir") {int intWindDir = atoi(tmpStr); WindDir.setValue(moduloWindDir(intWindDir + 90)); }
  else if (tmpTopic == "weather/windspeedmph") { WindSpeed.setText(tmpStr); }
  else if (tmpTopic == "weather/humidity") { Humidity.setText(tmpStr); }
  else if (tmpTopic == "weather/tempf") { TempF.setText(tmpStr); }
  else if (tmpTopic == "weather/rainin") { RainIn.setText(tmpStr); }
  else if (tmpTopic == "weather/dailyrainin") { DRainIn.setText(tmpStr); }
  else if (tmpTopic == "weather/batt_lvl") { BattLvl.setText(tmpStr); }
  else if (tmpTopic == "weather/altitude") { Altitude.setText(tmpStr); }
  else if (tmpTopic == "weather/light_lvl") { LightLvl.setText(tmpStr); }
  else if (tmpTopic == "weather/lat") { GpsLat.setText(tmpStr); }
  else if (tmpTopic == "weather/lng") { GpsLong.setText(tmpStr); }
  else if (tmpTopic == "weather/sats") { GpsSats.setText(tmpStr); }
  else if (tmpTopic == "weather/date") { GpsDate.setText(tmpStr); }
  else if (tmpTopic == "weather/time") { GpsTime.setText(tmpStr); }

  else if (tmpTopic == "generator/Status/Engine/Engine_State") { genEngState.setText(tmpStr); }
  else if (tmpTopic == "generator/Status/Engine/Switch_State") { genEngSwitch.setText(tmpStr); }
  else if (tmpTopic == "generator/Status/Engine/Battery_Voltage") { genEngBatt.setText(tmpStr); }

}

WiFiClient wClient;
PubSubClient client(wClient);

long lastReconnectAttempt = 0;



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

  WiFi.setPins(53,48,49);
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
}
