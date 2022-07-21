#include "LobbyManager.h"
#include <string>
#include <iostream>
#include "LobbyMessage.h"

using namespace std;

LobbyManager::LobbyManager()
{
	action = A_IDLE;
	m_bRequestingLobbies = false;
}

void LobbyManager::TryCreatingLobby(LobbyParams &lp)
{
	if (!m_SteamCallResultLobbyCreated.IsActive())
	{
		paramsForMakingLobby = lp;

		SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, paramsForMakingLobby.maxMembers);
		m_SteamCallResultLobbyCreated.Set(hSteamAPICall, this, &LobbyManager::OnLobbyCreated);
		action = A_REQUEST_CREATE_LOBBY;
		cout << "attempting to create lobby" << endl;
	}
	else
	{
		cout << "lobby creation call result is busy" << endl;
	}
}

void LobbyManager::OnLobbyCreated(LobbyCreated_t *pCallback, bool bIOFailure)
{
	if (action != A_REQUEST_CREATE_LOBBY)
		return;

	// record which lobby we're in
	if (pCallback->m_eResult == k_EResultOK)
	{
		// success
		currentLobby.m_steamIDLobby = pCallback->m_ulSteamIDLobby;
		//currentLobby.createdByMe = true;

		// set the name of the lobby if it's ours
		string lobbyName = SteamFriends()->GetPersonaName();
		lobbyName += "'s lobby";

		SteamMatchmaking()->SetLobbyData(currentLobby.m_steamIDLobby, "name", lobbyName.c_str());

		SteamMatchmaking()->SetLobbyData(currentLobby.m_steamIDLobby, "mapPath", paramsForMakingLobby.mapPath.c_str());

		SteamMatchmaking()->SetLobbyData(currentLobby.m_steamIDLobby, "fileHash", paramsForMakingLobby.fileHash.c_str());

		string creatorIDStr = to_string(paramsForMakingLobby.creatorID);
		SteamMatchmaking()->SetLobbyData(currentLobby.m_steamIDLobby, "creatorID", creatorIDStr.c_str());


		//SteamMatchmaking()->RequestLobbyData(currentLobby.m_steamIDLobby);

		//use to set params paramsForMakingLobby

		cout << "created: " << lobbyName << " successfully. map path: " << paramsForMakingLobby.mapPath  << endl;
		cout << "creatorID: " << creatorIDStr << endl;

		action = A_IN_LOBBY;




		//string test = "test messageeeee";
		//SteamMatchmaking()->SendLobbyChatMsg(currentLobby.m_steamIDLobby, test.c_str(), test.length() + 1);
	}
	else
	{
		// failed, show error
		cout << "failed to create a lobby (lost connection to steam back-end servers" << endl;
		action = A_ERROR;
	}
}

bool LobbyManager::IsInLobby()
{
	return action == A_IN_LOBBY;
}

void LobbyManager::PrintLobbies()
{
	int index = 0;
	cout << "printing lobbies" << endl;
	for (auto it = lobbyVec.begin(); it != lobbyVec.end(); ++it)
	{
		cout << index << ": " << (*it).name << endl;
		++index;
	}
}

void LobbyManager::FindLobby()
{
	cout << "finding lobby" << endl;
	RefreshLobbyList();
}

void LobbyManager::OnLobbyChatUpdateCallback(LobbyChatUpdate_t *pCallback)
{
	cout << "lobby chat update callback. updated member list" << endl;

	PopulateLobbyList(pCallback->m_ulSteamIDLobby);
}

int LobbyManager::GetNumMembers()
{
	return currentLobby.memberList.size();
}


void LobbyManager::OnLobbyEnterCallback(LobbyEnter_t *pCallback)
{
	cout << "lobby enter callback" << endl;

	PopulateLobbyList(pCallback->m_ulSteamIDLobby);
}

void LobbyManager::OnLobbyDataUpdateCallback(LobbyDataUpdate_t *pCallback)
{
	cout << "lobby data update callback" << endl;

	if (pCallback->m_bSuccess)
	{
		for (auto it = lobbyVec.begin(); it != lobbyVec.end(); ++it)
		{
			if ((*it).m_steamIDLobby == pCallback->m_ulSteamIDLobby)
			{
				(*it).dataIsRetrieved = true;
				cout << "data received for lobby!" << endl;

				if (action == A_IN_LOBBY_WAITING_FOR_DATA && (*it).m_steamIDLobby == currentLobby.m_steamIDLobby)
				{
					action = A_IN_LOBBY;
				}
				break;
			}
		}
	}
	//figure this out later

	/*for (auto it = lobbyList.begin(); it != lobbyList.end(); ++it)
	{
		if ((*it).m_steamIDLobby == pCallback->m_ulSteamIDLobby)
		{

		}
	}*/
}

void LobbyManager::PopulateLobbyList( CSteamID lobbyID )
{
	currentLobby.memberList.clear();

	int numMembers = SteamMatchmaking()->GetNumLobbyMembers(lobbyID);
	CSteamID currUser;
	CSteamID myId = SteamUser()->GetSteamID();

	for (int i = 0; i < numMembers; ++i)
	{
		currUser = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyID, i);

		currentLobby.memberList.push_back(currUser);
	}
}

