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

//KEY VALUE PAIRS OF UID AND CORRESPONDING SONGS///////////////////
char sa[] = {"BeatlesStandingThere.wav"};
int ka[10] = {4,193,93,1,109,72,3,0,0,0};
KeyValue A = {sa, ka};

char sb[] = "tante_marokko.wav";
int kb[10] = {4,209,32,1,126,72,3,0,0,0};
KeyValue B = {sb, kb};

char sc[] = "PenguenJudoka.wav";
int kc[10] = {4,144,255,1,61,77,3,0,0,0};
KeyValue C = {sc, kc};

char sd[] = "Faint.wav";
int kd[10] = {4,209,50,1,254,72,3,0,0,0};
KeyValue D = {sd, kd};

char sElk[] = "bieberfieber.wav";
int kElk[10] = {4,193,121,1,65,72,3,0,0,0};
KeyValue E = {sElk, kElk};

char shorse[] = "annekaffekanne.wav";
int khorse[10] = {4,209,50,1,23,72,3,0,0,0};
KeyValue Horse = {shorse, khorse};

char sF[] = "SympathyForTheDevil.wav";
int kF[10] = {4,193,135,1,214,72,3,0,0,0};
KeyValue Bear = {sF, kF};

char sH[] = "Moorhexe.wav";
int kH[] = {4,209,52,1,109,72,3,0,0,0};
KeyValue H = {sH,kH};

char sG[] = "DontGo.wav";
int kG[] = {4,193,129,1,124,72,3,0,0,0};
KeyValue G = {sG, kG};

char sMushroom[] = "CrabRave.wav";
int kMushroom[] = {4,209,17,1,211,72,3,0,0,0};
KeyValue Mushroom = {sMushroom, kMushroom};

char sBlueHouse[] = "Help.wav";
int kBlueHouse[] = {4,193,92,1,74,72,3,0,0,0};
KeyValue bHouse = {sBlueHouse, kBlueHouse};

char sZylinder[] = "DontStopMeNow.wav";
int kZylinder[] = {4,209,27,1,187,72,3,0,0,0};
KeyValue zylinder = {sZylinder, kZylinder};

char sZylinder2[] = "BohemianRhapsody.wav";
int kZylinder2[] = {4,193,169,1,220,72,3,0,0,0};
KeyValue zylinder2 = {sZylinder2, kZylinder2};

KeyValue songArray[] = {A,B,C,D,E,Horse,Bear,H,G,Mushroom,bHouse,zylinder,zylinder2};
int songArrayLen = 13;
///////////////END OF KEY VALUE PAIRS.////////////////////////////

// Sound Setup with I2s and audio CS PIN
AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;
int cs = 5;
AudioFileSourceBuffer *buff;

// RFID PINS
#define RST_PIN 4 // Configurable, see typical pin layout above
#define SS_PIN 16
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
unsigned long plannedNextScan; // used for timed PICC scanning
float timeToNextScan = 1000;   // time in milliseconds
bool isFirstIteration = true;

void setup()
{

	Serial.begin(115200);			   
	SPI.begin();					   
	mfrc522.PCD_Init();				   
	delay(4);
	mfrc522.PCD_DumpVersionToSerial(); 

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
	out->SetGain(0.24f); //0.24f
	randomSeed(analogRead(0));
}

//prints out the currently scanned UID
void printUID(byte* uid, int len){
	int uidArray[10];
	memset(uidArray, 0, sizeof(int)*10);
	getUidInt(uid, len, uidArray);
	Serial.printf("Scanned UID: ");
	for(int i=0;i<10;i++){
		Serial.printf("%d,", uidArray[i]);
	}
	Serial.println();
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
		printUID(mfrc522.uid.uidByte, mfrc522.uid.size);
	}
	return 1;
}

//takes in a character pointer of a filename, tries to play it with WAV driver.
void startPlayback(const char *filename)
{
	if (wav->isRunning())
		wav->stop();
	file = new AudioFileSourceSD(filename);
	buff = new AudioFileSourceBuffer(file, 2048);
	wav->begin(buff, out);
	isFirstIteration = false;
}

//scans for RFID card via Scan For Card function, tries to 
//play back corresponding song in WAV format.
void scanAndPlay(){
	char songName[100];
	memset(songName,0,sizeof(char)*100);
	char* returnFileName = songName;
	int cardFound = scanForCard(returnFileName);
	if (cardFound > 0) startPlayback(returnFileName);
	if(returnFileName == NULL) wav->stop();
}

void loop()
{
	if (wav->isRunning())
	{
		if (!wav->loop()) wav->stop();
		unsigned long millisSinceStart = millis();
		if (millisSinceStart > plannedNextScan)
		{
			scanAndPlay();
			plannedNextScan = millisSinceStart + timeToNextScan;
		}
		return;
	}
	else if (!wav->isRunning() && !isFirstIteration)
	{
		Serial.printf("done\n");
		delay(1000);
	}
	scanAndPlay();
}
