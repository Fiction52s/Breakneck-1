#ifndef __LOBBY_MANAGER_H__
#define __LOBBY_MANAGER_H__

#include "steam/steam_api.h"
#include <vector>
#include <string>
#include "LobbyMessage.h"
#include <list>

// an item in the list of lobbies we've found to display
struct Lobby
{
	Lobby()
	{
		m_steamIDLobby.Clear();
		maxMembers = 0;
		dataIsRetrieved = false;
	}
	CSteamID m_steamIDLobby;
	//char m_rgchName[256];
	std::string name;
	//bool createdByMe;
	int maxMembers;
	std::list<CSteamID> memberList;
	bool dataIsRetrieved;
};

struct LobbyParams
{
	int maxMembers;
	std::string mapPath;
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
		A_FOUND_NO_LOBBIES,
		A_REQUEST_JOIN_LOBBY,
		A_ERROR,
	};

	Action action;

	// Track whether we are in the middle of a refresh or not
	bool m_bRequestingLobbies;
	std::vector<Lobby> lobbyVec;

	bool readyForGameStart;

	LobbyParams paramsForMakingLobby;
	Lobby currentLobby;

	CCallResult<LobbyManager, LobbyCreated_t> m_SteamCallResultLobbyCreated;
	CCallResult<LobbyManager, LobbyMatchList_t> m_SteamCallResultLobbyMatchList;
	CCallResult<LobbyManager, LobbyEnter_t> m_SteamCallResultLobbyEnter;

	LobbyManager();

	void PopulateLobbyList( CSteamID lobbyID );
	void Update();

	void TryCreatingLobby(LobbyParams &lp);
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