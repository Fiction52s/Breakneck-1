#ifndef __NETPLAY_MANAGER_H__
#define __NETPLAY_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "LobbyManager.h"
#include "ConnectionManager.h"
#include <boost/thread.hpp>
#include "MatchParams.h"
#include <list>
#include "VectorMath.h"

struct MatchParams;
struct GameSession;
struct UdpMsg;

struct DesyncCheckInfo
{
	DesyncCheckInfo()
	{
		action = 0;
		actionFrame = 0;
		health = -1;
		gameFrame = -1;
	}
	V2d pos;
	uint32 action;
	uint32 actionFrame;
	int health;
	int gameFrame;
};


struct NetplayPlayer
{
	const static int MAX_DESYNC_CHECK_INFOS_STORED = 180;

	CSteamID id;
	bool isConnectedTo;
	HSteamNetConnection connection;
	bool isMe;
	int index;
	bool doneConnectingToAllPeers;
	bool doneLoading;
	bool readyToRun;
	bool isHost;
	DesyncCheckInfo desyncCheckInfoArray[MAX_DESYNC_CHECK_INFOS_STORED];
	int skinIndex;

	NetplayPlayer();
	void Clear();
	void AddDesyncCheckInfo(DesyncCheckInfo &dci);
	void RemoveDesyncCheckInfo();
	const DesyncCheckInfo & GetDesyncCheckInfo(int framesAgo);
	void DumpDesyncInfo();
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
		A_LOAD_MAP,
		A_WAIT_FOR_GGPO_SYNC,
		A_WAITING_FOR_START_MESSAGE,
		A_READY_TO_RUN,
		A_WAIT_FOR_ALL_READY,
		A_RUNNING_MATCH,
		A_MATCH_COMPLETE,
		A_DISCONNECT,
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

	std::list<SteamNetworkingMessage_t*> ggpoMessageQueue;
	std::list<SteamNetworkingMessage_t*> desyncMessageQueue;

	bool desyncDetected;

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

	void SendDesyncCheckToHost( int currGameFrame );



	void ReceiveMessages();
	HSteamNetConnection GetHostConnection();
	
	HSteamNetConnection GetConnection();
	void AddDesyncCheckInfo( int pIndex, DesyncCheckInfo &dci );
	void RemoveDesyncCheckInfos(int numRollbackFrames);
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
	const DesyncCheckInfo & GetDesyncCheckInfo(SteamNetworkingMessage_t *msg, int framesAgo);
	void DumpDesyncInfo();

	void BroadcastMapDetailsToLobby();
	

	STEAM_CALLBACK(NetplayManager, OnLobbyChatMessageCallback, LobbyChatMsg_t);
	STEAM_CALLBACK(NetplayManager, OnConnectionStatusChangedCallback, SteamNetConnectionStatusChangedCallback_t);
};

#endif