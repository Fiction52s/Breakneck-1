#ifndef __NETPLAY_MANAGER_H__
#define __NETPLAY_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "LobbyManager.h"
#include "ConnectionManager.h"
#include <boost/thread.hpp>
#include "MatchParams.h"

struct MatchParams;
struct GameSession;
struct UdpMsg;

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
	bool doneLoading;
	bool readyToRun;
	bool isHost;
};

struct NetplayManager
{
	enum Channels
	{
		CHANNEL_GGPO,
		CHANNEL_SESSION,
	};

	enum Action
	{
		A_IDLE,
		A_CHECKING_FOR_LOBBIES,
		A_GATHERING_USERS,
		A_GET_CONNECTIONS,
		A_WAIT_FOR_ALL_TO_CONNECT,
		A_WAIT_TO_LOAD_MAP,
		A_WAIT_TO_GGPO_SYNC,
		A_LOAD_MAP,
		A_LOAD_MAP_AND_WAIT_FOR_ALL,
		A_GGPO_SYNC,
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
	bool receivedStartGGPOSignal;

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

	bool IsReadyToRun();
	bool IsIdle();
	void LeaveLobby();
	int RunMatch();
	int GetConnectionIndex(HSteamNetConnection &con);
	void SetHost();
	
	void SendSignalToHost(int type);
	void SendSignalToAllClients(int type);



	void ReceiveMessages();
	HSteamNetConnection GetHostConnection();
	
	HSteamNetConnection GetConnection();
	bool IsHost();
	void Abort();
	void Update();
	void Draw(sf::RenderTarget *target);
	void FindMatch();
	void LoadMap();
	void HandleLobbyMessage(LobbyMessage &msg);
	void HandleMessage(HSteamNetConnection connection, SteamNetworkingMessage_t *msg);
	CSteamID GetHostID();
	CSteamID GetMyID();
	void SendUdpMsg(HSteamNetConnection con, UdpMsg *msg);

	void BroadcastMapDetailsToLobby();
	

	STEAM_CALLBACK(NetplayManager, OnLobbyChatMessageCallback, LobbyChatMsg_t);
	STEAM_CALLBACK(NetplayManager, OnConnectionStatusChangedCallback, SteamNetConnectionStatusChangedCallback_t);
};

#endif