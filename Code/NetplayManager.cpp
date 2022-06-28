#include "NetplayManager.h"
#include "VectorMath.h"
#include "GameSession.h"
#include "MatchParams.h"
#include "LobbyMessage.h"

using namespace std;
using namespace sf;

NetplayPlayer::NetplayPlayer()
{
	Clear();
}

void NetplayPlayer::Clear()
{
	connection = 0;
	isMe = false;
	isConnectedTo = false;
	doneConnectingToAllPeers = false;
	readyToRun = false;
	
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

	receivedMapLoadSignal = false;
	receivedGameStartSignal = false;

	numPlayers = -1;

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

	for (int i = 0; i < numPlayers; ++i)
	{
		if (i == playerIndex)
			continue;

		if (netplayPlayers[i].isConnectedTo)
		{
			SteamNetworkingSockets()->CloseConnection(netplayPlayers[i].connection, 0, NULL, false);
			//netplayPlayers[i].connection = 0;
			//netplayPlayers[i].isConnectedTo = false;
		}
	}

	if ( connectionManager != NULL && connectionManager->listenSocket != 0)
	{
		SteamNetworkingSockets()->CloseListenSocket(connectionManager->listenSocket);
	}

	if (connectionManager != NULL)
	{
		//connectionManager->CloseConnection();
		delete connectionManager;
		connectionManager = NULL;
	}

	for (int i = 0; i < 4; ++i)
	{
		netplayPlayers[i].Clear();
	}

	numPlayers = -1;

	action = A_IDLE;

	receivedMapLoadSignal = false;
	receivedGameStartSignal = false;
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
			numPlayers = 2;
			assert(GetHostID() == lobbyManager->currentLobby.memberList.front());
			int memberIndex = 0;
			playerIndex = -1;
			for (auto it = lobbyManager->currentLobby.memberList.begin(); it != lobbyManager->currentLobby.memberList.end(); ++it)
			{
				netplayPlayers[memberIndex].Clear();

				netplayPlayers[memberIndex].index = memberIndex;
				netplayPlayers[memberIndex].id = (*it);

				if ((*it) == GetMyID())
				{
					netplayPlayers[memberIndex].isMe = true;
					playerIndex = memberIndex;
				}

				++memberIndex;
			}

			action = A_GET_CONNECTIONS;

			if (playerIndex < numPlayers - 1)
			{
				connectionManager->CreateListenSocket();
			}

			SteamNetworkingIdentity identity;
			for (int i = playerIndex - 1; i >=0; --i)
			{
				identity.SetSteamID(netplayPlayers[i].id);

				netplayPlayers[i].connection = SteamNetworkingSockets()->ConnectP2P(identity, 0, 0, NULL);
			}
		}
		break;
	}

	case A_GET_CONNECTIONS:
	{
		bool connectedToAll = true;
		for (int i = 0; i < numPlayers; ++i)
		{
			if (i == playerIndex)
			{
				continue;
			}

			if (!netplayPlayers[i].isConnectedTo)
			{
				connectedToAll = false;
				break;
			}
		}

		//if connected to all others
		if (connectedToAll)
		{
			if (IsHost())
			{
				action = A_WAIT_FOR_ALL_TO_CONNECT;
			}
			else
			{
				PeerBroadcastDoneConnectingSignal();

				action = A_WAIT_TO_LOAD_MAP;
			}
		}
		break;
	}
	case A_WAIT_FOR_ALL_TO_CONNECT:
	{
		assert(IsHost());
		
		bool allDoneConnecting = true;
		for (int i = 0; i < numPlayers; ++i)
		{
			if (i == playerIndex)
				continue;

			if (!netplayPlayers[i].doneConnectingToAllPeers)
			{
				allDoneConnecting = false;
				break;
			}
		}

		if (allDoneConnecting)
		{
			action = A_WAIT_TO_LOAD_MAP;

			HostBroadcastLoadMapSignal();
		}
		break;
	}
	case A_WAIT_TO_LOAD_MAP:
	{
		if (receivedMapLoadSignal)
		{
			LoadMap();
		}
		break;
	}
	case A_LOAD_MAP:
	{
		if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
		{
			delete loadThread;
			loadThread = NULL;

			if (isSyncTest)
			{
				RunMatch();
			}
			else
			{
				if (IsHost())
				{
					action = A_WAIT_FOR_ALL_READY;
				}
				else
				{
					action = A_READY_TO_RUN;
					PeerBroadcastReadyToRunSignal();
				}
			}
			
		}
		break;
	}
	case A_WAIT_FOR_ALL_READY:
	{
		assert(IsHost());

		bool allReady = true;
		for (int i = 0; i < numPlayers; ++i)
		{
			if (i == playerIndex)
				continue;

			if (!netplayPlayers[i].readyToRun)
			{
				allReady = false;
				break;
			}
		}

		if (allReady)
		{
			action = A_READY_TO_RUN;

			HostBroadcastGameStartSignal();
		}
		break;
	}
	case A_READY_TO_RUN:
	{
		if (receivedGameStartSignal)
		{
			RunMatch();
		}
		break;
	}
	case A_RUNNING_MATCH:
		break;

	}
}

