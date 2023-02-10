#include <assert.h>
#include "CustomMatchManager.h"
#include "LobbyBrowser.h"
#include "WaitingRoom.h"
#include "MapBrowser.h"
#include "MapBrowserScreen.h"
#include "MainMenu.h"
#include "NetplayManager.h"
#include "LobbyMessage.h"
#include <fstream>
#include "md5.h"
#include "MapHeader.h"
#include "MapOptionsPopup.h"
#include "WorkshopBrowser.h"
#include "PostMatchOptionsPopup.h"
#include "PostMatchClientPopup.h"
#include "PostMatchQuickplayOptionsPopup.h"
//#include "ggpo/network/udp_msg.h"

using namespace sf;
using namespace std;

CustomMatchManager::CustomMatchManager()
{
	lobbyBrowser = new LobbyBrowser;
	waitingRoom = new WaitingRoom;
	
	mapOptionsPopup = new MapOptionsPopup(MapOptionsPopup::MODE_CREATE_LOBBY);

	messagePopup = new MessagePopup;

	postMatchPopup = new PostMatchOptionsPopup;

	postMatchQuickplayPopup = new PostMatchQuickplayOptionsPopup;

	postMatchClientPopup = new PostMatchClientPopup;

	SetAction(A_IDLE);

	fromWorkshopBrowser = false;

	preErrorAction = A_IDLE;

	currMapIndex = 0;

	nextMapMode = false;
}

CustomMatchManager::~CustomMatchManager()
{
	delete lobbyBrowser;
	delete waitingRoom;

	delete mapOptionsPopup;

	delete messagePopup;

	delete postMatchPopup;

	delete postMatchClientPopup;

	delete postMatchQuickplayPopup;
}

void CustomMatchManager::SetAction(Action a)
{
	action = a;
	frame = 0;
}

void CustomMatchManager::HandleEvent(sf::Event ev)
{
	switch (action)
	{
	case A_LOBBY_BROWSER:
		lobbyBrowser->HandleEvent(ev);
		break;
	case A_CHOOSE_MAP:
		mapBrowserScreen->HandleEvent(ev);
		//mapBrowserScreen->browserHandler->chooser->
		break;
	case A_CHOOSE_MAP_OPTIONS:
		mapOptionsPopup->HandleEvent(ev);
		break;
	case A_WAITING_ROOM:
		waitingRoom->HandleEvent(ev);
		break;
	case A_READY:
		break;
	case A_POST_MATCH_HOST:
	{
		postMatchPopup->HandleEvent(ev);
		break;
	}
	case A_POST_MATCH_CLIENT:
	{
		postMatchClientPopup->HandleEvent(ev);
		break;
	}
	case A_POST_MATCH_QUICKPLAY:
	{
		postMatchQuickplayPopup->HandleEvent(ev);
		break;
	}
	}
}

void CustomMatchManager::OpenPostMatchPopup()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	if (netplayManager->IsHost())
	{
		postMatchPopup->Start();

		SetAction(A_POST_MATCH_HOST);
	}
	else
	{
		postMatchClientPopup->Start();
		SetAction(A_POST_MATCH_CLIENT);
	}
}

void CustomMatchManager::BrowseForNextMap()
{
	currMapIndex++;

	nextMapMode = true;

	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	netplayManager->previewPath = "";
	netplayManager->matchParams.mapPath = "";

	netplayManager->ClearDataForNextMatch();

	mapBrowserScreen->StartLocalBrowsing(MapBrowser::CREATE_CUSTOM_GAME, true);
	SetAction(A_CHOOSE_MAP);
}

void CustomMatchManager::CreateCustomLobby()
{
	currMapIndex = 0;
	nextMapMode = false;
	fromWorkshopBrowser = false;
	//assert(action == A_LOBBY_BROWSER);

	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	netplayManager->Init();

	mapBrowserScreen = MainMenu::GetInstance()->mapBrowserScreen;

	mapBrowserScreen->StartLocalBrowsing(MapBrowser::CREATE_CUSTOM_GAME, true);
	
	SetAction(A_CHOOSE_MAP);
}

