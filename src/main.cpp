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

#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "I2S.h"

// Sound Setup with I2s and audio CS PIN
AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;
int cs = 5;

// D3 - DIN, BCRL - D8, LRC - D4
// D3 = GPIO 0, D8 = GPIO 15, D4 = GIPO 2
// SD CS D1, GIPO 5

// RFID PINS
#define RST_PIN 4 // Configurable, see typical pin layout above
#define SS_PIN 16

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
const char *songs[5] = {"auf_der_mauer.wav", "bibabutzemann.wav", "tante_marokko.wav", "vogelhochzeit.wav", "annekaffekanne.wav"};
int iteration = 0;
bool notIncremented = true;
bool isFirstIteration = true;

void setup()
{

	Serial.begin(115200); // Initialize serial communications with the PC
	while (!Serial)
		;							   // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
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
	out->SetGain(0.1f);
}

void loop()
{

	if (wav->isRunning())
	{
		if (!wav->loop())
			wav->stop();
		notIncremented = true;
		return;
	}
	else if (!wav->isRunning() && notIncremented && !isFirstIteration)
	{
		Serial.printf("WAV done\n");
		iteration += 1;
		int getArrayLength = sizeof(songs) / sizeof(int);
		iteration = iteration % getArrayLength;
		notIncremented = false;
		delay(1000);
	}

	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if (!mfrc522.PICC_IsNewCardPresent())
	{
		return;
	}
	else
	{
		Serial.printf("WAV start\n");
		file = new AudioFileSourceSD(songs[iteration]);
		wav->begin(file, out);
		isFirstIteration = false;
		// Select one of the cards
		if (!mfrc522.PICC_ReadCardSerial())
		{
			return;
		}

		// Dump debug info about the card; PICC_HaltA() is automatically called
		mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
	}
}