void NetplayManager::HostBroadcastLoadMapSignal()
{
	assert(IsHost());

	LobbyMessage msg;
	msg.mapPath = "Resources/Maps/W2/afighting1.brknk";//matchParams.mapPath.string();//
	msg.header.messageType = LobbyMessage::MESSAGE_TYPE_HOST_LOAD_MAP;

	uint8 *buffer = NULL;
	int bufferSize = msg.CreateBinaryMessage(buffer);

	SteamMatchmaking()->SendLobbyChatMsg(lobbyManager->currentLobby.m_steamIDLobby, buffer, bufferSize);

	delete[] buffer;
}

void NetplayManager::PeerBroadcastDoneConnectingSignal()
{
	LobbyMessage msg;
	msg.header.messageType = LobbyMessage::MESSAGE_TYPE_PEER_DONE_CONNECTING;

	uint8 *buffer = NULL;
	int bufferSize = msg.CreateBinaryMessage(buffer);

	SteamMatchmaking()->SendLobbyChatMsg(lobbyManager->currentLobby.m_steamIDLobby, buffer, bufferSize);

	delete[] buffer;
}

void NetplayManager::HostBroadcastGameStartSignal()
{
	LobbyMessage msg;
	msg.header.messageType = LobbyMessage::MESSAGE_TYPE_HOST_GAME_START;

	uint8 *buffer = NULL;
	int bufferSize = msg.CreateBinaryMessage(buffer);

	SteamMatchmaking()->SendLobbyChatMsg(lobbyManager->currentLobby.m_steamIDLobby, buffer, bufferSize);

	delete[] buffer;
}

void NetplayManager::PeerBroadcastReadyToRunSignal()
{
	LobbyMessage msg;
	msg.header.messageType = LobbyMessage::MESSAGE_TYPE_PEER_READY_TO_RUN;

	uint8 *buffer = NULL;
	int bufferSize = msg.CreateBinaryMessage(buffer);

	SteamMatchmaking()->SendLobbyChatMsg(lobbyManager->currentLobby.m_steamIDLobby, buffer, bufferSize);

	delete[] buffer;
}

int NetplayManager::GetConnectionIndex(HSteamNetConnection &con)
{
	int connectionIndex = -1;
	for (int i = 0; i < numPlayers; ++i)
	{
		if (i == playerIndex)
			continue;

		cout << "test: " << netplayPlayers[i].connection << ", " << con << endl;
		if (netplayPlayers[i].connection == con)
		{
			connectionIndex = i;
			break;
		}
	}

	return connectionIndex;
}

