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
#include <EEPROM.h>

//KEY VALUE PAIRS OF UID AND CORRESPONDING SONGS///////////////////
char sa[] = {"RedHouse.wav"};
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

char sBlueHouse[] = "Chantaje.wav";
int kBlueHouse[] = {4,193,92,1,74,72,3,0,0,0};
KeyValue bHouse = {sBlueHouse, kBlueHouse};

char sZylinder[] = "DontStopMeNow.wav";
int kZylinder[] = {4,209,27,1,187,72,3,0,0,0};
KeyValue zylinder = {sZylinder, kZylinder};

char sZylinder2[] = "BohemianRhapsody.wav";
int kZylinder2[] = {4,193,169,1,220,72,3,0,0,0};
KeyValue zylinder2 = {sZylinder2, kZylinder2};

char sBuilder[] = "vogelhochzeit.wav";
int kBuilder[] = {4,144,156,1,212,77,3,0,0,0};
KeyValue builder= {sBuilder, kBuilder};

char sGreenHouse[] = "good4u.wav";
int kGreenHouse[] = {4,225,179,1,179,5,3,0,0,0};
KeyValue greenHouse = {sGreenHouse, kGreenHouse};

char sSleep[] = "DEEP_SLEEP";
int kSleep[] = {4,209,52,1,107,72,3,0,0,0};
KeyValue kvSleep = {sSleep, kSleep};

char sbear[] = "bibabutzemann.wav";
int kbear[] = {4,193,150,1,240,72,3,0,0,0};
KeyValue kvbear = {sbear, kbear};

char spptrol[] = "Duality.wav";
int kpptrol[] = {4,160,58,1,245,77,3,0,0,0};
KeyValue kvpptrol = {spptrol, kpptrol};

char scat[] = "Bloodline.wav";
int kcat[] = {4,209,66,1,143,72,3,0,0,0};
KeyValue kvcat = {scat, kcat};

KeyValue songArray[] = {A,B,C,D,E,Horse,Bear,H,G,Mushroom,bHouse,zylinder,zylinder2,kvSleep,greenHouse,builder,kvbear,kvpptrol,kvcat};
int songArrayLen = 19;
///////////////END OF KEY VALUE PAIRS.////////////////////////////

#define DEVICE_TYPE_OLD_BOX 0
#define DEVICE_TYPE_NEW_BOX 1
#define DEVICE_TYPE DEVICE_TYPE_NEW_BOX

// Sound Setup with I2s and audio CS PIN
AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;
int cs = 5; //SD CARD!
AudioFileSourceBuffer *buff;

// RFID PINS
#define RST_PIN 4 // Configurable, see typical pin layout above
#define SS_PIN 16
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
unsigned long plannedNextScan; // used for timed PICC scanning
float timeToNextScan = 1000;   // time in milliseconds
bool isFirstIteration = true;

// VOLUME CONTROL
float gain = 0.07f; //0.24f should be max
float step = 0.02f;
int low_b_prev_state = 1; //input of low button in the last iteration
int high_b_prev_state = 1; //input of high button in the last iteration

//...reverse
#if DEVICE_TYPE == DEVICE_TYPE_OLD_BOX
#define HIGH_VOLUME 10 //GIPO 10 PIN SD3
#define LOW_VOLUME 0 //(GIPO 9 PIN SD2)-> GIPO 0 PIN D3
#elif DEVICE_TYPE == DEVICE_TYPE_NEW_BOX
#define LOW_VOLUME A0 //Pullup-Circuit on A0
#define HIGH_VOLUME 0 //(GIPO 9 PIN SD2)-> GIPO 0 PIN D3
#endif

//NEW BOX VERSION ONLY - SHUTOFF PIN
#if DEVICE_TYPE == DEVICE_TYPE_NEW_BOX
#define SHUTOFF_PIN 10 //GIPO 10 PIN SD3
#endif
unsigned long lastTimestamp = 0;
unsigned long minutes = 15; //mins until automatic shutdown
unsigned long timeUntilSleep = 1000*60*minutes;
unsigned long buttonPressSec = 2; //seconds to press minus button till shutdown
unsigned long lastPress = 0;
bool shutdown = false;


