/**
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
 */

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "I2S.h"
#include "AudioFileSourceBuffer.h"
#include <RFIDLogic.h>

char sa[] = "\"A.wav\"";
int ka[10] = {4,193,93,1,109,72,3,0,0,0};
KeyValue A = {sa, ka};

char sb[] = "\"B.wav\"";
int kb[10] = {4,209,32,1,126,72,3,0,0,0};
KeyValue B = {sb, kb};

char sc[] = "\"C.wav\"";
int kc[10] = {4,144,255,1,61,77,3,0,0,0};
KeyValue C = {sc, kc};

char sd[] = "\"D\"";
int kd[10] = {4,209,50,1,254,72,3,0,0,0};
KeyValue D = {sd, kd};

char se[] = "\"E.wav\"";
int ke[10] = {4,193,178,1,89,72,3,0,0,0};
KeyValue E = {se, ke};

char shorse[] = "\"annekaffekanne.wav\"";
int khorse[10] = {4,209,50,1,23,72,3,0,0,0};
KeyValue Horse = {shorse, khorse};

KeyValue songArray[] = {A,B,C,D,E,Horse};
int songArrayLen = 6;

// Sound Setup with I2s and audio CS PIN
AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;
int cs = 5;
AudioFileSourceBuffer *buff;

// D3 - DIN, BCRL - D8, LRC - D4
// D3 = GPIO 0, D8 = GPIO 15, D4 = GIPO 2
// SD CS D1, GIPO 5

// RFID PINS
#define RST_PIN 4 // Configurable, see typical pin layout above
#define SS_PIN 16
unsigned long plannedNextScan; // used for timed PICC scanning
float timeToNextScan = 1000;   // time in milliseconds
int currentSongUIDsum = 0;


MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

int iteration = 0;
bool notIncremented = true;
bool isFirstIteration = true;

void setup()
{

	Serial.begin(115200);			   // Initialize serial communications with the PC
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
	buff = new AudioFileSourceBuffer(file, 2048);
	out->SetGain(0.3f);
	randomSeed(analogRead(0));
}

//Scans for a new RFID card, returns the file name if UID matches, 
//returns null pointer if there is no match. in this case the return type int of the function returns -1.
int scanForCard(char* returnFileName)
{
	if (!mfrc522.PICC_IsNewCardPresent()) return -1;
	else{
		if (!mfrc522.PICC_ReadCardSerial())	return -1;
		getSongNameFromTag(mfrc522.uid.uidByte, songArray, songArrayLen, returnFileName);
		mfrc522.PICC_HaltA();
	}
	return 1;
}

void startPlayback(const char *filename)
{
	if (wav->isRunning())
		wav->stop();
	file = new AudioFileSourceSD(filename);
	buff = new AudioFileSourceBuffer(file, 2048);
	wav->begin(buff, out);
}

void loop()
{
	char songName[100];
	memset(songName,0,sizeof(char)*100);
	char* returnFileName = songName;
	int cardFound = scanForCard(returnFileName);
	if(cardFound>0) printf("song from UID: %s\n", returnFileName);
	return;
	/*
	if (wav->isRunning())
	{
		if (!wav->loop())
			wav->stop();
		unsigned long millisSinceStart = millis();
		if (millisSinceStart > plannedNextScan)
		{
			int isNewCard = scanForCard();
			if (isNewCard == cardHorse && isNewCard != currentSongUIDsum)
			{
				startPlayback(horseSong);
				currentSongUIDsum = isNewCard;
			}
			plannedNextScan = millisSinceStart + timeToNextScan;
		}
		return;
	}
	else if (!wav->isRunning() && !isFirstIteration)
	{
		Serial.printf("done\n");
		currentSongUIDsum = 0;
		delay(1000);
	}
	int isNewCard = scanForCard();
	if (isNewCard == cardHorse)
	{
		startPlayback(horseSong);
		currentSongUIDsum = isNewCard;
	}
	*/
}
