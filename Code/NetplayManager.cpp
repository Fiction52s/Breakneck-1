#include "NetplayManager.h"
#include "VectorMath.h"
#include "GameSession.h"
#include "MatchParams.h"
#include "LobbyMessage.h"

using namespace std;
using namespace sf;

NetplayPlayer::NetplayPlayer()
{
	connection = 0;
	isMe = false;
}

NetplayManager::NetplayManager()
{
	lobbyManager = NULL;
	connectionManager = NULL;
	loadThread = NULL;
	game = NULL;

	SetRectColor(quad, Color::Red);
	SetRectCenter(quad, 400, 400, Vector2f(960, 540));

	isSyncTest = false;

	//choose this in a separate function soon
	//matchParams.mapPath = "Resources/Maps/W2/afighting1.brknk";
	matchParams.netplayManager = this;
}

NetplayManager::~NetplayManager()
{
	Abort();
}

bool NetplayManager::IsConnected()
{
	if (isSyncTest)
	{
		return true;
	}
	else
	{
		if (connectionManager == NULL)
		{
			return false;
		}
		else
		{
			return connectionManager->connected;
		}
	}
	
}

bool NetplayManager::IsIdle()
{
	return action == A_IDLE;
}

bool NetplayManager::IsReadyToRun()
{
	return action == A_READY_TO_RUN;
}

void NetplayManager::LeaveLobby()
{
	if (lobbyManager != NULL)
		lobbyManager->LeaveLobby();
}

void NetplayManager::Abort()
{
	if (lobbyManager != NULL)
	{
		lobbyManager->LeaveLobby();
		delete lobbyManager;
		lobbyManager = NULL;
	}
	if (connectionManager != NULL)
	{
		connectionManager->CloseConnection();
		delete connectionManager;
		connectionManager = NULL;
	}
	action = A_IDLE;
}

void NetplayManager::Update()
{
	switch (action)
	{
	case A_IDLE:
		break;
	case A_GATHERING_USERS:
	{
		lobbyManager->Update();

		if (lobbyManager->GetNumCurrentLobbyMembers() == 2)
		{
			assert(GetHostID() == lobbyManager->currentLobby.memberList.front());
			int memberIndex = 0;
			playerIndex = -1;
			for (auto it = lobbyManager->currentLobby.memberList.begin(); it != lobbyManager->currentLobby.memberList.end(); ++it)
			{
				netplayPlayers[memberIndex].index = memberIndex;
				netplayPlayers[memberIndex].id = (*it);
				netplayPlayers[memberIndex].connection = 0;
				netplayPlayers[memberIndex].isMe = false;

				if ((*it) == GetMyID())
				{
					netplayPlayers[memberIndex].isMe = true;
					playerIndex = memberIndex;
				}

				++memberIndex;
			}

			action = A_GET_CONNECTIONS;

			if( playerIndex < )

			if (IsHost())
			{
				cout << "create listen socket" << endl;
				connectionManager->CreateListenSocket();
			}
			else
			{
				cout << "other test " << endl;
				//this is really bad/messy for 4 players. figure out how to do multiple p2p connections soon
				CSteamID myId = SteamUser()->GetSteamID();
				
				for (auto it = lobbyManager->currentLobby.memberList.begin(); it != lobbyManager->currentLobby.memberList.end(); ++it)
				{
					if ((*it) == myId)
					{
						continue;
					}

					cout << "try to connect" << endl;
					connectionManager->ConnectToID((*it));
				}
			}
		}
		break;
	}

	case A_GET_CONNECTIONS:
	{
		//if connected to all others
		if (connectionManager->connected)
		{
			action = A_WAIT_TO_LOAD_MAP;
			if (IsHost())
			{
				BroadcastLoadMapSignal();
			}
			//string test = "test messageeeee";
			//SteamMatchmaking()->SendLobbyChatMsg(lobbyManager->currentLobby.m_steamIDLobby, test.c_str(), test.length() + 1);
			//LoadMap();
		}
		break;
	}
	case A_WAIT_TO_LOAD_MAP:
	{
		if (receivedMapLoadSignal)
		{
			action = A_LOAD_MAP;
			LoadMap();
		}
		break;
	}
	case A_LOAD_MAP:
	{
		if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
		{
			action = A_READY_TO_RUN;
		}
		break;
	}
	case A_READY_TO_RUN:
	{
		break;
	}
	case A_RUNNING_MATCH:
		break;
	
	}
}

