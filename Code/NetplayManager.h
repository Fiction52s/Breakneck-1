#ifndef __NETPLAY_MANAGER_H__
#define __NETPLAY_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "LobbyManager.h"
#include "ConnectionManager.h"
#include <boost/thread.hpp>
#include "MatchParams.h"

struct MatchParams;
struct GameSession;

struct NetplayPlayer
{
	NetplayPlayer();
	CSteamID id;
	HSteamNetConnection connection;
	bool isMe;
	int index;
};

struct NetplayManager
{
	enum Action
	{
		A_IDLE,
		A_GATHERING_USERS,
		A_GET_CONNECTIONS,
		A_WAIT_TO_LOAD_MAP,
		A_LOAD_MAP,
		A_READY_TO_RUN,
		A_RUNNING_MATCH,
	};

	Action action;
	sf::Vertex quad[4];
	bool isSyncTest;

	bool receivedMapLoadSignal;

	boost::thread *loadThread;

	LobbyManager *lobbyManager;
	ConnectionManager *connectionManager;
	GameSession *game;

	int playerIndex;

	MatchParams matchParams;

	NetplayPlayer netplayPlayers[4];

	bool clientsDoneLoadingMap[4];

	NetplayManager();
	~NetplayManager();

	bool IsConnected();
	bool IsReadyToRun();
	bool IsIdle();
	void LeaveLobby();
	void RunMatch();
	
	HSteamNetConnection GetConnection();
	bool IsHost();
	void Abort();
	void Update();
	void Draw(sf::RenderTarget *target);
	void FindMatch();
	void LoadMap();
	void HandleMessage(LobbyMessage &msg);
	CSteamID GetHostID();
	CSteamID GetMyID();

	void BroadcastLoadMapSignal();

	STEAM_CALLBACK(NetplayManager, OnLobbyChatMessageCallback, LobbyChatMsg_t);

};

#endif