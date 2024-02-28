#include <Arduino.h>

#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "I2S.h"

AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;
int cs = 4;



void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.print("Initializing SD card...");

  if (!SD.begin(cs))
  {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  
  Serial.printf("WAV start\n");
  audioLogger = &Serial;
  file = new AudioFileSourceSD("annekaffekanne.wav");
  out = new AudioOutputI2S();
  wav = new AudioGeneratorWAV();
  out->SetGain(0.3f);
  wav->begin(file, out);
  
}

void loop()
{
  
  if (wav->isRunning()) {
    if (!wav->loop()) wav->stop();
  } else {
    Serial.printf("WAV done\n");
    delay(1000);
  }
  
}

