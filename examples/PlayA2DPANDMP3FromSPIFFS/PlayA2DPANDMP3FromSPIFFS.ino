#include <Arduino.h>

#define USE_TAS5825M

#ifdef USE_TAS5825M
#include <TAS5825M.h>
#endif

#include <Wire.h>
#ifdef ESP32
#include <WiFi.h>
#include "SPIFFS.h"
#else
#include <ESP8266WiFi.h>
#endif

#include "AudioGeneratorA2DP.h"
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "AudioOutputMixer.h"

AudioGeneratorWAV *mp3;
AudioFileSourceSPIFFS *file;
AudioGeneratorA2DP *ad2p;
AudioOutputI2S *out;
#ifdef USE_TAS5825M
TAS5825M tas5825m;
#endif
AudioOutputMixer *mixer;
AudioOutputMixerStub *stub[2];

void onConnectionChanged(){
  file = new AudioFileSourceSPIFFS("/change.wav");
  mp3->begin(file, stub[0]);
}
void setup()
{
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);
  Wire.begin();
  delay(1000);
  SPIFFS.begin();

  audioLogger = &Serial;
  out = new AudioOutputI2S();
  out->SetPinout(26, 25, 12);
  mixer = new AudioOutputMixer(2048, out);

  #ifdef USE_TAS5825M
  tas5825m.begin();

  Serial.println("Reading");
  readMyReg(0x39);
  readMyReg(0x68);
  readMyReg(0x69);
  readMyReg(0x70);
  readMyReg(0x71);
  readMyReg(0x72);
  readMyReg(0x73);
  readMyReg(0x28);
  readMyReg(0x37);
  readMyReg(0x38);
  readMyReg(0x5e);
  readMyReg(0x67);
  #endif

  stub[0] = mixer->NewInput();
  stub[0]->SetGain(1.0);
  stub[1] = mixer->NewInput();
  stub[1]->SetGain(1.0);

  mp3 = new AudioGeneratorWAV();
  file = new AudioFileSourceSPIFFS("/boot.wav");
  mp3->begin(file, stub[0]);

  ad2p = new AudioGeneratorA2DP();
  ad2p->set_on_connection_changed(onConnectionChanged);
  ad2p->begin(NULL, stub[1]);

  Serial.println(ESP.getFreeHeap());
}
void readMyReg(uint8_t reg) {
  Wire.beginTransmission(0x4c);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(0x4c, 1);
  Serial.println(Wire.read());
}
void loop()
{
  if (mp3->isRunning()) {
    if (!mp3->loop()) {
      mp3->stop();
      stub[0]->stop();
      Serial.printf("stopping mp3 file\n");
    }
  }

  if (ad2p->isRunning()) {
    if (!ad2p->loop()) {
      ad2p->stop();
      stub[1]->stop();
      Serial.printf("stopping ad2p\n");
    }
  }

}
