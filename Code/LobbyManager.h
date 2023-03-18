#ifndef __LOBBY_MANAGER_H__
#define __LOBBY_MANAGER_H__

#include "steam/steam_api.h"
#include <vector>
#include <string>
#include "LobbyMessage.h"
#include <list>

struct NetplayManager;

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
	enum LobbyType
	{
		LOBBYTYPE_NOT_SET,
		LOBBYTYPE_QUICKPLAY,
		LOBBYTYPE_PRACTICE,
		LOBBYTYPE_CUSTOM,
		LOBBYTYPE_Count,
	};

	std::string lobbyName;
	std::string mapPath;
	std::string fileHash;
	int gameModeType;
	bool isWorkshopMap;
	int randSeed;
	int maxMembers;
	int mapIndex; //when running next maps
	uint64 publishedFileId;
	uint64 creatorId;
	int lobbyType;
	int levelAdventureIndex;
	
	LobbyData();
	bool Update( CSteamID lobbyId );
	void SetLobbyData(CSteamID lobbyId);
	int GetNumStoredBytes();
	void StoreBytes( unsigned char *bytes );
	void SetFromBytes(unsigned char *bytes);
private:
	int ReadString( std::string &myStr, unsigned char *bytes);
	int StoreString(std::string &myStr, unsigned char *bytes);
	int StoreInt(int &myInt, unsigned char *bytes);
	int StoreBool(bool &myBool, unsigned char *bytes);
	int StoreUint64(uint64 &myUint64, unsigned char *bytes);

	//unsigned char * MakeBuffer( int &bufferLen );
};

struct Lobby
{
	Lobby()
	{
		m_steamIDLobby.Clear();
		dataIsRetrieved = false;
	}

	void Set(CSteamID p_lobbyId);
	void PopulateMemberList();

	CSteamID m_steamIDLobby;

	LobbyData data;
	std::list<LobbyMember> memberList;
	bool dataIsRetrieved;
};

struct LobbyManager
{
	enum SearchType
	{
		SEARCH_GET_ALL_OF_TYPE,
		SEARCH_MATCH_NAME,
		SEARCH_MATCH_WORLD,
		SEARCH_MATCH_WORLD_AND_SECTOR,
		SEARCH_MATCH_WORLD_AND_SECTOR_AND_LEVEL,
	};

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

	int currSearchType;

	Action action;
	NetplayManager *netplayManager;

	bool requestJoinFromInvite;

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

	int searchLobbyType;

	std::string searchMapPath;


	LobbyManager( NetplayManager *nm );

	void PopulateLobbyList( CSteamID lobbyID );
	void Update();
	bool IsAllLobbyDataReceived();

	void TryCreatingLobby(LobbyData &ld);
	void TryJoiningLobby( int lobbyIndex );
	void TryJoiningLobbyFromInvite(CSteamID id);

	void OnLobbyCreated(LobbyCreated_t *pCallback, bool bIOFailure);
	bool IsInLobby();

	bool CurrentLobbyHasMaxMembers();
	int GetNumCurrentLobbyMembers();

	int GetNumMembers();

	void FindQuickplayLobby();
	void FindPracticeLobby( const std::string &mapPath );
	void LeaveLobby();

	void RetrieveLobbyList( int lobbyType, int p_searchType );//add more params later
	void OnLobbyMatchListCallback(LobbyMatchList_t *pLobbyMatchList, bool bIOFailure);
	void ProcessLobbyList();
	
	void OnLobbyEnter(LobbyEnter_t *pCallback, bool bIOFailure);
	void PrintLobbies();

	//STEAM_CALLBACK(LobbyTester, OnLobbyDataUpdatedCallback, LobbyDataUpdate_t, m_CallbackLobbyDataUpdated);
	STEAM_CALLBACK(LobbyManager, OnLobbyChatUpdateCallback, LobbyChatUpdate_t);
	
	STEAM_CALLBACK(LobbyManager, OnLobbyEnterCallback, LobbyEnter_t);
	STEAM_CALLBACK(LobbyManager, OnLobbyDataUpdateCallback, LobbyDataUpdate_t);

	void QueryPracticeLobbies();
	


};

#endif