void setup()
{
	Serial.begin(115200);			   
	SPI.begin();	
	//Setup BOX Type
	Serial.print("Setting up device. Type: ");
	if(DEVICE_TYPE == DEVICE_TYPE_OLD_BOX){
		Serial.print("OLD_BOX");
		pinMode(HIGH_VOLUME, INPUT_PULLUP);
		pinMode(LOW_VOLUME, INPUT_PULLUP);
	}
	else if(DEVICE_TYPE == DEVICE_TYPE_NEW_BOX){
		pinMode(HIGH_VOLUME, INPUT_PULLUP);
		pinMode(SHUTOFF_PIN, OUTPUT);
		digitalWrite(SHUTOFF_PIN,HIGH);
		Serial.println("NEW_BOX"); 

	}	   
	//Sensor INIT
	mfrc522.PCD_Init();				   
	delay(4);
	mfrc522.PCD_DumpVersionToSerial(); 
	
	//SD Card INIT
	Serial.print("Initializing SD card...");
	if (!SD.begin(cs))
	{
		Serial.println("initialization failed!");
		return;
	}
	Serial.println("initialization done.");

	//Audio SETUP
	audioLogger = &Serial;
	out = new AudioOutputI2S();
	wav = new AudioGeneratorWAV();
	buff = new AudioFileSourceBuffer(file, 2048);
	out->SetGain(gain);
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

//Convert Signal from A0 Pin
int analogueToDigitalConversion(){
	int output;
	output = analogRead(A0);
	if(output<1024)output = 0;
	else output = 1;
	return output;
}

//returns -1 for no button, 0 for minus, 1 for plus
int ButtonInput(){
	int input = -1;
	int higher_volume;
	int lower_volume;
	//DANGER! Pullup pin setup returns 0 as pos. button press!!
	higher_volume = digitalRead(HIGH_VOLUME);
	lower_volume = analogueToDigitalConversion();
	if(lower_volume<=0 && low_b_prev_state>0) input=0;
	else if(higher_volume<=0 && high_b_prev_state>0) input=1;
	low_b_prev_state = lower_volume;
	high_b_prev_state = higher_volume;
	return input;
}

float adjustGain(int gainButton, float currGain){
	float g = currGain;
	if(gainButton==0) g -= step;
	else if(gainButton==1) g += step;
	g = constrain(g, 0,0.24f);
	return g;
}

//Trigger shutdown, activation with external MOSFET
void initiateShutdown(){
	Serial.println("Shutting down...");
	digitalWrite(SHUTOFF_PIN,LOW);
}

//scans minus button on A0, returns true if pressed for x sec.
bool ScanShutdownButton(unsigned long seconds){
	int minButtonState = analogueToDigitalConversion();
	//reset counter if minus button is not pressed - watch out for pullup resistor signal!
	if(minButtonState>0){
		lastPress = millis();
		return false;
	}
	else if(millis()>(lastPress+(seconds*1000))) return true;
	return false;
}


void loop()
{
	gain = adjustGain(ButtonInput(), gain);
	out->SetGain(gain);
	shutdown = ScanShutdownButton(buttonPressSec);
	if (wav->isRunning())
	{
		if (!wav->loop()) wav->stop();
		unsigned long millisSinceStart = millis();
		if (millisSinceStart > plannedNextScan)
		{
			scanAndPlay();
			plannedNextScan = millisSinceStart + timeToNextScan;
		}
		lastTimestamp = millis();
		if(shutdown) initiateShutdown();
		return;
	}
	else if(millis()>(lastTimestamp+timeUntilSleep)||shutdown) initiateShutdown();
	else if (!wav->isRunning() && !isFirstIteration)
	{
		Serial.printf("done\n");
		delay(1000);
	}
	scanAndPlay();
}
