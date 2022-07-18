#include <assert.h>
#include "CustomMatchManager.h"
#include "LobbyBrowser.h"
#include "WaitingRoom.h"
#include "MapBrowser.h"
#include "MapBrowserScreen.h"
#include "MainMenu.h"
#include "NetplayManager.h"
#include "LobbyMessage.h"


using namespace sf;
using namespace std;

CustomMatchManager::CustomMatchManager()
{
	lobbyBrowser = new LobbyBrowser;
	waitingRoom = new WaitingRoom;
	
	mapOptionsPopup = new MapOptionsPopup;

	SetAction(A_IDLE);
}

CustomMatchManager::~CustomMatchManager()
{
	delete lobbyBrowser;
	delete waitingRoom;
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
	//assert(action == A_LOBBY_BROWSER);

	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	netplayManager->Init();

	mapBrowserScreen = MainMenu::GetInstance()->mapBrowserScreen;
	mapBrowserScreen->StartLocalBrowsing();
	SetAction(A_CHOOSE_MAP);
}

void CustomMatchManager::BrowseCustomLobbies()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	netplayManager->Init();
	SetAction(A_LOBBY_BROWSER);
	lobbyBrowser->OpenPopup();
}

bool CustomMatchManager::Update()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	switch (action)
	{
	case A_LOBBY_BROWSER:
		if (lobbyBrowser->action == LobbyBrowser::A_IN_LOBBY)
		{
			SetAction(A_WAITING_ROOM);
		}
		break;
	case A_CHOOSE_MAP:
		if (mapBrowserScreen->browserHandler->chooser->selectedRect != NULL)
		{
			selectedMap = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;
			action = A_CHOOSE_MAP_OPTIONS;
			mapOptionsPopup->Activate();
		}
		break;
	case A_CHOOSE_MAP_OPTIONS:
		if (mapOptionsPopup->action == MapOptionsPopup::A_CONFIRMED)
		{

			SetAction(A_CREATING_LOBBY);
			LobbyParams lp;
			lp.mapPath = selectedMap->filePath.string();
			lp.maxMembers = 2;
			netplayManager->TryCreateCustomLobby(lp);
			//cout << "waiting room" << endl;
		}
		else if (mapOptionsPopup->action == MapOptionsPopup::A_CANCELLED)
		{
			SetAction(A_CHOOSE_MAP);
			selectedMap = NULL;
			mapBrowserScreen->browserHandler->ClearSelection();
		}
		break;
	case A_CREATING_LOBBY:
	{
		if (netplayManager->lobbyManager->IsInLobby())
		{
			SetAction(A_WAITING_ROOM);
		}
		break;
	}
	case A_WAITING_ROOM:
	{
		

		if (netplayManager->IsHost())
		{
			if (waitingRoom->action == WaitingRoom::A_READY_TO_START)
			{
				SetAction(A_READY);

				LobbyMessage lm;
				lm.header.messageType = LobbyMessage::MESSAGE_TYPE_START_CUSTOM_MATCH;
				netplayManager->BroadcastLobbyMessage(lm);

				//return false;
			}
		}
		else
		{
			if (netplayManager->action == NetplayManager::A_GET_CONNECTIONS)
			{
				waitingRoom->SetAction(WaitingRoom::A_READY_TO_START);
				SetAction(A_READY);
				//return false;
			}
		}
		
		if( waitingRoom->action == WaitingRoom::A_LEAVE_ROOM)
		{
			BrowseCustomLobbies();
			//SetAction(A_IDLE);
			//return false;
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
	case A_CHOOSE_MAP_OPTIONS:
		mapBrowserScreen->Draw(target);
		mapOptionsPopup->Draw(target);
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