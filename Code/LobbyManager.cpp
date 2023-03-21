#include "LobbyManager.h"
#include <string>
#include <iostream>
#include "LobbyMessage.h"
#include "WaitingRoom.h"
#include "NetplayManager.h"

using namespace std;

LobbyData::LobbyData()
{
	gameModeType = 0;
	isWorkshopMap = false;
	randSeed = 0;
	maxMembers = 0;
	mapIndex = 0;
	publishedFileId = 0;
	creatorId = 0;
	lobbyType = LOBBYTYPE_NOT_SET;
	levelAdventureIndex = -1;
}

bool LobbyData::Update( CSteamID lobbyId )
{
	lobbyName = SteamMatchmaking()->GetLobbyData(lobbyId, "name");

	if (lobbyName == "")
	{
		cout << "error updating lobby data. lobbyname is blank" << endl;
		return false;
	}

	mapPath = SteamMatchmaking()->GetLobbyData(lobbyId, "mapPath");

	fileHash = SteamMatchmaking()->GetLobbyData(lobbyId, "fileHash");

	string gameModeTypeStr = SteamMatchmaking()->GetLobbyData(lobbyId, "gameModeType");
	gameModeType = stoi(gameModeTypeStr);

	string receivedIsWorkshopModeStr = SteamMatchmaking()->GetLobbyData(lobbyId, "isWorkshop");
	isWorkshopMap = (receivedIsWorkshopModeStr == "true");

	string receivedRandSeedStr = SteamMatchmaking()->GetLobbyData(lobbyId, "randSeed");
	randSeed = stoi(receivedRandSeedStr);

	string maxMembersStr = SteamMatchmaking()->GetLobbyData(lobbyId, "maxMembers");
	maxMembers = stoi(maxMembersStr);

	string mapIndexStr = SteamMatchmaking()->GetLobbyData(lobbyId, "mapIndex");
	mapIndex = stoi(mapIndexStr);
	
	string creatorIdStr = SteamMatchmaking()->GetLobbyData(lobbyId, "creatorID");
	creatorId = stoll(creatorIdStr);

	string lobbyTypeStr = SteamMatchmaking()->GetLobbyData(lobbyId, "lobbyType");
	lobbyType = stoi(lobbyTypeStr);

	string levelAdventureIndexStr = SteamMatchmaking()->GetLobbyData(lobbyId, "levelAdventureIndex");
	levelAdventureIndex = stoi(levelAdventureIndexStr);
	//cout << "updated lobby info: " << mapIndex << endl;
	return true;
}

void LobbyData::SetLobbyData(CSteamID lobbyId)
{
	SteamMatchmaking()->SetLobbyData(lobbyId, "name", lobbyName.c_str());

	SteamMatchmaking()->SetLobbyData(lobbyId, "mapPath", mapPath.c_str());

	SteamMatchmaking()->SetLobbyData(lobbyId, "fileHash", fileHash.c_str());

	SteamMatchmaking()->SetLobbyData(lobbyId, "gameModeType", to_string(gameModeType).c_str());

	SteamMatchmaking()->SetLobbyData(lobbyId, "randSeed", to_string(randSeed).c_str());

	SteamMatchmaking()->SetLobbyData(lobbyId, "maxMembers", to_string(maxMembers).c_str());

	SteamMatchmaking()->SetLobbyData(lobbyId, "mapIndex", to_string(mapIndex).c_str());

	//string isWorkshopStr = //to_string((int)paramsForMakingLobby.isWorkshopMap);
	if (isWorkshopMap)
	{
		SteamMatchmaking()->SetLobbyData(lobbyId, "isWorkshop", "true");
		SteamMatchmaking()->SetLobbyData(lobbyId, "publishedFileId", to_string(publishedFileId).c_str());
	}
	else
	{
		SteamMatchmaking()->SetLobbyData(lobbyId, "isWorkshop", "false");
	}

	string creatorIDStr = to_string(creatorId);
	SteamMatchmaking()->SetLobbyData(lobbyId, "creatorID", creatorIDStr.c_str());

	SteamMatchmaking()->SetLobbyData(lobbyId, "lobbyType", to_string(lobbyType).c_str());

	SteamMatchmaking()->SetLobbyData(lobbyId, "levelAdventureIndex", to_string(levelAdventureIndex).c_str());
}

