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

	if (header.messageType == MESSAGE_TYPE_HOST_LOAD_MAP)
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

	if (header.messageType == MESSAGE_TYPE_HOST_LOAD_MAP)
	{
		numBytesStoredTotal += strLength;
	}

	bytesOut = new uint8[numBytesStoredTotal];

	uint8 *byteCurrent = bytesOut;

	memcpy(byteCurrent, &header, headerSize);
	byteCurrent += headerSize;
	
	if (header.messageType == MESSAGE_TYPE_HOST_LOAD_MAP)
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
	cout << "MESSAGE::  ";
	switch (header.messageType)
	{
	case MESSAGE_TYPE_PEER_DONE_CONNECTING:
		cout << "type: peer_done_connecting" << endl;
		break;
	case MESSAGE_TYPE_HOST_LOAD_MAP:
		cout << "type: host_load_map ";
		cout << "path: " << mapPath << endl;
		break;
	case MESSAGE_TYPE_PEER_READY_TO_RUN:
		cout << "type: peer_ready_to_run" << endl;
		break;
	case MESSAGE_TYPE_HOST_GAME_START:
		cout << "type: host_game_start" << endl;
		break;
	default:
		assert(false && "message header is invalid");
		break;
	}
}