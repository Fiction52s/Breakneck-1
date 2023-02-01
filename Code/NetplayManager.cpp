#include "NetplayManager.h"
#include "VectorMath.h"
#include "GameSession.h"
#include "MatchParams.h"
#include "LobbyMessage.h"
#include "ggpo\network\udp_msg.h"
#include "GGPO.h"
#include <boost/filesystem.hpp>
#include "md5.h"
#include <vector>
#include <fstream>
#include "MapBrowser.h" //for MapNode
#include "MainMenu.h"
#include "globals.h"

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
	doneVerifyingMap = false;
	readyToRun = false;
	isHost = false;
	skinIndex = 0;
	hasAllData = false;
	finishedWithResultsScreen = false;

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
	playerIndex = -1;

	desyncDetected = false;//false;

	SetRectColor(quad, Color::Red);
	SetRectCenter(quad, 400, 400, Vector2f(960, 540));

	isSyncTest = false;

	Abort();

	numPlayers = -1;

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

bool NetplayManager::IsConnectedToHost()
{
	for (int i = 0; i < 4; ++i)
	{
		if (netplayPlayers[i].isHost )
		{
			if (netplayPlayers[i].isMe)
			{
				return false; //I am host.
			}

			if (netplayPlayers[i].isConnectedTo)
			{
				return true;
			}

			return false;
		}
	}

	return false;
}