//dont sore levelAdventureIndex right now
int LobbyData::GetNumStoredBytes()
{
	return lobbyName.length() + 1
		+ mapPath.length() + 1
		+ fileHash.length() + 1
		+ sizeof(gameModeType)
		+ sizeof(isWorkshopMap)
		+ sizeof(randSeed)
		+ sizeof(maxMembers)
		+ sizeof(mapIndex)
		+ sizeof(publishedFileId)
		+ sizeof(creatorId);
}

void LobbyData::StoreBytes(unsigned char *bytes)
{
	bytes += StoreString(lobbyName, bytes);
	bytes += StoreString(mapPath, bytes);
	bytes += StoreString(fileHash, bytes);
	bytes += StoreInt(gameModeType, bytes);
	bytes += StoreBool(isWorkshopMap, bytes);
	bytes += StoreInt(randSeed, bytes);
	bytes += StoreInt(maxMembers, bytes);
	bytes += StoreInt(mapIndex, bytes);
	bytes += StoreUint64(publishedFileId, bytes);
	bytes += StoreUint64(creatorId, bytes);
}

void LobbyData::SetFromBytes(unsigned char *bytes)
{
	bytes += ReadString(lobbyName, bytes);
	bytes += ReadString(mapPath, bytes);
	bytes += ReadString(fileHash, bytes);

	memcpy(&gameModeType, bytes, sizeof(gameModeType));
	bytes += sizeof(gameModeType);
	memcpy(&isWorkshopMap, bytes, sizeof(isWorkshopMap));
	bytes += sizeof(isWorkshopMap);
	memcpy(&randSeed, bytes, sizeof(randSeed));
	bytes += sizeof(randSeed);
	memcpy(&maxMembers, bytes, sizeof(maxMembers));
	bytes += sizeof(maxMembers);

	memcpy(&mapIndex, bytes, sizeof(mapIndex));
	bytes += sizeof(mapIndex);

	memcpy(&publishedFileId, bytes, sizeof(publishedFileId));
	bytes += sizeof(publishedFileId);
	memcpy(&creatorId, bytes, sizeof(creatorId));
	bytes += sizeof(creatorId);
}

int LobbyData::StoreString(std::string &myStr, unsigned char *bytes)
{
	memcpy(bytes, myStr.c_str(), myStr.length() + 1);
	return myStr.length() + 1;
}
int LobbyData::StoreInt(int &myInt, unsigned char *bytes)
{
	memcpy(&myInt, bytes, sizeof(myInt));
	bytes += sizeof(myInt);
	return sizeof(myInt);
}
int LobbyData::StoreBool(bool &myBool, unsigned char *bytes)
{
	memcpy(&myBool, bytes, sizeof(myBool));
	bytes += sizeof(myBool);
	return sizeof(myBool);
}
int LobbyData::StoreUint64(uint64 &myUint64, unsigned char *bytes)
{
	memcpy(&myUint64, bytes, sizeof(myUint64));
	bytes += sizeof(myUint64);
	return sizeof(myUint64);
}

int LobbyData::ReadString(std::string &myStr, unsigned char *bytes)
{
	myStr = string((char*)bytes);
	return myStr.length() + 1;
}

void Lobby::Set(CSteamID p_lobbyId)
{
	m_steamIDLobby = p_lobbyId;
	data.SetLobbyData(m_steamIDLobby);
}

//this will only get the personal data of the people in the lobby
//if you are actually in the lobby. but if you aren't in the lobby it will still
//add members to memberlist. works well enough for now.
void Lobby::PopulateMemberList()
{
	memberList.clear();

	int numMembers = SteamMatchmaking()->GetNumLobbyMembers(m_steamIDLobby);
	CSteamID currUser;
	CSteamID myId = SteamUser()->GetSteamID();

	for (int i = 0; i < numMembers; ++i)
	{
		currUser = SteamMatchmaking()->GetLobbyMemberByIndex(m_steamIDLobby, i);

		memberList.push_back(LobbyMember(currUser, SteamFriends()->GetFriendPersonaName(currUser)));
	}
}

LobbyManager::LobbyManager( NetplayManager *p_netMan)
{
	netplayManager = p_netMan;
	action = A_IDLE;
	m_bRequestingLobbies = false;
	currWaitingRoom = NULL;
	searchLobbyType = -1;
}

void LobbyManager::TryCreatingLobby(LobbyData &ld)
{
	if (!m_SteamCallResultLobbyCreated.IsActive())
	{
		currentLobby.data = ld;

		SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, currentLobby.data.maxMembers);
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
		currentLobby.Set(pCallback->m_ulSteamIDLobby);

		cout << "created: " << currentLobby.data.lobbyName << " successfully. map path: " << currentLobby.data.mapPath  << endl;
		//cout << "creatorID: " << creatorIDStr << endl;

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
		cout << index << ": " << (*it).data.lobbyName << endl;
		++index;
	}
}

