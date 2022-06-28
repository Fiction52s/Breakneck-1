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
	void Clear();
	CSteamID id;
	bool isConnectedTo;
	HSteamNetConnection connection;
	bool isMe;
	int index;
	bool doneConnectingToAllPeers;
	bool readyToRun;
	
};

struct NetplayManager
{
	enum Action
	{
		A_IDLE,
		A_GATHERING_USERS,
		A_GET_CONNECTIONS,
		A_WAIT_FOR_ALL_TO_CONNECT,
		A_WAIT_TO_LOAD_MAP,
		A_LOAD_MAP,
		A_READY_TO_RUN,
		A_WAIT_FOR_ALL_READY,
		A_RUNNING_MATCH,
		A_MATCH_COMPLETE,
	};

	Action action;
	sf::Vertex quad[4];
	bool isSyncTest;

	int numPlayers;
	bool receivedMapLoadSignal;
	bool receivedGameStartSignal;

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
	int RunMatch();
	int GetConnectionIndex(HSteamNetConnection &con);
	
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

	void HostBroadcastLoadMapSignal();
	void HostBroadcastGameStartSignal();


	void PeerBroadcastDoneConnectingSignal();
	void PeerBroadcastReadyToRunSignal();
	

	STEAM_CALLBACK(NetplayManager, OnLobbyChatMessageCallback, LobbyChatMsg_t);
	STEAM_CALLBACK(NetplayManager, OnConnectionStatusChangedCallback, SteamNetConnectionStatusChangedCallback_t);
};

#endif