#ifndef __NETPLAY_MANAGER_H__
#define __NETPLAY_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "LobbyManager.h"
#include "ConnectionManager.h"
#include <boost/thread.hpp>
#include "MatchParams.h"
#include <list>
#include "VectorMath.h"
#include "PracticeMsg.h"
#include "BitField.h"

struct MatchParams;
struct GameSession;
struct UdpMsg;
//struct PracticeMsg;

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

struct PracticePlayer
{
	const static int MAX_BUFFERED_MESSAGES = 60 * 60; //can probably make this way bigger
	const static int MAX_SIM_FRAMES = 2;


	enum Action
	{
		A_NEEDS_START_MESSAGE,
		A_RUNNING,
	};


	bool readyToBeSentMessages;

	bool needsSessionRestart;
	bool alreadySentInitMessage;
	bool alreadySentStartMessage;

	int action;

	CSteamID id;
	HSteamNetConnection connection;
	std::string name;

	int indexInLobby;

	int skinIndex;
	BitField upgradeField;

	int waitingForFrame;
	int currReadIndex;
	int currWriteIndex;
	int nextFrameToRead;

	bool isConnectedTo;

	unsigned char *syncStateBuf;
	int syncStateBufSize;

	//GameSession *myGame;

	PracticeInputMsg messages[MAX_BUFFERED_MESSAGES];

	PracticePlayer();
	void Clear();
	void ClearSyncStateBuf();
	void ClearMessages();
	int HasInputs();
	void ReceiveInputMsg(PracticeInputMsg &pm);
	void ReceiveSteamMessage(SteamNetworkingMessage_t *message);
	COMPRESSED_INPUT_TYPE AdvanceInput();
	const PracticeInputMsg &GetNextMsg();

	bool HasBeenSentStartMessage();
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
	bool voteToKeepPlaying;

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
		POST_MATCH_A_QUICKPLAY_LEAVE,
		POST_MATCH_A_QUICKPLAY_KEEP_PLAYING,
	};

	enum Action
	{
		A_IDLE,
		A_PRACTICE_QUERYING_LOBBIES,
		A_PRACTICE_CHECKING_FOR_LOBBIES,
		A_PRACTICE_WAIT_FOR_IN_LOBBY,
		A_PRACTICE_SETUP,
		A_PRACTICE_TEST,
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
		A_WAIT_FOR_QUICKPLAY_VOTES_TO_KEEP_PLAYING,
		A_ALL_VOTED_TO_KEEP_PLAYING,
		A_DISCONNECT,
	};

	const static int MAX_PRACTICE_PLAYERS = 3;

	ControllerDualStateQueue *myControllerInput;
	ControlProfile *myCurrProfile;

	int myIndexInPracticeLobby;

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

	bool receivedLeaveNetplaySignal;

	boost::thread *loadThread;

	int numPracticeUsersPerWorld[8];

	LobbyManager *lobbyManager;
	ConnectionManager *connectionManager;
	GameSession *game;

	std::list<SteamNetworkingMessage_t*> ggpoMessageQueue;
	std::list<SteamNetworkingMessage_t*> desyncMessageQueue;

	std::vector<PracticePlayer> practicePlayers;// [MAX_PRACTICE_PLAYERS];

	bool desyncDetected;

	int playerIndex;

	bool isQuickplay;

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

	std::string practiceSearchMapPath;
	int practiceSearchAdventureMapIndex;

	int currMapIndex;


	NetplayManager();
	~NetplayManager();

	
	void Init();
	bool IsReadyToRun();
	bool IsIdle();
	void LeaveLobby();
	int RunMatch();
	int GetGGPOConnectionIndex(HSteamNetConnection &con);
	int GetPracticeConnectionIndex(HSteamNetConnection &con);
	void SetHost();
	bool IsConnectedToHost();
	
	void HostLoadNextQuickplayMap();

	void SendSignalToHost(int type);
	void SendSignalToAllClients(int type);

	std::string GetNextQuickplayMapName();
	void HostStartLoading();
	void HostQuickplayVoteToKeepPlaying();

	void SendDesyncCheckToHost( int currGameFrame );
	void ConnectToAll();
	void ReceiveMessages();
	HSteamNetConnection GetHostConnection();

	void PrepareClientForNextQuickplayMap();
	
	HSteamNetConnection GetConnection();
	void AddDesyncCheckInfo( int pIndex, DesyncCheckInfo &dci );
	void RemoveDesyncCheckInfos(int numRollbackFrames);
	void UpdateNetplayPlayers();
	void UpdatePracticePlayers();
	bool IsHost();
	bool IsLobbyHost();
	void Abort();
	void Update();
	void Draw(sf::RenderTarget *target);
	void FindQuickplayMatch();
	void FindPracticeMatch( const std::string &mapPath, int adventureMapIndex );
	
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

	void SendPostMatchQuickplayLeaveSignalToHost();

	void SendPostMatchChooseMapSignalToClients();
	void SendPostMatchQuickplayKeepPlayingSignalToClients();
	void SendPostMatchQuickplayVoteToKeepPlayingToHost();

	bool AllPlayersHaveFinishedWithResultsScreen();
	bool CheckResultsScreen();

	void SendConnectToAllSignalToAllClients();

	void SendFileToConnection(HSteamNetConnection connection, boost::filesystem::path &p,
		int udpMsgType );

	void SendBufferToConnection(HSteamNetConnection connection, unsigned char *buf, int bufSize,
		int udpMsgType);

	void SendLobbyDataForNextMapToClients(LobbyData *ld);

	
	bool SendPracticeInitMessageToPlayer(PracticePlayer &pracPlayer);
	void SendPracticeInitMessageToAllNewPeers();

	bool SendPracticeStartMessageToPlayer(PracticePlayer &pracPlayer, PracticeStartMsg &pm);
	bool SendPracticeStartMessageToPlayerAsBuffer(PracticePlayer &pracPlayer, unsigned char *buf, int bufSize);
	void SendPracticeStartMessageToAllNewPeers(PracticeStartMsg &pm);

	bool SendPracticeInputMessageToPlayer(PracticePlayer &pracPlayer, PracticeInputMsg &pm);
	void SendPracticeInputMessageToAllPeers(PracticeInputMsg &pm);
	

	STEAM_CALLBACK(NetplayManager, OnLobbyChatMessageCallback, LobbyChatMsg_t);
	STEAM_CALLBACK(NetplayManager, OnConnectionStatusChangedCallback, SteamNetConnectionStatusChangedCallback_t);
	//STEAM_CALLBACK(NetplayManager, OnSteamNetworkingMessagesSessionFailed, SteamNetworkingMessagesSessionFailed_t);
	//STEAM_CALLBACK(NetplayManager, OnSteamNetworkingMessagesSessionRequest, SteamNetworkingMessagesSessionRequest_t);

	void OnConnectStatusChangedGGPO(SteamNetConnectionStatusChangedCallback_t *callback);
	void OnConnectStatusChangedPractice(SteamNetConnectionStatusChangedCallback_t *callback);


	void OnLobbyChatUpdateCallback(LobbyChatUpdate_t *pCallback);

	bool IsPracticeMode();

	bool TrySetupPractice( GameSession *game );

	void QueryPracticeMatches();

	void PracticeConnect();
	void SetPracticePlayersToNeedStartMessage();
};

#endif