void LobbyManager::FindQuickplayLobby()
{
	cout << "finding quickplay lobby" << endl;
	RetrieveLobbyList(LobbyData::LOBBYTYPE_QUICKPLAY, SEARCH_GET_ALL_OF_TYPE);
}

void LobbyManager::FindPracticeLobby( const std::string &mapPath )
{
	cout << "finding practice lobby" << endl;
	searchMapPath = mapPath;
	RetrieveLobbyList(LobbyData::LOBBYTYPE_PRACTICE, SEARCH_MATCH_NAME);
}

void LobbyManager::OnLobbyChatUpdateCallback(LobbyChatUpdate_t *pCallback)
{
	cout << "lobby chat update callback. updated member list" << endl;

	PopulateLobbyList(pCallback->m_ulSteamIDLobby);

	if( netplayManager != NULL && netplayManager->IsPracticeMode())
		netplayManager->OnLobbyChatUpdateCallback(pCallback);

	if (currWaitingRoom != NULL)
	{
		currWaitingRoom->OnLobbyChatUpdateCallback(pCallback);
	}	
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
		if (action == A_IN_LOBBY)
		{
			if (currentLobby.m_steamIDLobby == pCallback->m_ulSteamIDLobby)
			{
				currentLobby.dataIsRetrieved = true;
				currentLobby.data.Update(pCallback->m_ulSteamIDLobby);
				cout << "data received for current lobby" << endl;
			}
		}
		else
		{
			for (auto it = lobbyVec.begin(); it != lobbyVec.end(); ++it)
			{
				if ((*it).m_steamIDLobby == pCallback->m_ulSteamIDLobby)
				{
					(*it).dataIsRetrieved = true;
					(*it).data.Update((*it).m_steamIDLobby);
					cout << "data received for lobby!" << endl;

					if (action == A_FOUND_LOBBIES_WAITING_FOR_DATA && IsAllLobbyDataReceived())
					{
						action = A_FOUND_LOBBIES;
					}

					//if (action == A_IN_LOBBY_WAITING_FOR_DATA && (*it).m_steamIDLobby == currentLobby.m_steamIDLobby)
					//{
					//	action = A_IN_LOBBY;
					//}
					break;
				}
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

		currentLobby.memberList.push_back(LobbyMember(currUser, SteamFriends()->GetFriendPersonaName( currUser )));
	}

	if (currWaitingRoom != NULL)
	{
		currWaitingRoom->UpdateMemberList();
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
	currWaitingRoom = NULL;
	cout << "leaving lobby" << endl;
}

void LobbyManager::TryJoiningLobby( int lobbyIndex )
{
	action = A_REQUEST_JOIN_LOBBY;
	requestJoinFromInvite = false;

	cout << "found a lobby. Attempting to join: " << lobbyVec[lobbyIndex].data.lobbyName << endl;
	auto apiCall = SteamMatchmaking()->JoinLobby(lobbyVec[lobbyIndex].m_steamIDLobby);
	m_SteamCallResultLobbyEnter.Set(apiCall, this, &LobbyManager::OnLobbyEnter);
}

void LobbyManager::TryJoiningLobbyFromInvite(CSteamID id)
{
	action = A_REQUEST_JOIN_LOBBY;
	requestJoinFromInvite = true;
	auto apiCall = SteamMatchmaking()->JoinLobby(id);
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

	if (!requestJoinFromInvite)
	{
		for (auto it = lobbyVec.begin(); it != lobbyVec.end(); ++it)
		{
			if ((*it).m_steamIDLobby == pCallback->m_ulSteamIDLobby)
			{
				currentLobby = (*it);
				cout << "joined lobby: " << (*it).data.lobbyName << " successfully" << endl;
				break;
			}
		}
	}
	else
	{
		Lobby lobby;
		lobby.m_steamIDLobby = pCallback->m_ulSteamIDLobby;

		// pull the name from the lobby metadata

		if (lobby.data.Update(lobby.m_steamIDLobby))
		{
			lobby.dataIsRetrieved = true;
		}
		else
		{
			//this can never happen because you immediately have the metadata after joining
			SteamMatchmaking()->RequestLobbyData(lobby.m_steamIDLobby);
		}

		currentLobby = lobby;
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
	//currentLobby.m_steamIDLobby = pCallback->m_ulSteamIDLobby;

	/*for (auto it = lobbyVec.begin(); it != lobbyVec.end(); ++it)
	{
		if ((*it).m_steamIDLobby == pCallback->m_ulSteamIDLobby)
		{
			currentLobby = (*it);
			cout << "joined lobby: " << (*it).data.lobbyName << " successfully" << endl;
			break;
		}
	}*/
}

void LobbyManager::ProcessLobbyList()
{
	if (lobbyVec.empty())
	{
		action = A_FOUND_NO_LOBBIES;
	}
	else
	{
		vector<Lobby> copyVec = lobbyVec;
		lobbyVec.clear();

		bool addToVec = false;
		for (auto it = copyVec.begin(); it != copyVec.end(); ++it)
		{
			if ((*it).data.lobbyType == searchLobbyType)
			{
				addToVec = false;
				switch (currSearchType)
				{
				case SEARCH_GET_ALL_OF_TYPE:
				{
					addToVec = true;
					break;
				}
				case SEARCH_MATCH_NAME:
				{
					if ((*it).data.mapPath == searchMapPath)
					{
						addToVec = true;
					}
					break;
				}
				case SEARCH_MATCH_WORLD:
				{
					break;
				}
				case SEARCH_MATCH_WORLD_AND_SECTOR:
				{
					break;
				}
				case SEARCH_MATCH_WORLD_AND_SECTOR_AND_LEVEL:
				{
					break;
				}
				}

				if( addToVec )
					lobbyVec.push_back((*it));
				
			}
		}
		
		if (lobbyVec.empty())
		{
			action = A_FOUND_NO_LOBBIES;
		}
		else
		{
			PrintLobbies();

			if (IsAllLobbyDataReceived())
			{
				action = A_FOUND_LOBBIES;

				for (auto it = lobbyVec.begin(); it != lobbyVec.end(); ++it)
				{
					(*it).PopulateMemberList();
				}
			}
			else
			{
				action = A_FOUND_LOBBIES_WAITING_FOR_DATA;
			}
		}
	}
}

void LobbyManager::RetrieveLobbyList( int lobbyType, int p_searchType )//more params later for searching etc
{
	if (!m_bRequestingLobbies)
	{
		searchLobbyType = lobbyType;

		currSearchType = p_searchType;

		m_bRequestingLobbies = true;
		// request all lobbies for this game
		SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();
		// set the function to call when this API call has completed
		//m_SteamCallResultLobbyMatchList.Cancel();
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

		if (lobby.data.Update(steamIDLobby))
		{
			lobby.dataIsRetrieved = true;
		}
		else
		{
			SteamMatchmaking()->RequestLobbyData(steamIDLobby);
		}

		//to avoid caching of stuff in wrong ways
		//SteamMatchmaking()->RequestLobbyData(steamIDLobby);

		//this is where I'm working on stuff rn. Basically you have to make sure that the data is not cached,
		//because it will give you the wrong member indexes if you're the first one in and then leave and then rejoin.



		//const char *pchLobbyName = SteamMatchmaking()->GetLobbyData(steamIDLobby, "name");
		//if (pchLobbyName && pchLobbyName[0])
		//{
		//	lobby.data.name = pchLobbyName;
		//}
		//else
		//{
		//	// we don't have info about the lobby yet, request it
		//	//need to figure out how to use this!
		//	SteamMatchmaking()->RequestLobbyData(steamIDLobby);

		//	//lobby.
		//	lobby.name = "Lobby " + to_string(steamIDLobby.GetAccountID());
		//	// results will be returned via LobbyDataUpdate_t callback
		//}

		lobbyVec.push_back(lobby);
		
	}

	ProcessLobbyList();
}

bool LobbyManager::IsAllLobbyDataReceived()
{
	for (auto it = lobbyVec.begin(); it != lobbyVec.end(); ++it)
	{
		if (!(*it).dataIsRetrieved)
			return false;
	}

	return true;
}

bool LobbyManager::CurrentLobbyHasMaxMembers()
{
	if (action != A_IN_LOBBY)
		return false;

	if (GetNumMembers() < currentLobby.data.maxMembers)
	{
		return false;
	}
	
	return true;
	//if( currentLobby.mem)
}

int LobbyManager::GetNumCurrentLobbyMembers()
{
	if (action != A_IN_LOBBY)
		return 0;

	return currentLobby.memberList.size();
}

void LobbyManager::QueryPracticeLobbies()
{
	cout << "querying practice lobbies" << endl;
	RetrieveLobbyList(LobbyData::LOBBYTYPE_PRACTICE, SearchType::SEARCH_GET_ALL_OF_TYPE);
}