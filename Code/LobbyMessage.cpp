#include "LobbyMessage.h"
#include <iostream>
#include <assert.h>

using namespace std;

void LobbyMessage::SetFromBytes(uint8*bytes)
{
	memcpy(&header, bytes, sizeof(MessageHeader));
	bytes += sizeof(MessageHeader);

	//uint32 strLength = (uint32)(*bytes);

	//bytes += sizeof(uint32);

	if (header.messageType == MESSAGE_TYPE_SHARE_MAP_DETAILS)
	{
		mapPath = (char*)bytes;

		bytes += mapPath.size() + 1;
	}
}

int LobbyMessage::CreateBinaryMessage( uint8 *&bytesOut)
{
	int numBytesStoredTotal = 0;

	int headerSize = sizeof(MessageHeader);

	uint32 strLength = mapPath.length() + 1;

	numBytesStoredTotal += headerSize;

	if (header.messageType == MESSAGE_TYPE_SHARE_MAP_DETAILS)
	{
		numBytesStoredTotal += strLength;
	}

	bytesOut = new uint8[numBytesStoredTotal];

	uint8 *byteCurrent = bytesOut;

	memcpy(byteCurrent, &header, headerSize);
	byteCurrent += headerSize;
	
	if (header.messageType == MESSAGE_TYPE_SHARE_MAP_DETAILS)
	{
		for (int i = 0; i < strLength - 1; ++i)
		{
			byteCurrent[i] = (uint8)mapPath.at(i);
		}
		byteCurrent[strLength-1] = '\0';

		byteCurrent += strLength;
	}
	

	return numBytesStoredTotal;
}

void LobbyMessage::Print()
{
	cout << "LOBBY MESSAGE::  ";
	switch (header.messageType)
	{
	case MESSAGE_TYPE_SHARE_MAP_DETAILS:
		cout << "type: share_map_details ";
		cout << "path: " << mapPath << endl;
		break;
	case MESSAGE_TYPE_START_CUSTOM_MATCH:
	{
		cout << "custom match start" << endl;
		break;
	}
	default:
		assert(false && "message header is invalid");
		break;
	}
}