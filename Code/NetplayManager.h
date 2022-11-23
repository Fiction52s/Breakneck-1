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

struct MatchResultsScreen;

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
	bool hasPreviewAndMap;
	bool doneVerifyingMap;
	bool doneLoading;
	bool readyToRun;
	bool isHost;
	bool finishedWithResultsScreen;
	std::string name;
	DesyncCheckInfo desyncCheckInfoArray[MAX_DESYNC_CHECK_INFOS_STORED];
	int skinIndex;
	bool hasAllData;

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

	enum PostMatchAction
	{
		POST_MATCH_A_REMATCH,
		POST_MATCH_A_CHOOSE_MAP,
		POST_MATCH_A_LEAVE,
	};

	enum Action
	{
		A_IDLE,
		A_QUICKPLAY_CHECKING_FOR_LOBBIES,
		A_QUICKPLAY_GATHERING_USERS,
		A_CUSTOM_HOST_GATHERING_USERS,
		A_WAIT_FOR_PREVIEW,
		A_CONNECT_TO_ALL,
		A_WAIT_FOR_ALL_TO_VERIFY,
		A_WAIT_FOR_ALL_TO_CONNECT,
		A_WAIT_TO_VERIFY,
		A_WAIT_FOR_MAP_FROM_HOST,
		A_WAIT_FOR_MAP_FROM_WORKSHOP,
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
	bool receivedMapVerifySignal;
	bool receivedGameStartSignal;
	bool receivedStartGGPOSignal;
	bool receivedMap;
	bool receivedPreview;
	bool receivedPostOptionsSignal;

	int postMatchOptionReceived;

	bool receivedNextMapData;
	LobbyData nextMapData;

	bool waitingForMap;
	bool waitingForPreview;

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

	bool checkWorkshopMap;
	std::string receivedMapName;
	PublishedFileId_t workshopDownloadPublishedFileId;
	boost::filesystem::path mapDownloadFilePath;
	boost::filesystem::path previewPath;
	std::string mapDownloadReceivedHash;

	MatchResultsScreen *resultsScreen;


	NetplayManager();
	~NetplayManager();

	void Init();
	bool IsReadyToRun();
	bool IsIdle();
	void LeaveLobby();
	int RunMatch();
	int GetConnectionIndex(HSteamNetConnection &con);
	void SetHost();
	bool IsConnectedToHost();
	
	void SendSignalToHost(int type);
	void SendSignalToAllClients(int type);

	void HostStartLoading();

	void SendDesyncCheckToHost( int currGameFrame );
	void ConnectToAll();
	void ReceiveMessages();
	HSteamNetConnection GetHostConnection();
	
	HSteamNetConnection GetConnection();
	void AddDesyncCheckInfo( int pIndex, DesyncCheckInfo &dci );
	void RemoveDesyncCheckInfos(int numRollbackFrames);
	void UpdateNetplayPlayers();
	bool IsHost();
	void Abort();
	void Update();
	void Draw(sf::RenderTarget *target);
	void FindQuickplayMatch();
	void LoadMap();
	void HandleLobbyMessage(LobbyMessage &msg);
	void HandleMessage(HSteamNetConnection connection, SteamNetworkingMessage_t *msg);
	CSteamID GetHostID();
	CSteamID GetMyID();
	void SendUdpMsg(HSteamNetConnection con, UdpMsg *msg);
	const DesyncCheckInfo & GetDesyncCheckInfo(SteamNetworkingMessage_t *msg, int framesAgo);
	void DumpDesyncInfo();
	void SendMapToClient(HSteamNetConnection connection, boost::filesystem::path &p);
	void SendPreviewToClient(HSteamNetConnection connection, boost::filesystem::path &p);

	void TryCreateCustomLobby(LobbyData &ld);
	void RequestPreviewFromHost();
	void RequestMapFromHost();
	void SendReceivedAllDataSignalToHost();

	void BroadcastMapDetailsToLobby();
	void BroadcastLobbyMessage(LobbyMessage &msg);

	void HostInitiateRematch();

	//void CheckForMapAndSetMatchParams( LobbyData &lobbyData );
	void CheckForMapAndSetMatchParams();
	bool ClientCheckWorkshopMapInstalled();

	void CleanupMatch();

	bool AllClientsHaveReceivedAllData();

	MatchResultsScreen *CreateResultsScreen();

	void SendFinishedResultsScreenSignalToHost();

	void ClearClientsFinishingResultsScreen();
	void HostFinishResultsScreen();
	void ClearDataForNextMatch();

	void SendPostMatchChooseMapSignalToClients();

	bool AllPlayersHaveFinishedWithResultsScreen();
	bool CheckResultsScreen();

	void SendConnectToAllSignalToAllClients();

	void SendFileToConnection(HSteamNetConnection connection, boost::filesystem::path &p,
		int udpMsgType );

	void SendBufferToConnection(HSteamNetConnection connection, unsigned char *buf, int bufSize,
		int udpMsgType);

	void SendLobbyDataForNextMapToClients(LobbyData *ld);

	STEAM_CALLBACK(NetplayManager, OnLobbyChatMessageCallback, LobbyChatMsg_t);
	STEAM_CALLBACK(NetplayManager, OnConnectionStatusChangedCallback, SteamNetConnectionStatusChangedCallback_t);

	void OnLobbyChatUpdateCallback(LobbyChatUpdate_t *pCallback);
};

#endif