void LobbyManager::Update()
{
	switch (action)
	{
	case A_REQUEST_CREATE_LOBBY:
		break;
	case A_IN_LOBBY:
		break;
	case A_REQUEST_JOIN_LOBBY:
		break;
	case A_ERROR:
		break;
	}
}

void LobbyManager::LeaveLobby()
{
	SteamMatchmaking()->LeaveLobby(currentLobby.m_steamIDLobby);
	action = A_IDLE;
	cout << "leaving lobby" << endl;
}

void LobbyManager::TryJoiningLobby( int lobbyIndex )
{
	action = A_REQUEST_JOIN_LOBBY;

	cout << "found a lobby. Attempting to join: " << lobbyVec[lobbyIndex].name << endl;
	auto apiCall = SteamMatchmaking()->JoinLobby(lobbyVec[lobbyIndex].m_steamIDLobby);
	m_SteamCallResultLobbyEnter.Set(apiCall, this, &LobbyManager::OnLobbyEnter);
}

void LobbyManager::OnLobbyEnter(LobbyEnter_t *pCallback, bool bIOFailure)
{
	if (action != A_REQUEST_JOIN_LOBBY)
		return;

	if (pCallback->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess)
	{
		cout << "failed to enter lobby" << endl;
		action = A_IDLE;
		return;
	}

	if (currentLobby.dataIsRetrieved)
	{
		action = A_IN_LOBBY;
		cout << "current lobby data has already been received" << endl;
	}
	else
	{
		action = A_IN_LOBBY_WAITING_FOR_DATA;
		cout << "current lobby data has NOT been received yet" << endl;
	}
	

	// move forward the state
	currentLobby.m_steamIDLobby = pCallback->m_ulSteamIDLobby;

	for (auto it = lobbyVec.begin(); it != lobbyVec.end(); ++it)
	{
		if ((*it).m_steamIDLobby == pCallback->m_ulSteamIDLobby)
		{
			currentLobby = (*it);
			cout << "joined lobby: " << (*it).name << " successfully" << endl;
			break;
		}
	}
}

void LobbyManager::ProcessLobbyList()
{
	if (lobbyVec.empty())
	{
		/*LobbyParams lp;
		lp.maxMembers = 2;

		TryCreatingLobby(lp);*/

		action = A_FOUND_NO_LOBBIES;
	}
	else
	{
		PrintLobbies();
		action = A_FOUND_LOBBIES;
		//TryJoiningLobby();
	}
}

void LobbyManager::RefreshLobbyList()
{
	if (!m_bRequestingLobbies)
	{
		m_bRequestingLobbies = true;
		// request all lobbies for this game
		SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();
		// set the function to call when this API call has completed
		m_SteamCallResultLobbyMatchList.Set(hSteamAPICall, this, &LobbyManager::OnLobbyMatchListCallback);

		action = A_REQUEST_LOBBY_LIST;
		cout << "request lobby list" << endl;
	}
}

void LobbyManager::OnLobbyMatchListCallback(LobbyMatchList_t *pCallback, bool bIOFailure)
{
	lobbyVec.clear();
	m_bRequestingLobbies = false;

	if (bIOFailure)
	{
		// we had a Steam I/O failure - we probably timed out talking to the Steam back-end servers
		// doesn't matter in this case, we can just act if no lobbies were received
	}


	if (pCallback->m_nLobbiesMatching == 0)
	{
		cout << "no lobbies found!" << endl;
	}
	else
	{
		cout << "retrieved lobby list: " << endl;
	}

	// lobbies are returned in order of closeness to the user, so add them to the list in that order
	lobbyVec.reserve(pCallback->m_nLobbiesMatching);
	for (uint32 iLobby = 0; iLobby < pCallback->m_nLobbiesMatching; iLobby++)
	{
		CSteamID steamIDLobby = SteamMatchmaking()->GetLobbyByIndex(iLobby);

		// add the lobby to the list
		Lobby lobby;
		//lobby.createdByMe = false;
		lobby.m_steamIDLobby = steamIDLobby;

		// pull the name from the lobby metadata
		const char *pchLobbyName = SteamMatchmaking()->GetLobbyData(steamIDLobby, "name");
		if (pchLobbyName && pchLobbyName[0])
		{
			lobby.name = pchLobbyName;
		}
		else
		{
			// we don't have info about the lobby yet, request it
			//need to figure out how to use this!
			SteamMatchmaking()->RequestLobbyData(steamIDLobby);

			//lobby.
			lobby.name = "Lobby " + to_string(steamIDLobby.GetAccountID());
			// results will be returned via LobbyDataUpdate_t callback
		}

		lobbyVec.push_back(lobby);
		
	}

	ProcessLobbyList();
}

bool LobbyManager::CurrentLobbyHasMaxMembers()
{
	return false;
	//if( currentLobby.mem)
}

int LobbyManager::GetNumCurrentLobbyMembers()
{
	if (action != A_IN_LOBBY)
		return 0;

	return currentLobby.memberList.size();
}