void NetplayManager::BroadcastLoadMapSignal()
{
	LobbyMessage msg;
	msg.mapPath = "Resources/Maps/W2/afighting1.brknk";//matchParams.mapPath.string();//
	msg.header.messageType = LobbyMessage::MESSAGE_TYPE_LOAD_MAP;

	uint8 *buffer;
	int bufferSize = msg.CreateBinaryMessage(buffer);

	SteamMatchmaking()->SendLobbyChatMsg(lobbyManager->currentLobby.m_steamIDLobby, buffer, bufferSize);

	delete[] buffer;
}

CSteamID NetplayManager::GetHostID()
{
	return SteamMatchmaking()->GetLobbyOwner(lobbyManager->currentLobby.m_steamIDLobby);
}

CSteamID NetplayManager::GetMyID()
{
	return SteamUser()->GetSteamID();
}

void NetplayManager::LoadMap()
{
	action = A_LOAD_MAP;

	/*if (IsHost())
	{
		string test = "test messageeeee";
		SteamMatchmaking()->SendLobbyChatMsg(currentLobby.m_steamIDLobby, test.c_str(), test.length() + 1);
	}*/

	matchParams.numPlayers = lobbyManager->GetNumMembers();

	assert(game == NULL);
	game = new GameSession(&matchParams);

	assert(loadThread == NULL);
	loadThread = new boost::thread(GameSession::sLoad, game);
}

void NetplayManager::FindMatch()
{
	Abort();

	if (isSyncTest)
	{
		action = A_READY_TO_RUN;
	}
	else
	{
		lobbyManager = new LobbyManager(this);
		connectionManager = new ConnectionManager;

		action = A_GATHERING_USERS;

		lobbyManager->FindLobby();
	}
}

void NetplayManager::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);
}

HSteamNetConnection NetplayManager::GetConnection()
{
	if (isSyncTest)
	{
		return 0;
	}
	else
	{
		if (connectionManager != NULL && connectionManager->connected )
		{
			return connectionManager->connection;
		}
		else
		{
			return 0;
		}
		
	}
}

bool NetplayManager::IsHost()
{
	if (isSyncTest)
	{
		return true;
	}
	else
	{
		//assert(lobbyManager->currentLobby.m_steamIDLobby != 0);
		return GetHostID() == GetMyID();

		/*if (lobbyManager != NULL)
		{
			return lobbyManager->IsLobbyCreator();
		}
		else
		{
			return true;
		}*/
	}
}

void NetplayManager::HandleMessage(LobbyMessage &msg)
{
	msg.Print();

	if (msg.header.messageType == LobbyMessage::MESSAGE_TYPE_LOAD_MAP && GetHostID() == msg.sender)
	{
		cout << "received a message to load the map from the host" << endl;

		receivedMapLoadSignal = true;
		matchParams.mapPath = msg.mapPath;
		matchParams.numPlayers = lobbyManager->GetNumMembers();
		//LoadMap();

	}
}

void NetplayManager::OnLobbyChatMessageCallback(LobbyChatMsg_t *pCallback)
{
	uint8 pvData[1024 * 4];
	int bufSize = 4 * 1024;

	SteamMatchmaking()->GetLobbyChatEntry(pCallback->m_ulSteamIDLobby, pCallback->m_iChatID, NULL, pvData, bufSize, NULL);

	LobbyMessage msg;

	msg.sender = pCallback->m_ulSteamIDUser;
	msg.SetFromBytes(pvData);

	HandleMessage(msg);
}