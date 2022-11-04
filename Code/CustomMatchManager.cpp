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


using namespace sf;
using namespace std;

CustomMatchManager::CustomMatchManager()
{
	lobbyBrowser = new LobbyBrowser;
	waitingRoom = new WaitingRoom;
	
	mapOptionsPopup = new MapOptionsPopup;

	messagePopup = new MessagePopup;

	SetAction(A_IDLE);

	fromWorkshopBrowser = false;

	preErrorAction = A_IDLE;
}

CustomMatchManager::~CustomMatchManager()
{
	delete lobbyBrowser;
	delete waitingRoom;

	delete mapOptionsPopup;

	delete messagePopup;
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
	}
}

void CustomMatchManager::CreateCustomLobby()
{
	fromWorkshopBrowser = false;
	//assert(action == A_LOBBY_BROWSER);

	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	netplayManager->Init();

	mapBrowserScreen = MainMenu::GetInstance()->mapBrowserScreen;

	mapBrowserScreen->StartLocalBrowsing();
	/*if (sf::Keyboard::isKeyPressed(Keyboard::LShift))
	{
		mapBrowserScreen->StartWorkshopBrowsing();
	}
	else
	{
		mapBrowserScreen->StartLocalBrowsing();
	}*/

	
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
	TryActivateOptionsPanel(selectedMap);
}

void CustomMatchManager::BrowseCustomLobbies()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	netplayManager->Init();
	SetAction(A_LOBBY_BROWSER);
	lobbyBrowser->OpenPopup();
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
			waitingRoom->OpenPopup();
			SetAction(A_WAITING_ROOM);
		}
		else if (lobbyBrowser->action == LobbyBrowser::A_RETURN_TO_MENU)
		{
			SetAction(A_IDLE);
			netplayManager->Abort();
			return false;
		}
		break;
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

		if (mapBrowserScreen->browserHandler->chooser->action == MapBrowser::A_CANCELLED)
		{
			SetAction(A_IDLE);
			netplayManager->Abort();
			return false;
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
			SetAction(A_CREATING_LOBBY);

			cout << "creating custom lobby test: " << mapOptionsPopup->currLobbyParams->mapPath << endl;
			cout << "hash: " << mapOptionsPopup->currLobbyParams->fileHash << endl;
			cout << "creatorID: " << mapOptionsPopup->currLobbyParams->creatorID << endl;

			netplayManager->TryCreateCustomLobby(*mapOptionsPopup->currLobbyParams);
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
			waitingRoom->OpenPopup();
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

				LobbyMessage lm;
				lm.header.messageType = LobbyMessage::MESSAGE_TYPE_START_CUSTOM_MATCH;
				netplayManager->BroadcastLobbyMessage(lm);
				cout << "broadcasting start message" << endl;
			}
		}
		else
		{
			if (netplayManager->action == NetplayManager::A_GET_CONNECTIONS)
			{
				cout << "processed start message" << endl;
				waitingRoom->SetAction(WaitingRoom::A_READY_TO_START);
				SetAction(A_READY);
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
		break;
	case A_CREATING_LOBBY:
	{
		//netplayManager->Update();
		break;
	}
	case A_READY:
		break;
	}

	//return false when its time to progress the match
	return true;
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
		break;
	}
	case A_CHOOSE_MAP_OPTIONS:
	{
		mapBrowserScreen->Draw(target);
		sf::RectangleShape rect;
		rect.setFillColor(Color(0, 0, 0, 100));
		rect.setSize(Vector2f(1920, 1080));
		rect.setPosition(0, 0);
		target->draw(rect);
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
	}
}