void CustomMatchManager::CreateCustomLobbyFromWorkshopBrowser()
{
	fromWorkshopBrowser = true;

	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	netplayManager->Init();

	mapBrowserScreen = MainMenu::GetInstance()->mapBrowserScreen;

	//SetAction(A_CHOOSE_MAP);

	if (mapBrowserScreen->browserHandler->chooser->selectedRect != NULL)
	{
		selectedMap = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;
	}

	assert(selectedMap != NULL);

	if( selectedMap->CheckIfFullyInstalled() )
	{
		TryActivateOptionsPanel(selectedMap);
	}
	else
	{
		action = A_DOWNLOADING_WORKSHOP_MAP;
		selectedMap->Subscribe();
	}
}

void CustomMatchManager::BrowseCustomLobbies()
{
	currMapIndex = 0;
	nextMapMode = false;
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	netplayManager->Init();
	SetAction(A_LOBBY_BROWSER);
	lobbyBrowser->OpenPopup();
}

void CustomMatchManager::TryEnterLobbyFromInvite( CSteamID lobbyId )
{
	cout << "TryEnterLobbyFromInvite start" << endl;
	currMapIndex = 0;
	nextMapMode = false;
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	netplayManager->Init();

	lobbyBrowser->ClearSelection();
	lobbyBrowser->ClearLobbyRects();

	//lobbyBrowser->OpenPopup();
	lobbyBrowser->TryJoinLobbyFromInvite(lobbyId);
	SetAction(A_LOBBY_BROWSER);
	cout << "TryEnterLobbyFromInvite end" << endl;
}

void CustomMatchManager::TryActivateOptionsPanel( MapNode *mp )
{
	if (mapOptionsPopup->Activate(mp) )
	{
		action = A_CHOOSE_MAP_OPTIONS;
	}
	else
	{
		messagePopup->Pop("ERROR: Map choice not valid.");
		preErrorAction = action;
		action = A_ERROR_MESSAGE;
		selectedMap = NULL;
		mapBrowserScreen->browserHandler->ClearSelection();
	}
}

