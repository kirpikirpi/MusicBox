#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

struct KeyValue{
    char* songName;
    int* key;
};

int compareUids(int* uidOne ,int* uidTwo);
int getSongNameFromTag(byte* scannedUid, KeyValue* songArray, int songArrayLen, char* fileName);
int getUidInt(byte* uid, int len, int* uidInt);
//int scanForCard();