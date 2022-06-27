#include "LobbyMessage.h"
#include <iostream>
#include <assert.h>

using namespace std;

void LobbyMessage::SetFromBytes(uint8*bytes)
{
	memcpy(&header, bytes, sizeof(MessageHeader));
	bytes += sizeof(MessageHeader);

	uint32 strLength = (uint32)(*bytes);

	bytes += sizeof(uint32);

	mapPath = (char*)bytes;

	bytes += strLength;
}

int LobbyMessage::CreateBinaryMessage( uint8 *bytesOut)
{
	int numBytesStoredTotal = 0;

	int headerSize = sizeof(MessageHeader);
	uint32 strLength = mapPath.length() + 1;

	numBytesStoredTotal += headerSize;
	numBytesStoredTotal += strLength;

	bytesOut = new uint8[numBytesStoredTotal];

	memcpy(bytesOut, &header, headerSize);
	bytesOut += headerSize;
	
	for (int i = 0; i < strLength - 1; ++i)
	{
		bytesOut[i] = (uint8)mapPath.at(i);
	}
	bytesOut[strLength] = '\0';

	bytesOut += strLength;

	return numBytesStoredTotal;
}

void LobbyMessage::Print()
{
	cout << "MESSAGE::  ";
	switch (header.messageType)
	{
	case MESSAGE_TYPE_LOAD_MAP:
		cout << "type: load_map ";
		cout << "path: " << mapPath << endl;
		break;
	default:
		assert(false && "message header is invalid");
		break;
	}
}