bool CustomMatchManager::Update()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	switch (action)
	{
	case A_LOBBY_BROWSER:
		if (lobbyBrowser->action == LobbyBrowser::A_IN_LOBBY)
		{
			//waitingRoom->OpenPopup();
			//SetAction(A_WAITING_ROOM);
			netplayManager->UpdateNetplayPlayers();

			for (int i = 0; i < 4; ++i)
			{
				if (netplayManager->netplayPlayers[i].isHost)
				{
					if (i != netplayManager->playerIndex)
					{
						SteamNetworkingIdentity identity;
						identity.SetSteamID(netplayManager->netplayPlayers[i].id);
							
						cout << "attempting to connect to host" << endl;
						netplayManager->netplayPlayers[i].connection = SteamNetworkingSockets()->ConnectP2P(identity, 0, 0, NULL);
						break;
					}
				}
			}

			cout << "connect to hosttt action" << endl;
			

			SetAction(A_CONNECT_TO_HOST);

		}
		else if (lobbyBrowser->action == LobbyBrowser::A_RETURN_TO_MENU)
		{
			SetAction(A_IDLE);
			netplayManager->Abort();
			return false;
		}
		break;
	case A_CONNECT_TO_HOST:
	{
		for (int i = 0; i < 4; ++i)
		{
			if (netplayManager->netplayPlayers[i].isHost)
			{
				if (netplayManager->netplayPlayers[i].isConnectedTo)
				{
					cout << "connected to host" << endl;

					waitingRoom->OpenPopup();
					SetAction(A_WAITING_ROOM);
					
					netplayManager->CheckForMapAndSetMatchParams();

					if (netplayManager->matchParams.mapPath == "")
					{
						netplayManager->RequestMapFromHost();
					}
					else
					{
						waitingRoom->UpdateMapHeader(netplayManager->matchParams.mapPath.string());
					}

					if (netplayManager->previewPath == "")
					{
						netplayManager->RequestPreviewFromHost();
					}
					else
					{
						waitingRoom->SetPreview(netplayManager->previewPath.string());
					}
				}
				break;
			}
		}
		break;
	}	
	case A_CHOOSE_MAP:
		if (mapBrowserScreen->browserHandler->chooser->selectedRect != NULL)
		{
			selectedMap = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;
			

			if (mapBrowserScreen->browserHandler->CheckIfSelectedItemInstalled())
			{
				//mapOptionsPopup->Activate(boost::filesystem::relative(selectedMap->filePath).string());
				TryActivateOptionsPanel(selectedMap);
			}
			else
			{
				action = A_DOWNLOADING_WORKSHOP_MAP;
				selectedMap->Subscribe();
			}
		}

		if (nextMapMode)
		{
			
		}
		else
		{
			if (mapBrowserScreen->browserHandler->chooser->action == MapBrowser::A_CANCELLED)
			{
				SetAction(A_IDLE);
				netplayManager->Abort();
				return false;
			}
		}
		
		break;
	case A_DOWNLOADING_WORKSHOP_MAP:
	{
		if (mapBrowserScreen->browserHandler->CheckIfSelectedItemInstalled())
		{
			cout << "map download complete" << endl;

			MapNode *selectedNode = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;

			if (selectedNode == NULL)
				assert(0);

			TryActivateOptionsPanel(selectedNode);
			//boost::filesystem::relative(selectedMap->filePath).string()
		}
		break;
	}
	case A_CHOOSE_MAP_OPTIONS:
		if (mapOptionsPopup->action == MapOptionsPopup::A_HOST)
		{
			if (fromWorkshopBrowser)
			{
				MainMenu::GetInstance()->workshopBrowser->ClearAllPreviewsButSelected();

				//works for now, but this process needs to be cleaned up later for better looking loading
			}

			if (nextMapMode)
			{
				mapOptionsPopup->currLobbyData->mapIndex = currMapIndex;
				netplayManager->lobbyManager->currentLobby.data = *mapOptionsPopup->currLobbyData;
				mapOptionsPopup->currLobbyData->SetLobbyData(netplayManager->lobbyManager->currentLobby.m_steamIDLobby);
				

				//netplayManager->SendLobbyDataForNextMapToClients(mapOptionsPopup->currLobbyData);
				SetAction(A_WAITING_ROOM);

				boost::filesystem::path mapPath = mapOptionsPopup->currLobbyData->mapPath;
				string previewPath = mapPath.parent_path().string() + "\\" + mapPath.stem().string() + ".png";

				netplayManager->previewPath = previewPath;

				netplayManager->CheckForMapAndSetMatchParams();

				waitingRoom->OpenPopup();
				waitingRoom->SetPreview(previewPath);
				waitingRoom->UpdateMapHeader(mapPath.string());
				
				//send a packet of data containing a buffer of lobbyData
			}
			else
			{
				SetAction(A_CREATING_LOBBY);

				cout << "creating custom lobby test: " << mapOptionsPopup->currLobbyData->mapPath << endl;
				cout << "hash: " << mapOptionsPopup->currLobbyData->fileHash << endl;
				cout << "creatorID: " << mapOptionsPopup->currLobbyData->creatorId << endl;

				netplayManager->TryCreateCustomLobby(*mapOptionsPopup->currLobbyData);
			}
			
			//cout << "waiting room" << endl;
		}
		else if (mapOptionsPopup->action == MapOptionsPopup::A_CANCELLED)
		{
			if (fromWorkshopBrowser)
			{
				SetAction(A_IDLE);
				netplayManager->Abort();
				selectedMap = NULL;
				//mapBrowserScreen->browserHandler->ClearSelection();
				return false;
			}
			else
			{
				SetAction(A_CHOOSE_MAP);
				selectedMap = NULL;
				mapBrowserScreen->browserHandler->ClearSelection();
			}
			
		}
		break;
	case A_ERROR_MESSAGE:
	{
		if (messagePopup->action == MessagePopup::A_INACTIVE)
		{
			action = (Action)preErrorAction;
			frame = 0;
		}
		break;
	}
	case A_CREATING_LOBBY:
	{
		if (netplayManager->lobbyManager->IsInLobby())
		{
			SetAction(A_WAITING_ROOM);
			netplayManager->connectionManager->CreateListenSocket();

			boost::filesystem::path mapPath = mapOptionsPopup->currLobbyData->mapPath;
			string previewPath = mapPath.parent_path().string() + "\\" + mapPath.stem().string() + ".png";

			netplayManager->previewPath = previewPath;

			netplayManager->CheckForMapAndSetMatchParams();

			waitingRoom->OpenPopup();
			waitingRoom->SetPreview(previewPath);
			waitingRoom->UpdateMapHeader(mapPath.string());
			
		}
		break;
	}
	case A_WAITING_ROOM:
	{
		if (netplayManager->IsHost())
		{
			if (waitingRoom->action == WaitingRoom::A_STARTING)
			{
				SetAction(A_READY);

				if (!nextMapMode)
				{
					netplayManager->ConnectToAll();

					netplayManager->SendConnectToAllSignalToAllClients();
				}
				else
				{
					netplayManager->HostStartLoading();
				}
				
				//LobbyMessage lm;
				//lm.header.messageType = LobbyMessage::MESSAGE_TYPE_START_CUSTOM_MATCH;
				//netplayManager->BroadcastLobbyMessage(lm);
				//cout << "broadcasting start message" << endl;
			}
		}
		else
		{
			if (nextMapMode)
			{
				if (netplayManager->lobbyManager->currentLobby.data.mapIndex > currMapIndex )//netplayManager->receivedNextMapData)
				{
					currMapIndex = netplayManager->lobbyManager->currentLobby.data.mapIndex;
					//maybe tell the lobby manager to refresh or check for updates to the lobby if this doens't match
					
					//netplayManager->receivedNextMapData = false;

					netplayManager->CheckForMapAndSetMatchParams();

					if (netplayManager->matchParams.mapPath == "")
					{
						netplayManager->RequestMapFromHost();
					}
					else
					{
						waitingRoom->UpdateMapHeader(netplayManager->matchParams.mapPath.string());
					}

					if (netplayManager->previewPath == "")
					{
						netplayManager->RequestPreviewFromHost();
					}
					else
					{
						waitingRoom->SetPreview(netplayManager->previewPath.string());
					}
				}

				/*if (waitingRoom->action == WaitingRoom::A_READY_TO_START)
				{
					SetAction(A_READY);
				}*/
			}
			else
			{
				if (netplayManager->action == NetplayManager::A_CONNECT_TO_ALL)
				{
					//not even sure if this code gets run anymore??
					cout << "processed start message" << endl;
					waitingRoom->SetAction(WaitingRoom::A_READY_TO_START); //set back to before start button was pressed.
					SetAction(A_READY);
				}
			}
		}
		
		if( waitingRoom->action == WaitingRoom::A_LEAVE_ROOM)
		{
			if (waitingRoom->ownerID == netplayManager->GetMyID() )
			{
				SteamMatchmaking()->SetLobbyJoinable(netplayManager->lobbyManager->currentLobby.m_steamIDLobby, false);
			}
			//BrowseCustomLobbies();
			netplayManager->Abort();
			SetAction(A_IDLE);
			return false;
		}
		break;
	}
	case A_READY:
		break;
	}

	switch (action)
	{
	case A_LOBBY_BROWSER:
		lobbyBrowser->Update();
		break;
	case A_CHOOSE_MAP:
		mapBrowserScreen->Update();
		break;
	case A_CHOOSE_MAP_OPTIONS:
		mapOptionsPopup->Update();
		break;
	case A_ERROR_MESSAGE:
		messagePopup->Update();
		break;
	case A_WAITING_ROOM:
		waitingRoom->Update();

		if (!netplayManager->IsHost() && nextMapMode && waitingRoom->action == WaitingRoom::A_READY_TO_START && netplayManager->action == NetplayManager::A_IDLE)
		{
			netplayManager->action = NetplayManager::A_WAIT_TO_LOAD_MAP;
		}
		
		if (netplayManager->action == NetplayManager::A_WAIT_TO_LOAD_MAP && netplayManager->receivedMapLoadSignal)
		{
			//right before map starts loading
			SetAction(A_READY); //just added this.
		}
		break;
	case A_CREATING_LOBBY:
	{
		//netplayManager->Update();
		break;
	}
	case A_READY:
		break;
	case A_POST_MATCH_HOST:
	{
		postMatchPopup->Update();

		switch (postMatchPopup->action)
		{
		case PostMatchOptionsPopup::A_REMATCH:
			action = A_POST_MATCH_HOST_REMATCH;
			break;
		case PostMatchOptionsPopup::A_CHOOSE_MAP:
			action = A_POST_MATCH_HOST_CHOOSE_MAP;
			break;
		case PostMatchOptionsPopup::A_LEAVE:
			action = A_POST_MATCH_HOST_LEAVE;
			break;
		}	
		break;
	}
	case A_POST_MATCH_CLIENT:
	{
		postMatchClientPopup->Update();

		if (postMatchClientPopup->action == PostMatchClientPopup::A_LEAVE)
		{
			action = A_POST_MATCH_CLIENT_LEAVE;
		}
		break;
	}
	case A_POST_MATCH_QUICKPLAY:
	{
		postMatchQuickplayPopup->Update();

		if (postMatchQuickplayPopup->action == PostMatchQuickplayOptionsPopup::A_LEAVE)
		{
			action = A_POST_MATCH_QUICKPLAY_LEAVE;
		}
		else if (postMatchQuickplayPopup->action == PostMatchQuickplayOptionsPopup::A_KEEP_PLAYING)
		{
			action = A_POST_MATCH_QUICKPLAY_VOTE_KEEP_PLAYING;
		}
		break;
	}
	case A_POST_MATCH_QUICKPLAY_VOTE_KEEP_PLAYING:
	{
		break;
	}
	case A_POST_MATCH_QUICKPLAY_VOTE_KEEP_PLAYING_WAIT_FOR_OTHERS:
	{
		break;
	}
	}

	//testing this
	netplayManager->Update();

	//return false when its time to progress the match
	return true;
}

