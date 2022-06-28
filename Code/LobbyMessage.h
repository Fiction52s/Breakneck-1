#ifndef __LOBBY_MESSAGE_H__
#define __LOBBY_MESSAGE_H__

#include <string>
#include "steam\steam_api.h"

struct LobbyMessage
{
	enum MessageType : uint8
	{
		MESSAGE_TYPE_PEER_DONE_CONNECTING,
		MESSAGE_TYPE_HOST_LOAD_MAP,
		MESSAGE_TYPE_PEER_READY_TO_RUN,
		MESSAGE_TYPE_HOST_GAME_START,
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