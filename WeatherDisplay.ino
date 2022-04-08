#include <RHReliableDatagram.h>
#include <RH_RF69.h>
#include <SPI.h>
#include "Nextion.h"


int gblWindDir = 0;
float gblWindSpeed = 0.00;
float gblHumidity = 0.00;
float gblTempF = 0.00;
float gblRainIn = 0.00;
float gblDRainIn = 0.00;
float gblBattLvl = 0.00;
float gblLightLvl = 0.00;

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Singleton instance of the radio driver
RH_RF69 driver(23, 18);
//RH_RF69 driver(15, 16); // For RF69 on PJRC breakout board with Teensy 3.1
//RH_RF69 rf69(4, 2); // For MoteinoMEGA https://lowpowerlab.com/shop/moteinomega
//RH_RF69 driver(8, 7); // Adafruit Feather 32u4

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);


NexGauge WindDir  = NexGauge(0, 15, "z0");
NexText WindSpeed = NexText(0, 20, "t16");
NexText Humidity = NexText(0, 17, "t7");
NexText TempF = NexText(0, 16, "t4");
NexText RainIn = NexText(0, 18, "t9");
NexText DRainIn = NexText(0, 19, "t12");
NexText BattLvl = NexText(0, 22, "t20");
NexText LightLvl = NexText(0, 21, "t19");
NexWaveform WindSpeedGraph = NexWaveform(0, 23, "s0");

int moduloWindDir(int intBearing) {
  if (intBearing > 359) { return(intBearing - 360); }
  else {return(intBearing);}
}

void callback(char* payload) {
  String tmpBuff = (char*) payload;
  String tmpTopic = tmpBuff.substring(0,1);
  String tmpStr = tmpBuff.substring(1);

  if (tmpTopic == "0") {int intWindDir = tmpStr.toInt(); gblWindDir = (moduloWindDir(intWindDir + 90)); }
  else if (tmpTopic == "1") { gblWindSpeed = tmpStr.toFloat(); }
  else if (tmpTopic == "2") { gblHumidity = tmpStr.toFloat(); }
  else if (tmpTopic == "3") { gblTempF = tmpStr.toFloat(); }
  else if (tmpTopic == "4") { gblRainIn = tmpStr.toFloat(); }
  else if (tmpTopic == "5") { gblDRainIn = tmpStr.toFloat(); }
  else if (tmpTopic == "6") { gblBattLvl = tmpStr.toFloat(); }


}

void updateDisp() {
  char buffer[10];
  memset(buffer, 0, sizeof(buffer));


  WindDir.setValue(gblWindDir);

  dtostrf(gblWindSpeed,5, 2, buffer);
  WindSpeed.setText(buffer);
  WindSpeedGraph.addValue(0, gblWindSpeed);

  dtostrf(gblHumidity,5, 2, buffer);
  Humidity.setText(buffer);
  WindSpeedGraph.addValue(1, gblHumidity);

  dtostrf(gblTempF,5, 2, buffer);
  TempF.setText(buffer);
  WindSpeedGraph.addValue(2, gblTempF);

  dtostrf(gblRainIn,5, 2, buffer);
  RainIn.setText(buffer);
  WindSpeedGraph.addValue(3, gblRainIn);

  dtostrf(gblDRainIn,5, 2, buffer);
  DRainIn.setText(buffer);

  dtostrf(gblBattLvl,5, 2, buffer);
  BattLvl.setText(buffer);

  dtostrf(gblLightLvl,5, 2, buffer);
  LightLvl.setText(buffer);

}

unsigned long lastDisplay = 0UL;


void setup()
{
  Serial.begin (9600);



  nexInit();

  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  driver.setTxPower(14, true);

  delay(1500);


  Serial.println("Weather Display online!");

}

uint8_t data[] = "ok";
// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

void loop()
{
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      callback((char*)buf);


      // Send a reply back to the originator client
      if (!manager.sendtoWait(data, sizeof(data), from))
        Serial.println("sendtoWait failed");
    }
  }
  if (millis() - lastDisplay > 500) {
    lastDisplay = millis();
    updateDisp();
  }
}
