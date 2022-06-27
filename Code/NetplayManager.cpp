#include "NetplayManager.h"
#include "VectorMath.h"
#include "GameSession.h"
#include "MatchParams.h"

using namespace std;
using namespace sf;

NetplayManager::NetplayManager()
{
	lobbyManager = NULL;
	connectionManager = NULL;

	SetRectColor(quad, Color::Red);
	SetRectCenter(quad, 400, 400, Vector2f(960, 540));

	isSyncTest = false;
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

void NetplayManager::RunMatch(MatchParams *mp)
{
	if (IsReadyToRun())
	{
		cout << "running match" << endl;
		//cout << "leaving lobby and starting the game" << endl;
		action = A_RUNNING_MATCH;

		//MatchParams mp;
		//mp.netplayManager = this;
		//mp.numPlayers = 2;
		//mp.filePath = "Resources/Maps/W2/afighting1.brknk";
		game = new GameSession(mp);

		GameSession::sLoad(game);
		game->Run();

		delete game;
		game = NULL;

		//Abort();
		//connectionManager->CloseConnection();
		action = A_IDLE;
	}
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
			action = A_GET_CONNECTIONS;

			if (lobbyManager->currentLobby.createdByMe)
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
		if (connectionManager->connected)
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

void NetplayManager::FindMatch()
{
	Abort();

	if (isSyncTest)
	{
		action = A_READY_TO_RUN;
	}
	else
	{
		lobbyManager = new LobbyManager;
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

bool NetplayManager::IsLobbyCreator()
{
	if (isSyncTest)
	{
		return true;
	}
	else
	{
		if (lobbyManager != NULL)
		{
			return lobbyManager->IsLobbyCreator();
		}
		else
		{
			return true;
		}
	}
}