void NetplayManager::Abort()
{
	if (lobbyManager != NULL)
	{
		lobbyManager->LeaveLobby();
		delete lobbyManager;
		lobbyManager = NULL;
	}

	if (loadThread != NULL)
	{
		assert(game != NULL);
		game->SetContinueLoading(false);
		loadThread->join();
		delete loadThread;
		loadThread = NULL;
		CleanupMatch();
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

	//CleanupMatch();

	previewPath = "";

	numPlayers = -1;

	desyncDetected = false;

	action = A_IDLE;

	receivedMapLoadSignal = false;
	receivedMapVerifySignal = false;
	receivedGameStartSignal = false;
	receivedStartGGPOSignal = false;
	receivedMap = false;
	waitingForMap = false;
	waitingForPreview = false;
	receivedPostOptionsSignal = false;
	receivedNextMapData = false;
	postMatchOptionReceived = -1;
}

void NetplayManager::UpdateNetplayPlayers()
{
	cout << "updating netplay players" << endl;

	int numLobbyMembers = lobbyManager->GetNumCurrentLobbyMembers();

	numPlayers = numLobbyMembers;

	matchParams.numPlayers = numPlayers; //always keep this synced up with numPlayers

	int memberIndex = 0;
	playerIndex = -1;

	map<CSteamID, HSteamNetConnection> connections;

	for (int i = 0; i < 4; ++i)
	{
		if (netplayPlayers[i].isConnectedTo)
		{
			connections[netplayPlayers[i].id] = netplayPlayers[i].connection;
		}
		netplayPlayers[i].Clear();
	}

	for (auto it = lobbyManager->currentLobby.memberList.begin(); it != lobbyManager->currentLobby.memberList.end(); ++it)
	{
		netplayPlayers[memberIndex].Clear();

		netplayPlayers[memberIndex].index = memberIndex;
		netplayPlayers[memberIndex].id = (*it).id;
		netplayPlayers[memberIndex].name = (*it).name;

		auto con = connections.find((*it).id);
		if (con != connections.end())
		{
			netplayPlayers[memberIndex].isConnectedTo = true;
			netplayPlayers[memberIndex].connection = (*con).second;
			connections.erase(con);
			cout << "found old connection for: " << memberIndex << endl;
		}

		if ((*it).id == GetMyID())
		{
			netplayPlayers[memberIndex].isMe = true;
			playerIndex = memberIndex;
		}

		++memberIndex;
	}


	SetHost();
}

bool NetplayManager::ClientCheckWorkshopMapInstalled()
{
	if (checkWorkshopMap)
	{
		MapNode mp;
		mp.publishedFileId = workshopDownloadPublishedFileId;
		mp.isWorkshop = true;
		mp.fileName = receivedMapName;

		if (mp.CheckIfFullyInstalled())
		{
			checkWorkshopMap = false;
			//mapVerified = true; //maybe verify hash later
			matchParams.mapPath = mp.filePath;
			previewPath = mp.filePath.parent_path().string() + "\\" + mp.filePath.stem().string() + ".png";

			cout << "client fully installed workshop map" << endl;

			return true;
		}
	}
	else
	{
		return true;
	}

	return false;
}

//void NetplayManager::CheckForMapAndSetMatchParams( LobbyData &lobbyData )
void NetplayManager::CheckForMapAndSetMatchParams()
{
	checkWorkshopMap = false;
	workshopDownloadPublishedFileId = 0;
	mapDownloadFilePath = "";
	receivedMapName = "";

	//LobbyData &lobbyData = *ld;//lobbyManager->currentLobby.data;
	LobbyData &lobbyData = lobbyManager->currentLobby.data;

	matchParams.gameModeType = lobbyData.gameModeType;
	matchParams.randSeed = lobbyData.randSeed;
	//matchParams.numPlayers = numPlayers;

	if (IsHost())
	{
		matchParams.mapPath = lobbyData.mapPath;
		//for host, is preview already set?
	}
	else
	{
		

		boost::filesystem::path receivedMapPath = lobbyData.mapPath;
		string receivedCreatorIDStr;
		receivedMapName = receivedMapPath.stem().string();
		mapDownloadReceivedHash = lobbyData.fileHash;

		if (lobbyData.isWorkshopMap)
		{
			checkWorkshopMap = true;
			workshopDownloadPublishedFileId = lobbyData.publishedFileId;
		}
		else
		{
			receivedCreatorIDStr = to_string(lobbyData.creatorId);
		}


		bool mapVerified = false;

		if (checkWorkshopMap)
		{
			MapNode mp;
			mp.publishedFileId = workshopDownloadPublishedFileId;
			mp.isWorkshop = true;
			mp.fileName = receivedMapName;

			if (mp.CheckIfFullyInstalled())
			{
				checkWorkshopMap = false;
				//mapVerified = true; //maybe verify hash later
				matchParams.mapPath = mp.filePath;

				previewPath = mp.filePath.parent_path().string() + "\\" + mp.filePath.stem().string() + ".png";

				cout << "item is fully installed at the outset: " << mp.filePath << endl;
			}
			else
			{
				mp.Subscribe();
			}
		}
		else
		{
			string mapFile = receivedMapPath.filename().string();

			//first check if the file exists 
			if (boost::filesystem::exists(receivedMapPath))
			{
				string myHash = md5file(receivedMapPath.string());
				bool same = (mapDownloadReceivedHash == myHash);
				cout << "received hash: " << mapDownloadReceivedHash << ", my hash: " << myHash << endl;
				if (same)
				{
					cout << "hashes match!" << endl;
					mapVerified = true;
					matchParams.mapPath = receivedMapPath;

					previewPath = receivedMapPath.parent_path().string() + "/" + receivedMapPath.stem().string() + ".png";

					cout << "client setting mapPath to: " << matchParams.mapPath << endl;
				}
				else
				{
					cout << "hashes dont match :(" << endl;
				}
			}
			else
			{
				cout << "file doesn't exist at the received path. checking other options." << endl;
			}

			if (!mapVerified)
			{
				string userDownloadsPath = "Resources/Maps/UserDownloads/";
				string downloadPath = userDownloadsPath + receivedCreatorIDStr + "/";

				if (boost::filesystem::exists(downloadPath) && boost::filesystem::is_directory(downloadPath))
				{
					vector<boost::filesystem::path> v;
					copy(boost::filesystem::directory_iterator(downloadPath), boost::filesystem::directory_iterator(), back_inserter(v));

					sort(v.begin(), v.end());

					for (vector<boost::filesystem::path>::const_iterator it(v.begin()); it != v.end(); ++it)
					{
						if (boost::filesystem::is_regular_file((*it)))
						{
							if ((*it).extension().string() == MAP_EXT)
							{
								cout << "found file in folder: " << (*it).stem().string() << endl;
								if ((*it).filename().string() == mapFile)
								{
									cout << "file matches mapName. Checking hash" << endl;

									string myHash = md5file((*it).string());
									bool same = (mapDownloadReceivedHash == myHash);

									if (same)
									{
										cout << "hashes match in download folder" << endl;
										mapVerified = true;
										matchParams.mapPath = (*it);
										previewPath = matchParams.mapPath.parent_path().string() + "/" + matchParams.mapPath.stem().string() + ".png";

										cout << "client setting mapPath to: " << matchParams.mapPath << endl;
									}
								}

							}
						}
					}

					if (!mapVerified)
					{
						mapDownloadFilePath = downloadPath + mapFile;
					}
				}
				else
				{
					if (!(boost::filesystem::exists(userDownloadsPath) && boost::filesystem::is_directory(downloadPath)))
					{
						try
						{
							cout << "making user downloads folder" << endl;
							boost::filesystem::create_directory(userDownloadsPath);
						}
						catch (boost::filesystem::filesystem_error &e)
						{
							std::cerr << e.what() << '\n';
						}
					}

					cout << "user downloads folder for: " << receivedCreatorIDStr << " doesnt exist yet. creating." << endl;
					try
					{
						boost::filesystem::create_directory(downloadPath);
					}
					catch (boost::filesystem::filesystem_error &e)
					{
						std::cerr << e.what() << '\n';
					}

					mapDownloadFilePath = downloadPath + mapFile;
				}
			}
		}
	}

}

void NetplayManager::ConnectToAll()
{
	action = A_CONNECT_TO_ALL;

	UpdateNetplayPlayers();

	if (playerIndex < numPlayers - 1)
	{
		connectionManager->CreateListenSocket();
	}

	SteamNetworkingIdentity identity;
	for (int i = playerIndex - 1; i >= 0; --i)
	{
		identity.SetSteamID(netplayPlayers[i].id);

		if (netplayPlayers[i].connection == 0)
		{
			netplayPlayers[i].connection = SteamNetworkingSockets()->ConnectP2P(identity, 0, 0, NULL);
		}
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
			LobbyData ld;
			ld.maxMembers = 2;
			//lp.gameModeType = MatchParams::GAME_MODE_FIGHT;
			ld.gameModeType = MatchParams::GAME_MODE_PARALLEL_RACE;//MatchParams::GAME_MODE_RACE;

			// set the name of the lobby if it's ours
			string lobbyName = SteamFriends()->GetPersonaName();
			lobbyName += "'s lobby";

			ld.lobbyName = lobbyName;

			ld.mapPath = "Resources/Maps/W2/afighting6" + string(MAP_EXT);

			ld.fileHash = md5file(ld.mapPath);
			ld.creatorId = 0;
			ld.randSeed = time(0);

			lobbyManager->TryCreatingLobby(ld);
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
			ConnectToAll();
			CheckForMapAndSetMatchParams();
			
		}
		break;
	}
	case A_CONNECT_TO_ALL:
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
				/*int index = 0;
				for (auto it = lobbyManager->currentLobby.memberList.begin(); it != lobbyManager->currentLobby.memberList.end(); ++it)
				{
					netplayPlayers[index].name = (*it).name;
					++index;
				}*/


				//LeaveLobby();



				SendSignalToHost(UdpMsg::Game_Client_Done_Connecting);
				//action = A_WAIT_TO_VERIFY;//A_WAIT_TO_LOAD_MAP;
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
			//LeaveLobby();
			HostStartLoading();
			//SendSignalToAllClients(UdpMsg::Game_Host_Says_Load);
			//LoadMap();
			
			//SendSignalToAllClients(UdpMsg::Game_Host_Says_Verify_Map);

			

			//action = A_WAIT_FOR_ALL_TO_VERIFY;
		}
		break;
	}
	case A_WAIT_TO_VERIFY:
	{
		if (receivedMapVerifySignal)
		{
			if (checkWorkshopMap)
			{
				MapNode mp;
				mp.publishedFileId = workshopDownloadPublishedFileId;
				mp.isWorkshop = true;
				mp.fileName = receivedMapName;

				if (mp.CheckIfFullyInstalled())
				{
					checkWorkshopMap = false;
					//mapVerified = true; //maybe verify hash later
					matchParams.mapPath = mp.filePath;
					cout << "workshop item is fully installed when asked to verify: " << mp.filePath << endl;

					SendSignalToHost(UdpMsg::Game_Client_Done_Verifying);

					//map is already verified
					action = A_WAIT_TO_LOAD_MAP;
				}
				else
				{
					uint32 itemState = SteamUGC()->GetItemState(mp.publishedFileId);
					if (!(itemState & k_EItemStateSubscribed))
					{
						cout << "subbing to item" << endl;
						SteamUGC()->SubscribeItem(mp.publishedFileId);
						cout << "map download started" << endl;
					}

					cout << "wait for map from workshop" << endl;
					action = A_WAIT_FOR_MAP_FROM_WORKSHOP;
				}
			}
			else if (mapDownloadFilePath != "")
			{
				SendSignalToHost(UdpMsg::Game_Client_Needs_Map);
				action = A_WAIT_FOR_MAP_FROM_HOST;
			}
			else
			{
				SendSignalToHost(UdpMsg::Game_Client_Done_Verifying);

				//map is already verified
				action = A_WAIT_TO_LOAD_MAP;
			}
		}
		break;
	}
	case A_WAIT_FOR_MAP_FROM_HOST:
	{
		if (receivedMap)
		{
			string myHash = md5file(mapDownloadFilePath.string());
			bool same = (mapDownloadReceivedHash == myHash);
			
			if (same)
			{
				cout << "received map from host and it was verified correctly." << endl;
				matchParams.mapPath = mapDownloadFilePath;

				mapDownloadFilePath = "";
				mapDownloadReceivedHash = "";

				SendSignalToHost(UdpMsg::Game_Client_Done_Verifying);

				action = A_WAIT_TO_LOAD_MAP;
			}
			else
			{
				cout << "received map over the network, but the hash doesn't match what it's supposed to be. requesting again." << endl;
				receivedMap = false;
				SendSignalToHost(UdpMsg::Game_Client_Needs_Map);
			}
		}
		break;
	}
	case A_WAIT_FOR_MAP_FROM_WORKSHOP:
	{
		assert(checkWorkshopMap);

		if (checkWorkshopMap)
		{
			MapNode mp;
			mp.publishedFileId = workshopDownloadPublishedFileId;
			mp.isWorkshop = true;
			mp.fileName = receivedMapName;

			if (mp.CheckIfFullyInstalled())
			{
				checkWorkshopMap = false;
				//mapVerified = true; //maybe verify hash later
				matchParams.mapPath = mp.filePath;
				cout << "workshop item is fully installed after waiting: " << mp.filePath << endl;

				SendSignalToHost(UdpMsg::Game_Client_Done_Verifying);

				//map is already verified
				action = A_WAIT_TO_LOAD_MAP;
			}
		}
		break;
	}
	case A_WAIT_FOR_ALL_TO_VERIFY:
	{
		assert(IsHost());

		bool allDoneVerifying = true;
		for (int i = 0; i < numPlayers; ++i)
		{
			if (i == playerIndex)
				continue;

			if (!netplayPlayers[i].doneVerifyingMap)
			{
				allDoneVerifying = false;
				break;
			}
		}

		//host behavior
		if (allDoneVerifying)
		{
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
				SendSignalToHost(UdpMsg::Game_Client_Done_Loading);
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
	msg.mapPath = "Resources/Maps/W2/afighting1" + string(MAP_EXT);
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
				cout << "sending signal to host at index " << i << " on connection " << netplayPlayers[i].connection << ": " << type << endl;
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

	//lobbyManager->LeaveLobby();
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
	cout << "loading map" << endl;

	assert(game == NULL);
	MainMenu::GetInstance()->gameRunType = MainMenu::GRT_FREEPLAY;
	game = new GameSession(&matchParams);

	assert(loadThread == NULL);
	loadThread = new boost::thread(GameSession::sLoad, game);
}

int NetplayManager::RunMatch()
{
	action = A_RUNNING_MATCH;

	if( !isSyncTest )
	{
		if (IsHost())
		{
			SendSignalToAllClients(UdpMsg::Game_Host_Says_Start);
		}
	}


	int gameResult = game->Run();

	resultsScreen = game->CreateResultsScreen(); //netplay always needs a results screen

	ClearClientsFinishingResultsScreen();

	//delete game;
	//game = NULL;

	bool disconnected = false;
	if (action == A_DISCONNECT)
	{
		disconnected = true;
		//eventually make this more complex to account for different failure states
	}

	//	Abort();

	if (disconnected)
	{
		action = A_DISCONNECT;
		Abort();
	}

	if (action == A_RUNNING_MATCH)
	{
		action = A_MATCH_COMPLETE;
	}
	//eventually needs to go to like, another game? rematches etc
	

	return gameResult;
}

MatchResultsScreen *NetplayManager::CreateResultsScreen()
{
	assert(game != NULL);
	return game->CreateResultsScreen();
}

void NetplayManager::CleanupMatch()
{
	if (game != NULL)
	{
		delete game;
		game = NULL;
		cout << "cleaning up map" << endl;
	}
}

void NetplayManager::FindQuickplayMatch()
{
	if (isSyncTest)
	{
		Abort();

		playerIndex = 0;

		matchParams.mapPath = "Resources/Maps/W2/afighting6" + string(MAP_EXT);
		matchParams.numPlayers = 2;
		matchParams.gameModeType = MatchParams::GAME_MODE_FIGHT;//MatchParams::GAME_MODE_PARALLEL_RACE;

		netplayPlayers[0].name = SteamFriends()->GetPersonaName();
		for (int i = 1; i < 4; ++i)
		{
			netplayPlayers[i].name = "CPU" + to_string(i - 1);
		}
		//matchParams.gameModeType = MatchParams::GAME_MODE_PARALLEL_RACE;

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
	if (isSyncTest) //if synctest is turned on for quickplay, itll ruin the ishost checks in custom matches also
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

	if ( msg.sender == GetHostID() && LobbyMessage::MESSAGE_TYPE_START_CUSTOM_MATCH)
	{
		if (IsHost())
		{
			//testing for now, host starts connecting before.
		}
		else
		{
			//all clients start the process to eventually load the game and play, starting with connecting to everyone
			ConnectToAll();
		}
		
	}
}

void NetplayManager::SendConnectToAllSignalToAllClients()
{
	SendSignalToAllClients(UdpMsg::Game_Host_Says_Connect_To_All);
}

bool NetplayManager::AllClientsHaveReceivedAllData()
{
	bool allHaveData = true;
	for (int i = 0; i < numPlayers; ++i)
	{
		if (i == playerIndex)
		{
			continue;
		}

		if (netplayPlayers[i].isConnectedTo && !netplayPlayers[i].hasAllData)
		{
			allHaveData = false;
			break;
		}
	}

	return allHaveData;
}

void NetplayManager::SendMapToClient( HSteamNetConnection connection, boost::filesystem::path &p)
{
	assert(IsHost());
	SendFileToConnection(connection, p, UdpMsg::Game_Map);
}

void NetplayManager::SendPreviewToClient(HSteamNetConnection connection, boost::filesystem::path &p)
{
	assert(IsHost());
	SendFileToConnection(connection, p, UdpMsg::Game_Preview);
}

void NetplayManager::SendFileToConnection(HSteamNetConnection connection, boost::filesystem::path &p,
	int udpMsgType)
{
	try
	{
		if (!boost::filesystem::exists(p))
		{
			cout << "file cant be sent as it doesnt exist: " << p << endl;
			return;
		}
		else
		{
			cout << "sending file to connection: " << p << endl;
		}
	}
	catch (boost::filesystem::filesystem_error &e)
	{
		cout << "file cant be sent because of file error" << endl;
		return;
	}

	UdpMsg msg((UdpMsg::MsgType)udpMsgType);

	std::ifstream is;
	is.open(p.string(), std::ios::binary);
	assert(is.is_open());

	is.seekg(0, std::ios::end);
	size_t file_size_in_byte = is.tellg();
	std::vector<char> data; // used to store text data
	data.resize(file_size_in_byte);
	is.seekg(0, std::ios::beg);
	is.read(&data[0], file_size_in_byte);
	is.close();

	int bufferSize = sizeof(UdpMsg) + file_size_in_byte;
	char*buffer = new char[bufferSize];
	char *tempBuffer = buffer;
	UdpMsg *bufferMsg = (UdpMsg*)buffer;
	*bufferMsg = msg;
	tempBuffer += sizeof(UdpMsg);

	for (int i = 0; i < file_size_in_byte; ++i)
	{
		tempBuffer[i] = data[i];
	}

	//cout << "done building buffer for sending preview" << endl;

	EResult res = SteamNetworkingSockets()->SendMessageToConnection(connection, buffer, bufferSize, k_EP2PSendReliable, NULL);

	if (res == k_EResultOK)
	{
		cout << "succeeded in sending file: " << p << endl;
		//Log("sent packet length %d to %d. res: %d\n", len, p_connection, res);
	}
	else
	{
		//cout << "failing to send packet" << endl;
	}
}

void NetplayManager::SendBufferToConnection(HSteamNetConnection connection, unsigned char *buf, int bufSize, int udpMsgType)
{
	UdpMsg msg((UdpMsg::MsgType)udpMsgType);

	int bufferSize = sizeof(UdpMsg) + bufSize + 1;
	char*buffer = new char[bufferSize];
	char *tempBuffer = buffer;
	UdpMsg *bufferMsg = (UdpMsg*)buffer;
	*bufferMsg = msg;
	tempBuffer += sizeof(UdpMsg);

	memcpy(tempBuffer, buf, bufSize);

	EResult res = SteamNetworkingSockets()->SendMessageToConnection(connection, buffer, bufferSize, k_EP2PSendReliable, NULL);

	if (res == k_EResultOK)
	{
		cout << "succeeded in sending buffer of size: " << bufSize << " with msg type: " << udpMsgType << endl;
		//Log("sent packet length %d to %d. res: %d\n", len, p_connection, res);
	}
	else
	{
		//cout << "failing to send packet" << endl;
	}
}

void NetplayManager::SendLobbyDataForNextMapToClients(LobbyData *ld)
{
	int numBytes = ld->GetNumStoredBytes();
	unsigned char *bytes = new unsigned char[numBytes];
	ld->StoreBytes(bytes);

	for (int i = 0; i < numPlayers; ++i)
	{
		if (i == playerIndex)
		{
			continue;
		}

		if (netplayPlayers[i].isConnectedTo)
		{
			SendBufferToConnection(netplayPlayers[i].connection, bytes, numBytes, UdpMsg::Game_Host_Next_Map_Data);
		}
	}

	delete[] bytes;
}

void NetplayManager::HandleMessage(HSteamNetConnection connection, SteamNetworkingMessage_t *steamMsg)
{
	const void *steamMsgData = steamMsg->GetData();
	UdpMsg *msg = (UdpMsg*)steamMsgData;

	
	uint8 hdrType = msg->hdr.type;
	switch (hdrType)
	{
	case UdpMsg::Game_Client_Needs_Preview:
	{
		SendPreviewToClient(connection, previewPath);
		break;
	}
	case UdpMsg::Game_Preview:
	{
		if (waitingForPreview)
		{
			char *previewBinary = (char*)steamMsgData + sizeof(UdpMsg);
			int sizeofPreview = steamMsg->GetSize() - sizeof(UdpMsg);

			cout << "received preview. attempting to write to file." << endl;

			previewPath = boost::filesystem::current_path().string() + "/testpreview.png";

			FILE *file = fopen(previewPath.string().c_str(), "wb");
			fwrite(previewBinary, 1, sizeofPreview, file);
			fclose(file);
			cout << "finished saving preview: " << previewPath << endl;

			action = A_IDLE;
			waitingForPreview = false;
		}
		break;
	}
	case UdpMsg::Game_Client_Needs_Map:
	{
		SendMapToClient(connection, matchParams.mapPath);
		break;
	}
	case UdpMsg::Game_Map:
	{
		if (waitingForMap)
		{
			assert(!IsHost() && mapDownloadFilePath != "");

			char *mapBinary = (char*)steamMsgData + sizeof(UdpMsg);
			int sizeOfMap = steamMsg->GetSize() - sizeof(UdpMsg);

			cout << "received game map. attempting to write to file." << endl;
			FILE *file = fopen(mapDownloadFilePath.string().c_str(), "wb");
			fwrite(mapBinary, 1, sizeOfMap, file);
			fclose(file);
			cout << "finished saving new map: " << mapDownloadFilePath << endl;

			receivedMap = true;
			waitingForMap = false;

			matchParams.mapPath = mapDownloadFilePath;
		}
		break;
	}
	case UdpMsg::Game_Client_Has_Received_All_Data:
	{
		if (IsHost())
		{
			for (int i = 0; i < numPlayers; ++i)
			{
				if (i == playerIndex)
					continue;

				if (netplayPlayers[i].connection == connection)
				{
					netplayPlayers[i].hasAllData = true;
					break;
				}
			}
		}
		else
		{
			assert(0);
		}
		break;
	}
	case UdpMsg::Game_Client_Done_Connecting:
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

			cout << "handling game done connecting message from index on connection " << connection << endl;
		}
		else
		{
			assert(0);
		}
		break;
	}
	case UdpMsg::Game_Host_Says_Verify_Map:
	{
		cout << "received a message to verify the map" << endl;
		receivedMapVerifySignal = true;
		break;
	}
	case UdpMsg::Game_Client_Done_Verifying:
	{
		if (IsHost())
		{
			for (int i = 0; i < numPlayers; ++i)
			{
				if (i == playerIndex)
					continue;

				if (netplayPlayers[i].connection == connection)
				{
					netplayPlayers[i].doneVerifyingMap = true;
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
	case UdpMsg::Game_Client_Done_Loading:
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
	case UdpMsg::Game_Host_Rematch:
	{
		assert(!IsHost());

		postMatchOptionReceived = POST_MATCH_A_REMATCH;//UdpMsg::Game_Host_Rematch;

		cout << "host sent rematch signal" << endl;
		//action = NetplayManager::A_WAIT_FOR_GGPO_SYNC;
		break;
	}
	case UdpMsg::Game_Client_Finished_Results_Screen:
	{
		if (IsHost())
		{
			for (int i = 0; i < numPlayers; ++i)
			{
				if (i == playerIndex)
					continue;

				if (netplayPlayers[i].connection == connection)
				{
					netplayPlayers[i].finishedWithResultsScreen = true;
					break;
				}
			}

			cout << "handle client finished with results screen msg: " << connection << endl;
		}
		else
		{
			assert(0);
		}
		break;
	}
	case UdpMsg::Game_Host_Show_Post_Options:
	{
		assert(!IsHost());
		receivedPostOptionsSignal = true;
		break;
	}
	case UdpMsg::Game_Host_Next_Map_Data:
	{
		assert(!IsHost());

		if (!receivedNextMapData)
		{
			unsigned char *buf = (unsigned char*)steamMsgData + sizeof(UdpMsg);
			int bufSize = steamMsg->GetSize() - sizeof(UdpMsg);
			nextMapData.SetFromBytes(buf);

			receivedNextMapData = true;
			//cout << "received game map. attempting to write to file." << endl;
			//FILE *file = fopen(mapDownloadFilePath.string().c_str(), "wb");
			//fwrite(buf, 1, bufSize, file);
			//fclose(file);
			//cout << "finished saving new map: " << mapDownloadFilePath << endl;
		}
		

		break;
	}
	case UdpMsg::Game_Host_Post_Choose_Map:
	{
		postMatchOptionReceived = POST_MATCH_A_CHOOSE_MAP;//UdpMsg::Game_Host_Post_Choose_Map;
		break;
	}
	case UdpMsg::Game_Host_Says_Connect_To_All:
	{
		assert(!IsHost());
		ConnectToAll();
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
				//cout << "sending desync check to host: " << msg.u.desync_info.x << ", " << msg.u.desync_info.y << "   : " << currGameFrame << endl;
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

void NetplayManager::TryCreateCustomLobby(LobbyData &ld)
{
	lobbyManager->TryCreatingLobby(ld);
	action = A_CUSTOM_HOST_GATHERING_USERS;
}

void NetplayManager::OnLobbyChatUpdateCallback(LobbyChatUpdate_t *pCallback)
{
	if (pCallback->m_ulSteamIDUserChanged != GetMyID().ConvertToUint64())
	{
		uint32 flags = pCallback->m_rgfChatMemberStateChange;
		if ((flags & k_EChatMemberStateChangeEntered))
		{
			cout << "member joined. updating netplay players" << endl;
			UpdateNetplayPlayers();
		}
		
	}
}

void NetplayManager::RequestPreviewFromHost()
{
	cout << "sending request for preview" << endl;
	waitingForPreview = true;
	SendSignalToHost(UdpMsg::Game_Client_Needs_Preview);
}

void NetplayManager::RequestMapFromHost()
{
	cout << "sending request for map" << endl;
	waitingForMap = true;
	SendSignalToHost(UdpMsg::Game_Client_Needs_Map);
}

void NetplayManager::SendReceivedAllDataSignalToHost()
{
	SendSignalToHost(UdpMsg::Game_Client_Has_Received_All_Data);
}

void NetplayManager::SendFinishedResultsScreenSignalToHost()
{
	SendSignalToHost(UdpMsg::Game_Client_Finished_Results_Screen);
}

void NetplayManager::ClearClientsFinishingResultsScreen()
{
	for (int i = 0; i < 4; ++i)
	{
		netplayPlayers[i].finishedWithResultsScreen = false;
	}
	receivedPostOptionsSignal = false;
}

void NetplayManager::ClearDataForNextMatch()
{
	for (int i = 0; i < 4; ++i)
	{
		netplayPlayers[i].hasAllData = false;
		netplayPlayers[i].readyToRun = false;
		netplayPlayers[i].doneLoading = false;
	}
}

void NetplayManager::HostInitiateRematch()
{
	action = NetplayManager::A_WAIT_FOR_GGPO_SYNC;//NetplayManager::A_READY_TO_RUN;
	SendSignalToAllClients(UdpMsg::Game_Host_Rematch);
}

void NetplayManager::HostFinishResultsScreen()
{
	assert(IsHost());
	cout << "host finished with results screen" << endl;
	netplayPlayers[playerIndex].finishedWithResultsScreen = true;
}

bool NetplayManager::AllPlayersHaveFinishedWithResultsScreen()
{
	assert(IsHost());
	if (!netplayPlayers[playerIndex].finishedWithResultsScreen)
	{
		return false;
	}

	bool allFinished = true;
	for (int i = 0; i < numPlayers; ++i)
	{
		if (i == playerIndex)
		{
			continue;
		}

		if (netplayPlayers[i].isConnectedTo && !netplayPlayers[i].finishedWithResultsScreen)
		{
			return false;
		}
	}

	return true;
}

bool NetplayManager::CheckResultsScreen()
{
	if (!IsHost())
	{
		return false;
	}
	//assert(IsHost());

	if (AllPlayersHaveFinishedWithResultsScreen())
	{
		cout << "tell clients to go to post options" << endl;
		SendSignalToAllClients(UdpMsg::Game_Host_Show_Post_Options);
		return true;
	}

	return false;
}

void NetplayManager::SendPostMatchChooseMapSignalToClients()
{
	SendSignalToAllClients(UdpMsg::Game_Host_Post_Choose_Map);
}

void NetplayManager::HostStartLoading()
{
	SendSignalToAllClients(UdpMsg::Game_Host_Says_Load);
	LoadMap();
}