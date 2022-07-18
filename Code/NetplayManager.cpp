#include "NetplayManager.h"
#include "VectorMath.h"
#include "GameSession.h"
#include "MatchParams.h"
#include "LobbyMessage.h"
#include "ggpo\network\udp_msg.h"
#include "GGPO.h"

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
	isHost = false;
	skinIndex = 0;

	//memset(desyncCheckInfoArray, 0, sizeof(DesyncCheckInfo) * MAX_DESYNC_CHECK_INFOS_STORED);
}

void NetplayPlayer::AddDesyncCheckInfo(DesyncCheckInfo &dci)
{
	for (int i = NetplayPlayer::MAX_DESYNC_CHECK_INFOS_STORED - 1; i >= 1 ; --i)
	{
		desyncCheckInfoArray[i] = desyncCheckInfoArray[i - 1];
	}
	desyncCheckInfoArray[0] = dci;
}

void NetplayPlayer::RemoveDesyncCheckInfo()
{
	for (int i = 0; i < MAX_DESYNC_CHECK_INFOS_STORED-1; ++i)
	{
		desyncCheckInfoArray[i] = desyncCheckInfoArray[i + 1];
	}
	desyncCheckInfoArray[MAX_DESYNC_CHECK_INFOS_STORED-1] = DesyncCheckInfo();
}



const DesyncCheckInfo & NetplayPlayer::GetDesyncCheckInfo(int framesAgo)
{
	assert(framesAgo >= 0 && framesAgo < MAX_DESYNC_CHECK_INFOS_STORED);
	return desyncCheckInfoArray[framesAgo];
}

void NetplayPlayer::DumpDesyncInfo()
{
	cout << "dumping for player: " << index << endl;
	for (int i = 0; i < MAX_DESYNC_CHECK_INFOS_STORED; ++i)
	{
		DesyncCheckInfo &dci = desyncCheckInfoArray[i];
		cout << "i: " << i << ", frame: " << dci.gameFrame << ", pos: " << dci.pos.x << ", " << dci.pos.y << ", action: " << dci.action << ", actionframe: " << dci.actionFrame << ", health: " << dci.health << "\n";
	}
}

