#ifndef __LOBBY_MESSAGE_H__
#define __LOBBY_MESSAGE_H__

#include <string>
#include "steam\steam_api.h"

struct LobbyMessage
{
	enum MessageType : uint8
	{
		MESSAGE_TYPE_SHARE_MAP_DETAILS,
		MESSAGE_TYPE_START_CUSTOM_MATCH,
	};

	struct MessageHeader
	{
		uint8 messageType;
	};

	MessageHeader header;
	CSteamID sender;
	std::string mapPath;

	void SetFromBytes(uint8*bytes);
	int CreateBinaryMessage(uint8 *&bytesOut);
	void Print();
	

};

#endif