void NetplayManager::OnConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pCallback)
{
	//cout << "connection status changed callback" << endl;
	
	int connectionIndex = GetConnectionIndex(pCallback->m_hConn);
	//assert(connectionIndex >= 0);

	if (pCallback->m_eOldState == k_ESteamNetworkingConnectionState_None
		&& pCallback->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting)
	{
		if (pCallback->m_info.m_hListenSocket)
		{
			EResult result = SteamNetworkingSockets()->AcceptConnection(pCallback->m_hConn);

			for (int i = 0; i < numPlayers; ++i)
			{
				if (i == playerIndex)
					continue;

				if (netplayPlayers[i].id == pCallback->m_info.m_identityRemote.GetSteamID())
				{
					cout << "setting connection " << i << endl;
					netplayPlayers[i].connection = pCallback->m_hConn;
					connectionIndex = i;
					break;
				}
				
			}
			//pCallback->m_info.m_identityRemote

			if (result == k_EResultOK)
			{
				cout << "accepting connection to " << connectionIndex << endl;
			}
			else
			{
				cout << "failing to accept connection to " << connectionIndex << endl;
			}
		}
		else
		{
			cout << "connecting but I'm not the one with a listen socket" << endl;
		}
	}
	else if (pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connecting
		&& pCallback->m_info.m_eState == k_ESteamNetworkingConnectionState_FindingRoute)
	{
		cout << "finding route.." << endl;
	}
	else if ((pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connecting
		|| pCallback->m_eOldState == k_ESteamNetworkingConnectionState_FindingRoute)
		&& pCallback->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected)
	{
		cout << "connection to " << connectionIndex << " is complete!" << endl;

		netplayPlayers[connectionIndex].isConnectedTo = true;
		
	}
	else if ((pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connecting
		|| pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
		&& pCallback->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer)
	{
		cout << "connection closed by peer: " << pCallback->m_info.m_eEndReason << endl;

		//do I still need to close the connection?
		//SteamNetworkingSockets()->CloseConnection(connection, 0, NULL, false);
		netplayPlayers[connectionIndex].isConnectedTo = false;
		//connected = false;
	}
	else if ((pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connecting
		|| pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
		&& pCallback->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
	{
		cout << "connection state problem locally detected: " << pCallback->m_info.m_eEndReason << endl;

		netplayPlayers[connectionIndex].isConnectedTo = false;
	}
	else if (pCallback->m_eOldState == k_ESteamNetworkingConnectionState_ClosedByPeer
		&& pCallback->m_info.m_eState == k_ESteamNetworkingConnectionState_None)
	{
		//connection returned to default state
	}
	else
	{
		//cout << "state is confused" << endl;
		cout << "confused: " << "old: " << pCallback->m_eOldState << ", new state: " << pCallback->m_info.m_eState << endl;
	}
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

	if (!isSyncTest)
	{
		matchParams.numPlayers = lobbyManager->GetNumMembers();
	}
	

	assert(game == NULL);
	game = new GameSession(&matchParams);

	assert(loadThread == NULL);
	loadThread = new boost::thread(GameSession::sLoad, game);
}

int NetplayManager::RunMatch()
{
	action = A_RUNNING_MATCH;
	int gameResult = game->Run();
	delete game;
	game = NULL;

	Abort();

	//eventually needs to go to like, another game? rematches etc
	action = A_MATCH_COMPLETE;

	return gameResult;
}

void NetplayManager::FindMatch()
{
	Abort();

	if (isSyncTest)
	{
		matchParams.mapPath = "Resources/Maps/W2/afighting1.brknk";
		matchParams.numPlayers = 2;

		LoadMap();
		
		receivedGameStartSignal = true;
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

	if (msg.header.messageType == LobbyMessage::MESSAGE_TYPE_HOST_LOAD_MAP && GetHostID() == msg.sender)
	{
		assert(action == A_WAIT_TO_LOAD_MAP);
		cout << "received a message to load the map from the host" << endl;

		matchParams.mapPath = msg.mapPath;
		matchParams.numPlayers = lobbyManager->GetNumMembers();

		receivedMapLoadSignal = true;
	}
	else if (msg.header.messageType == LobbyMessage::MESSAGE_TYPE_HOST_GAME_START && GetHostID() == msg.sender)
	{
		assert(action == A_READY_TO_RUN);
		cout << "received a message to start the game from the host" << endl;

		RunMatch();
	}
	else if (msg.header.messageType == LobbyMessage::MESSAGE_TYPE_PEER_DONE_CONNECTING)
	{
		if (IsHost())
		{
			for (int i = 0; i < numPlayers; ++i)
			{
				if (i == playerIndex)
					continue;

				if (netplayPlayers[i].id == msg.sender)
				{
					netplayPlayers[i].doneConnectingToAllPeers = true;
				}
			}
		}
	}
	else if (msg.header.messageType == LobbyMessage::MESSAGE_TYPE_PEER_READY_TO_RUN)
	{
		if (IsHost())
		{
			for (int i = 0; i < numPlayers; ++i)
			{
				if (i == playerIndex)
					continue;

				if (netplayPlayers[i].id == msg.sender)
				{
					netplayPlayers[i].readyToRun = true;
				}
			}
		}
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