NetplayManager::NetplayManager()
{
	lobbyManager = NULL;
	connectionManager = NULL;
	loadThread = NULL;
	game = NULL;

	desyncDetected = false;

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

void NetplayManager::Init()
{
	Abort();

	lobbyManager = new LobbyManager;
	connectionManager = new ConnectionManager;
}

void NetplayManager::Abort()
{
	if (lobbyManager != NULL)
	{
		lobbyManager->LeaveLobby();
		delete lobbyManager;
		lobbyManager = NULL;
	}

	//there might be something off with this, i keep getting exit crashes right after this
	for (int i = 0; i < numPlayers; ++i)
	{
		if (i == playerIndex)
			continue;

		if (netplayPlayers[i].isConnectedTo)
		{
			SteamNetworkingSockets()->CloseConnection(netplayPlayers[i].connection, 0, NULL, false);
			netplayPlayers[i].isConnectedTo = false;
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

	desyncDetected = false;

	action = A_IDLE;

	receivedMapLoadSignal = false;
	receivedGameStartSignal = false;
}

void NetplayManager::StartConnecting()
{
	int numLobbyMembers = lobbyManager->GetNumCurrentLobbyMembers();

	numPlayers = numLobbyMembers; //2
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

	matchParams.mapPath = SteamMatchmaking()->GetLobbyData(lobbyManager->currentLobby.m_steamIDLobby, "mapPath");
	cout << "setting mapPath to: " << matchParams.mapPath << endl;

	matchParams.numPlayers = numLobbyMembers;

	action = A_GET_CONNECTIONS;

	if (playerIndex < numPlayers - 1)
	{
		connectionManager->CreateListenSocket();
	}

	SteamNetworkingIdentity identity;
	for (int i = playerIndex - 1; i >= 0; --i)
	{
		identity.SetSteamID(netplayPlayers[i].id);

		netplayPlayers[i].connection = SteamNetworkingSockets()->ConnectP2P(identity, 0, 0, NULL);
	}

	SetHost();
}


void NetplayManager::Update()
{
	switch (action)
	{
	case A_IDLE:
		break;
	case A_QUICKPLAY_CHECKING_FOR_LOBBIES:
	{
		if (lobbyManager->action == LobbyManager::A_FOUND_LOBBIES)
		{
			lobbyManager->TryJoiningLobby(0);
			action = A_QUICKPLAY_GATHERING_USERS;

		}
		else if (lobbyManager->action == LobbyManager::A_FOUND_NO_LOBBIES)
		{
			LobbyParams lp;
			lp.maxMembers = 2;

			int r = rand() % 2;
			if (r == 0)
			{
				lp.mapPath = "Resources/Maps/W2/afighting1.brknk";
			}
			else
			{
				lp.mapPath = "Resources/Maps/W2/afighting2.brknk";
			}
			lobbyManager->TryCreatingLobby(lp);
			action = A_QUICKPLAY_GATHERING_USERS;
		}
		break;
	}
	case A_QUICKPLAY_GATHERING_USERS:
	{
		lobbyManager->Update();

		int numLobbyMembers = lobbyManager->GetNumCurrentLobbyMembers();
		if (numLobbyMembers == 2)
		{
			StartConnecting();
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
				LeaveLobby();
				SendSignalToHost(UdpMsg::Game_Done_Connecting);
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

		//host behavior
		if (allDoneConnecting)
		{
			action = A_WAIT_TO_LOAD_MAP;

			LeaveLobby();

			SendSignalToAllClients(UdpMsg::Game_Host_Says_Load);

			LoadMap();
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

			game->InitGGPO();

			if (isSyncTest)
			{
				action = A_READY_TO_RUN;
			}
			else
			{
				action = A_WAIT_FOR_GGPO_SYNC;
				if (IsHost())
				{
					//action = A_WAIT_FOR_ALL_READY;
				}
				else
				{
					//action = A_READY_TO_RUN;
					//SendSignalToHost(UdpMsg::Game_Done_Loading);
				}
			}
		}
		break;
	}
	case A_WAIT_FOR_GGPO_SYNC:
	{
		game->UpdateJustGGPO();

		if (game->ggpoReady)
		{
			cout << "ggpo is readyy" << endl;
			if( IsHost())
			{
				action = A_WAIT_FOR_ALL_READY;
			}
			else
			{
				action = A_WAITING_FOR_START_MESSAGE;
				SendSignalToHost(UdpMsg::Game_Done_Loading);
			}
		}
		break;
	}
	case A_WAITING_FOR_START_MESSAGE:
	{

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
			//RunMatch();
		}
		break;
	}
	case A_READY_TO_RUN:
	{
		/*if (receivedGameStartSignal)
		{
			RunMatch();
		}*/
		break;
	}
	case A_RUNNING_MATCH:
		break;
	}

	ReceiveMessages();
}

void NetplayManager::ReceiveMessages()
{
	SteamNetworkingMessage_t *messages[1];

	//Session *sess = Session::GetSession();

	for (int i = 0; i < 4; ++i)
	{
		if (i == playerIndex)
		{
			continue;
		}

		if (netplayPlayers[i].isConnectedTo)
		{
			for (;;)
			{
				int numMsges = SteamNetworkingSockets()->ReceiveMessagesOnConnection(netplayPlayers[i].connection, messages, 1);

				if (numMsges == 1)
				{
					UdpMsg *msg = (UdpMsg *)messages[0]->GetData();

					
					
					//cout << "received messageeee" << endl;
					if (msg->IsGameMsg())
					{
						HandleMessage(netplayPlayers[i].connection, messages[0]);
						//messages[0]->Release();
					}
					else
					{
						//ggpo message(s) that we received before our own ggpo is up and running
						ggpoMessageQueue.push_back(messages[0]);

						//cout << "queueing message with type: " << (int)msg->hdr.type << endl;
					}

					

				}
				else
				{
					break;
				}
			}

		}
	}
}

void NetplayManager::BroadcastLobbyMessage(LobbyMessage &msg)
{
	uint8 *buffer = NULL;
	int bufferSize = msg.CreateBinaryMessage(buffer);

	SteamMatchmaking()->SendLobbyChatMsg(lobbyManager->currentLobby.m_steamIDLobby, buffer, bufferSize);

	delete[] buffer;
}

void NetplayManager::BroadcastMapDetailsToLobby()
{
	assert(IsHost());

	LobbyMessage msg;
	msg.mapPath = "Resources/Maps/W2/afighting1.brknk";//matchParams.mapPath.string();//
	msg.header.messageType = LobbyMessage::MESSAGE_TYPE_SHARE_MAP_DETAILS;

	uint8 *buffer = NULL;
	int bufferSize = msg.CreateBinaryMessage(buffer);

	SteamMatchmaking()->SendLobbyChatMsg(lobbyManager->currentLobby.m_steamIDLobby, buffer, bufferSize);

	delete[] buffer;
}

void NetplayManager::SendUdpMsg( HSteamNetConnection con, UdpMsg *msg)
{
	//(char *)entry.msg, entry.msg->PacketSize(), 0, entry.connection

	//cout << "sending msg thru netplay manager: " << msg->hdr.type << endl;
	EResult res = SteamNetworkingSockets()->SendMessageToConnection(con, (char*)msg, msg->PacketSize(), k_EP2PSendReliable, NULL);

	if (res == k_EResultOK)
	{
		//Log("sent packet length %d to %d. res: %d\n", len, p_connection, res);
	}
	else
	{
		//cout << "failing to send packet" << endl;
	}
}

void NetplayManager::SendSignalToHost(int type)
{
	assert(!IsHost());
	UdpMsg msg((UdpMsg::MsgType)type);

	cout << "attempt to send msg to host " << type << endl;
	HSteamNetConnection con = 0;
	for (int i = 0; i < 4; ++i)
	{
		if (netplayPlayers[i].isHost)
		{
			if (i != playerIndex)
			{
				SendUdpMsg(netplayPlayers[i].connection, &msg);
				cout << "sending signal to host: " << type << endl;
			}
			break;
		}
	}
}

void NetplayManager::SendSignalToAllClients(int type)
{
	assert(IsHost());
	UdpMsg msg((UdpMsg::MsgType)type);

	HSteamNetConnection con = 0;
	cout << "sending signal to clients: " << type << endl;
	for (int i = 0; i < 4; ++i)
	{
		if (!netplayPlayers[i].isHost)
		{
			assert(i != playerIndex);
			SendUdpMsg(netplayPlayers[i].connection, &msg);
		}
	}	
}

HSteamNetConnection NetplayManager::GetHostConnection()
{
	HSteamNetConnection con = 0;
	for (int i = 0; i < 4; ++i)
	{
		if (netplayPlayers[i].isHost)
		{
			if (i != playerIndex)
			{
				con = netplayPlayers[i].connection;
			}
			break;
		}
	}

	return con;
}

int NetplayManager::GetConnectionIndex(HSteamNetConnection &con)
{
	int connectionIndex = -1;
	for (int i = 0; i < numPlayers; ++i)
	{
		if (i == playerIndex)
			continue;

		//cout << "test: " << netplayPlayers[i].connection << ", " << con << endl;
		if (netplayPlayers[i].connection == con)
		{
			connectionIndex = i;
			break;
		}
	}

	return connectionIndex;
}

void NetplayManager::SetHost()
{
	for (int i = 0; i < numPlayers; ++i)
	{
		netplayPlayers[i].isHost = false;
		if (GetHostID() == netplayPlayers[i].id)
		{
			netplayPlayers[i].isHost = true;
			break;
		}
	}

	lobbyManager->LeaveLobby();
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

		action = A_DISCONNECT;
		//Abort();
		//for now, just close the match when anyone quits. eventually be able to handle people leaving in certain
		//game modes, as long as they aren't the host.

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

	//if (!isSyncTest)
	//{
	//	matchParams.numPlayers = 2;//lobbyManager->GetNumMembers();
	//}
	

	assert(game == NULL);
	game = new GameSession(&matchParams);

	assert(loadThread == NULL);
	loadThread = new boost::thread(GameSession::sLoad, game);
}

int NetplayManager::RunMatch()
{
	action = A_RUNNING_MATCH;



	if (isSyncTest)
	{
		game->InitGGPO();
	}
	else
	{
		if (IsHost())
		{
			SendSignalToAllClients(UdpMsg::Game_Host_Says_Start);
		}
	}


	int gameResult = game->Run();
	delete game;
	game = NULL;

	bool disconnected = false;
	if (action == A_DISCONNECT)
	{
		disconnected = true;
		//eventually make this more complex to account for different failure states
	}

	Abort();

	if (disconnected)
	{
		action = A_DISCONNECT;
	}

	if (action == A_RUNNING_MATCH)
	{
		action = A_MATCH_COMPLETE;
	}
	//eventually needs to go to like, another game? rematches etc
	

	return gameResult;
}

void NetplayManager::FindQuickplayMatch()
{
	if (isSyncTest)
	{
		Abort();

		matchParams.mapPath = "Resources/Maps/W2/afighting1.brknk";
		matchParams.numPlayers = 2;

		LoadMap();
		
		receivedGameStartSignal = true;
	}
	else
	{
		Init();

		action = A_QUICKPLAY_CHECKING_FOR_LOBBIES;

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

void NetplayManager::AddDesyncCheckInfo( int pIndex, DesyncCheckInfo &dci )
{
	netplayPlayers[pIndex].AddDesyncCheckInfo(dci);
	//cout << "add desync check for player: " << pIndex << "\n";
	//cout << "action: " << dci.action << ", " << dci.actionFrame << endl;
	//netplayPlayers[pIndex].desyncCheckInfoArray[0] = 
	
}

void NetplayManager::RemoveDesyncCheckInfos(int numRollbackFrames)
{
	int numRemovals = numRollbackFrames;
	for (int i = 0; i < 4; ++i)
	{
		for (int r = 0; r < numRemovals; ++r)
		{
			netplayPlayers[i].RemoveDesyncCheckInfo();
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
		if (lobbyManager == NULL)
		{
			assert(0);
		}

		if (lobbyManager->action == LobbyManager::A_IN_LOBBY)
		{
			return GetHostID() == GetMyID();
		}
		else
		{
			return netplayPlayers[playerIndex].isHost;
		}
	}
}

void NetplayManager::HandleLobbyMessage(LobbyMessage &msg)
{
	msg.Print();

	//still need a lobby message to set our game name

	/*if (msg.header.messageType == LobbyMessage::MESSAGE_TYPE_SHARE_MAP_DETAILS && GetHostID() == msg.sender)
	{
		assert(action == A_WAIT_TO_LOAD_MAP);
		cout << "received a message on which map we're playing" << endl;

		matchParams.mapPath = msg.mapPath;
		matchParams.numPlayers = lobbyManager->GetNumMembers();
	}*/

	if ( !IsHost() && msg.sender == GetHostID() && LobbyMessage::MESSAGE_TYPE_START_CUSTOM_MATCH)
	{
		StartConnecting();
	}
}

void NetplayManager::HandleMessage(HSteamNetConnection connection, SteamNetworkingMessage_t *steamMsg)
{
	UdpMsg *msg = (UdpMsg*)steamMsg->GetData();

	
	uint8 hdrType = msg->hdr.type;
	switch (hdrType)
	{
	case UdpMsg::Game_Done_Connecting:
	{
		if (IsHost())
		{
			for (int i = 0; i < numPlayers; ++i)
			{
				if (i == playerIndex)
					continue;

				if (netplayPlayers[i].connection == connection)
				{
					netplayPlayers[i].doneConnectingToAllPeers = true;
					break;
				}
			}

			cout << "handling game done connecting message" << endl;
		}
		else
		{
			assert(0);
		}
		break;
	}
	case UdpMsg::Game_Host_Says_Load:
	{
		cout << "received a message to load the map from the host" << endl;
		receivedMapLoadSignal = true;
		break;
	}
	case UdpMsg::Game_Done_Loading:
	{
		if (IsHost())
		{
			for (int i = 0; i < numPlayers; ++i)
			{
				if (i == playerIndex)
					continue;

				if (netplayPlayers[i].connection == connection)
				{
					netplayPlayers[i].readyToRun = true;
					break;
				}
			}

			cout << "handling gamd done loading message" << endl;
		}
		else
		{
			assert(0);
		}
		break;
	}
	case UdpMsg::Game_Host_Says_Start:
	{
		assert(!IsHost());
		cout << "received a message to start the game from the host" << endl;
		assert(action == A_WAITING_FOR_START_MESSAGE);

		action = A_READY_TO_RUN;
		//steamMsg->Release();
		//steamMsg = NULL; //set to NULL to avoid double delete
		//RunMatch();
		break;
	}
	}

	if (steamMsg != NULL )
	{
		steamMsg->Release();
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

	HandleLobbyMessage(msg);
}

void NetplayManager::SendDesyncCheckToHost( int currGameFrame )
{
	assert(!IsHost());

	UdpMsg msg(UdpMsg::Game_Desync_Check);

	const DesyncCheckInfo &dci = netplayPlayers[playerIndex].GetDesyncCheckInfo(0);
	msg.u.desync_info.x = dci.pos.x;
	msg.u.desync_info.y = dci.pos.y;

	msg.u.desync_info.frame_number = currGameFrame;

	msg.u.desync_info.player_action = dci.action;
	msg.u.desync_info.player_action_frame = dci.actionFrame;

	msg.u.desync_info.health = dci.health;

	//cout << "sending "
	HSteamNetConnection con = 0;
	for (int i = 0; i < 4; ++i)
	{
		if (netplayPlayers[i].isHost)
		{
			if (i != playerIndex)
			{
				SendUdpMsg(netplayPlayers[i].connection, &msg);
				cout << "sending desync check to host: " << msg.u.desync_info.x << ", " << msg.u.desync_info.y << "   : " << currGameFrame << endl;
			}
			break;
		}
	}
}

const DesyncCheckInfo & NetplayManager::GetDesyncCheckInfo(SteamNetworkingMessage_t *steamMsg, int framesAgo)
{
	UdpMsg *msg = (UdpMsg*)steamMsg->GetData();

	int targetPlayerIndex = -1;
	for (int i = 0; i < 4; ++i)
	{
		if (i == playerIndex)
			continue;

		if (netplayPlayers[i].connection == steamMsg->GetConnection())
		{
			targetPlayerIndex = i;
			break;
		}
	}

	assert(targetPlayerIndex >= 0);

	return netplayPlayers[targetPlayerIndex].GetDesyncCheckInfo( framesAgo );
}

void NetplayManager::DumpDesyncInfo()
{
	for (int i = 0; i < numPlayers; ++i)
	{
		netplayPlayers[i].DumpDesyncInfo();
	}
}

void NetplayManager::TryCreateCustomLobby(LobbyParams &lp)
{
	lobbyManager->TryCreatingLobby(lp);
	action = A_CUSTOM_HOST_GATHERING_USERS;
}