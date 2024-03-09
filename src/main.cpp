/**
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from more than one PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 *
 * Example sketch/program showing how to read data from more than one PICC (that is: a RFID Tag or Card) using a
 * MFRC522 based RFID Reader on the Arduino SPI interface.
 *
 * Warning: This may not work! Multiple devices at one SPI are difficult and cause many trouble!! Engineering skill
 *          and knowledge are required!
 *
 * @license Released into the public domain.
 *
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino      ESP8266
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST          D3-GPIO 0
 * SPI SS 1    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **          D1-GPIO 5
 * SPI SS 2    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16           D7-GPIO 13
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14           D6-GPIO 12
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15           D5-GPIO 14
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 *
 */

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Regexp.h>

#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include "I2S.h"
//#include "AudioFileSourceBuffer.h"

// Sound Setup with I2s and audio CS PIN
AudioGeneratorWAV *wav;
AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;
int cs = 5;
//AudioFileSourceBuffer *buff;

// D3 - DIN, BCRL - D8, LRC - D4
// D3 = GPIO 0, D8 = GPIO 15, D4 = GIPO 2
// SD CS D1, GIPO 5

// RFID PINS
#define RST_PIN 4 // Configurable, see typical pin layout above
#define SS_PIN 16
unsigned long plannedNextScan; // used for timed PICC scanning
float timeToNextScan = 1000;   // time in milliseconds
byte *currentUID;
byte *currentUIDlength;
const String testUID = "04 D1 32 01 17 48 03";

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
const char *songs_0_WAV[] = {"bibabutzemann.wav", "auf_der_mauer.wav", "tante_marokko.wav", "vogelhochzeit.wav", "annekaffekanne.wav", NULL};
const char *songs_1_MP3[] = {"dontgo.mp3", "pinguin.mp3", "dumbbell.mp3", "mrbluesky.mp3", "crabrave.mp3",
							 "good4you.mp3", "bohemian.mp3", "moorhexe.mp3", "hedwig.mp3", "moveitmoveit.mp3",
							 "faint.mp3", "wannabela.mp3", "omnissiah.mp3", "vampire.mp3", NULL};
int iteration = 0;
bool notIncremented = true;
bool isFirstIteration = true;

// regex
MatchState ms;
char isWav;
char isMP3;
char target[100];

void setup()
{

	Serial.begin(115200); // Initialize serial communications with the PC
	SPI.begin();					   // Init SPI bus
	mfrc522.PCD_Init();				   // Init MFRC522
	delay(4);						   // Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details

	Serial.print("Initializing SD card...");

	if (!SD.begin(cs))
	{
		Serial.println("initialization failed!");
		return;
	}
	Serial.println("initialization done.");

	audioLogger = &Serial;
	out = new AudioOutputI2S();
	wav = new AudioGeneratorWAV();
	mp3 = new AudioGeneratorMP3();
	//buff = new AudioFileSourceBuffer(file, 2048);
	out->SetGain(0.025f);
	randomSeed(analogRead(0));
}

size_t getArraySize(const char *array[])
{
	size_t size = 0;
	while (array[size] != NULL)
	{
		size++;
	}
	return size;
}

int randomNextSongIndex(const char *songarray[])
{
	size_t size = getArraySize(songarray);
	long randomNum = random(0, size);
	return (int)randomNum;
}

int subsequentIndex(const char *songarray[])
{
	int num = iteration + 1;
	int getArrayLength = getArraySize(songarray);
	num = num % getArrayLength;
	return num;
}

void changeSong(const char *songarray[], int index)
{
	Serial.println("index: " + index);
	const char *filename = songarray[index];

	strcpy(target, filename);
	ms.Target(target); // set its address
	isWav = ms.Match(".wav");
	isMP3 = ms.Match(".mp3");
	if (isWav > 0)
	{
		Serial.printf("start wav\n");
		file = new AudioFileSourceSD(filename);
		//buff = new AudioFileSourceBuffer(file, 2048);
		wav->begin(file, out); // regex found the .wav
	}
	else if (isMP3 > 0)
	{
		Serial.printf("start mp3\n");
		file = new AudioFileSourceSD(filename);
		//buff = new AudioFileSourceBuffer(file, 2048);
		mp3->begin(file, out); // regex found the .mp3
	}
	else
		Serial.printf("invalid filetype!");
	isFirstIteration = false;
}

void scanForCardAndChangeSong()
{
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if (!mfrc522.PICC_IsNewCardPresent())
	{
		return;
	}
	else
	{

		// Select one of the cards
		if (!mfrc522.PICC_ReadCardSerial())
		{
			return;
		}

		// Dump debug info about the card; PICC_HaltA() is automatically called
		mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
		iteration = randomNextSongIndex(songs_1_MP3);
		changeSong(songs_1_MP3, iteration);
	}
}

void loop()
{
	// return;

	if (mp3->isRunning())
	{
		if (!mp3->loop())
			mp3->stop();
		notIncremented = true;
		unsigned long millisSinceStart = millis();
		if (millisSinceStart > plannedNextScan)
		{
			scanForCardAndChangeSong();
			Serial.printf("scanned for card\n");
			plannedNextScan = millisSinceStart + timeToNextScan;
		}
		return;
	}
	else if (!mp3->isRunning() && notIncremented && !isFirstIteration)
	{
		Serial.printf("done\n");
		iteration = randomNextSongIndex(songs_1_MP3);
		notIncremented = false;
		delay(1000);
	}
	scanForCardAndChangeSong();
}
