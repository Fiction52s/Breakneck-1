#include "Quickplayer.h"
#include "LobbyManager.h"
#include "ConnectionManager.h"
#include "VectorMath.h"

using namespace std;
using namespace sf;

Quickplayer::Quickplayer()
{
	lobbyManager = NULL;
	connectionManager = NULL;

	SetRectColor(quad, Color::Red);
	SetRectCenter(quad, 400, 400, Vector2f(960, 540));
}

Quickplayer::~Quickplayer()
{
	if (lobbyManager != NULL)
	{
		delete lobbyManager;
	}
	if (connectionManager != NULL)
	{
		delete connectionManager;
	}
}

void Quickplayer::Update()
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
			//lobbyManager->LeaveLobby(); //need to see if this causes problems or not. I don't think so.

			//InitGGPO(); //call this once I have the connections ready.
			//SetActionRunGame();
			cout << "leaving lobby and starting the game" << endl;
			action = RUNNING_MATCH;

			//make netplay gamesession
		}
		break;
	}
		
	case RUNNING_MATCH:
		break;
	}
}

void Quickplayer::FindMatch()
{
	if (lobbyManager != NULL)
	{
		delete lobbyManager;
	}
	if (connectionManager != NULL)
	{
		delete connectionManager;
	}

	lobbyManager = new LobbyManager;
	connectionManager = new ConnectionManager;

	action = A_GATHERING_USERS;

	lobbyManager->FindLobby();
}

void Quickplayer::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);
}