void CustomMatchManager::DrawPopupBG(sf::RenderTarget *target)
{
	sf::RectangleShape rect;
	rect.setFillColor(Color(0, 0, 0, 100));
	rect.setSize(Vector2f(1920, 1080));
	rect.setPosition(0, 0);
	target->draw(rect);
}

void CustomMatchManager::Draw(sf::RenderTarget *target)
{
	switch (action)
	{
	case A_LOBBY_BROWSER:
		lobbyBrowser->panel->Draw(target);
		break;
	case A_CHOOSE_MAP:
		mapBrowserScreen->Draw(target);
		break;
	case A_DOWNLOADING_WORKSHOP_MAP:
	{
		mapBrowserScreen->Draw(target);
		break;
	}
	case A_CHOOSE_MAP_OPTIONS:
	{
		mapBrowserScreen->Draw(target);
		DrawPopupBG(target);
		mapOptionsPopup->Draw(target);
		break;
	}
	case A_ERROR_MESSAGE:
		messagePopup->Draw(target);
		break;
	case A_CREATING_LOBBY:
	{
		mapBrowserScreen->Draw(target);
		mapOptionsPopup->Draw(target);
		break;
	}	
	case A_WAITING_ROOM:
		waitingRoom->Draw(target);
		break;
	case A_READY:
		break;
	case A_POST_MATCH_HOST:
	{
		postMatchPopup->Draw(target);
		break;
	}
	case A_POST_MATCH_CLIENT:
	{
		postMatchClientPopup->Draw(target);
		break;
	}
	case A_POST_MATCH_QUICKPLAY:
	{
		postMatchQuickplayPopup->Draw(target);
		break;
	}
	case A_POST_MATCH_QUICKPLAY_VOTE_KEEP_PLAYING:
	{
		postMatchQuickplayPopup->Draw(target);
		break;
	}
	case A_POST_MATCH_QUICKPLAY_VOTE_KEEP_PLAYING_WAIT_FOR_OTHERS:
	{
		postMatchQuickplayPopup->Draw(target);
		break;
	}
	}
}

