#include "RFIDLogic.h"

//returns one if they are the same, -1 if not.
int compareUids(int* uidOne ,int* uidTwo){
	for (int i = 0; i< 10; i++){
		if(uidOne[i] != uidTwo[i]) return -1;
	}
	return 1;
}


//returns corresponding song name to uid in fileName.
int getSongNameFromTag(byte* scannedUid, KeyValue* songArray, int songArrayLen, char* fileName) {
	int scannedUidArray[10];
	getUidInt(scannedUid, 10, scannedUidArray);
	
	for (int i = 0; i < songArrayLen; i++) {
		if (compareUids(scannedUidArray, songArray[i].key)>0) {
			char* sn = songArray[i].songName;
			while (*sn) *fileName++ = *sn++;
			return 1;
		}
	}
	fileName = NULL;
	return -1;
}

//UID has max. 10 bytes, this function returns them as int array.
int getUidInt(byte* uid, int len, int* uidInt){
	for (int i = 0; i<10;i++){
		*uidInt++ = (i<len) ? *uid++ : 0;
	}
	return 1;
}