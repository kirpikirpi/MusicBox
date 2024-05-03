#include "RFIDLogic.h"

//returns one if they are the same, -1 if not.
int compareUids(int* uidOne ,int* uidTwo){
	for (int i = 0; i< 10; i++){
		if(uidOne[i] == uidTwo[i]){
		}
	}
	return -1;
}

//returns corresponding song name to uid in fileName.
int getSongNameFromTag(byte* uid, int len, KeyValue* songArray, int songArrayLen, char* fileName){
	int uidArray[10];
	getUidInt(uid, len, uidArray); 
	
	for (int i = 0; i< songArrayLen; i++){
		for(int j=0;j<10;j++){
			if(songArray[i].key[j] != uidArray[j]) break;
			if (j<=0){
				char* sn = songArray[i].songName;
				while(*sn) *fileName++ = *sn++;
				return j;
			}
		}
	}
	return -1;
}

//UID has max. 10 bytes, this function returns them as int array.
int getUidInt(byte* uid, int len, int* uidInt){
	for (int i = 0; i<10;i++){
		*uidInt++ = (i<len) ? *uid++ : 0;
	}
	return 1;
}

/*int scanForCard()
{
	
	int uidSum = -1;
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if (!mfrc522.PICC_IsNewCardPresent())
	{
		return -1;
	}
	else
	{

		// Select one of the cards
		if (!mfrc522.PICC_ReadCardSerial())
		{
			return -1;
		}

		// Dump debug info about the card; PICC_HaltA() is automatically called
		uidSum = mfrc522.getUid(&(mfrc522.uid));
		mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
	}
	return uidSum;
	
return -1;
}*/