void CustomMatchManager::StartClientWaitingRoomForNextMap()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	//currMapIndex++; //remain at the current value until the lobby makes you change it

	nextMapMode = true;

	cout << "client is now set to next map mode!" << endl;


	netplayManager->receivedMapLoadSignal = false;
	netplayManager->receivedMapVerifySignal = false;
	netplayManager->receivedGameStartSignal = false;
	netplayManager->receivedStartGGPOSignal = false;
	netplayManager->receivedMap = false;
	netplayManager->waitingForMap = false;
	netplayManager->waitingForPreview = false;
	netplayManager->receivedPostOptionsSignal = false;
	netplayManager->receivedNextMapData = false;

	netplayManager->action = NetplayManager::A_IDLE;

	netplayManager->previewPath = "";
	netplayManager->matchParams.mapPath = "";

	waitingRoom->OpenPopup();

	SetAction(A_WAITING_ROOM);

	/*netplayManager->CheckForMapAndSetMatchParams(netplayManager->);

	if (netplayManager->matchParams.mapPath == "")
	{
		netplayManager->RequestMapFromHost();
	}
	else
	{
		waitingRoom->UpdateMapHeader(netplayManager->matchParams.mapPath.string());
	}

	if (netplayManager->previewPath == "")
	{
		netplayManager->RequestPreviewFromHost();
	}
	else
	{
		waitingRoom->SetPreview(netplayManager->previewPath.string());
	}*/

}