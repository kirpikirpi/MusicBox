#include <Arduino.h>

#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "I2S.h"

AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;
int cs = 4;

const char *songs_1_MP3[] = {"dontgo.mp3", "pinguin.mp3", "dumbbell.mp3", "mrbluesky.mp3", "crabrave.mp3",
							 "good4you.mp3", "bohemian.mp3", "moorhexe.mp3", "hedwig.mp3", "moveitmoveit.mp3",
							 "faint.mp3", "wannabela.mp3", "omnissiah.mp3", "vampire.mp3", "paintitblack.mp3",
							 "surf.mp3", "sympathyforthedevil.mp3", "turntostone.mp3", "dontstopmenow.mp3",
							 "everybodyhides.mp3", "help.mp3", "honky.mp3", "hound.mp3", "hound.mp3", "igetaround.mp3",
							 "isawher.mp3", "jailhouse.mp3", "killerqueen.mp3", "bottom2.mp3", NULL};

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

