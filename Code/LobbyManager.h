#ifndef __LOBBY_MANAGER_H__
#define __LOBBY_MANAGER_H__

#include "steam/steam_api.h"
#include <vector>
#include <string>
#include "LobbyMessage.h"
#include <list>

struct WaitingRoom;

// an item in the list of lobbies we've found to display

struct LobbyMember
{
	CSteamID id;
	std::string name;
	HSteamNetConnection connection;
	LobbyMember(CSteamID p_id, const char *p_name)
	{
		id = p_id;
		name = p_name;
		connection = 0;
	}
};

struct LobbyData
{
	LobbyData()
	{
		gameModeType = 0;
		isWorkshopMap = false;
		randSeed = 0;
		maxMembers = 0;
		publishedFileId = 0;
		creatorId = 0;
	}

	std::string lobbyName;
	std::string mapPath;
	std::string fileHash;
	int gameModeType;
	bool isWorkshopMap;
	int randSeed;
	int maxMembers;
	uint64 publishedFileId;
	uint64 creatorId;

	bool Update( CSteamID lobbyId );
	void SetLobbyData(CSteamID lobbyId);
};

struct Lobby
{
	Lobby()
	{
		m_steamIDLobby.Clear();
		dataIsRetrieved = false;
	}

	void Set(CSteamID p_lobbyId);

	CSteamID m_steamIDLobby;

	LobbyData data;
	std::list<LobbyMember> memberList;
	bool dataIsRetrieved;
};

struct LobbyManager
{
	enum Action
	{
		A_IDLE,
		A_REQUEST_CREATE_LOBBY,
		A_IN_LOBBY_WAITING_FOR_DATA,
		A_IN_LOBBY,
		A_REQUEST_LOBBY_LIST,
		A_FOUND_LOBBIES,
		A_FOUND_LOBBIES_WAITING_FOR_DATA,
		A_FOUND_NO_LOBBIES,
		A_REQUEST_JOIN_LOBBY,
		A_ERROR,
	};

	Action action;

	// Track whether we are in the middle of a refresh or not
	bool m_bRequestingLobbies;
	std::vector<Lobby> lobbyVec;

	WaitingRoom *currWaitingRoom;
	bool readyForGameStart;

	LobbyData dataForMakingLobby;
	Lobby currentLobby;

	CCallResult<LobbyManager, LobbyCreated_t> m_SteamCallResultLobbyCreated;
	CCallResult<LobbyManager, LobbyMatchList_t> m_SteamCallResultLobbyMatchList;
	CCallResult<LobbyManager, LobbyEnter_t> m_SteamCallResultLobbyEnter;

	LobbyManager();

	void PopulateLobbyList( CSteamID lobbyID );
	void Update();
	bool IsAllLobbyDataReceived();

	void TryCreatingLobby(LobbyData &ld);
	void TryJoiningLobby( int lobbyIndex );

	void OnLobbyCreated(LobbyCreated_t *pCallback, bool bIOFailure);
	bool IsInLobby();

	bool CurrentLobbyHasMaxMembers();
	int GetNumCurrentLobbyMembers();

	int GetNumMembers();

	void FindLobby();
	void LeaveLobby();

	void RefreshLobbyList();
	void OnLobbyMatchListCallback(LobbyMatchList_t *pLobbyMatchList, bool bIOFailure);
	void ProcessLobbyList();
	
	void OnLobbyEnter(LobbyEnter_t *pCallback, bool bIOFailure);
	void PrintLobbies();

	//STEAM_CALLBACK(LobbyTester, OnLobbyDataUpdatedCallback, LobbyDataUpdate_t, m_CallbackLobbyDataUpdated);
	STEAM_CALLBACK(LobbyManager, OnLobbyChatUpdateCallback, LobbyChatUpdate_t);
	
	STEAM_CALLBACK(LobbyManager, OnLobbyEnterCallback, LobbyEnter_t);
	STEAM_CALLBACK(LobbyManager, OnLobbyDataUpdateCallback, LobbyDataUpdate_t);


};

#endif