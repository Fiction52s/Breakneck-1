#include "CustomMatchManager.h"
#include "LobbyBrowser.h"
#include "WaitingRoom.h"
#include <assert.h>

using namespace sf;
using namespace std;

CustomMatchManager::CustomMatchManager()
{
	lobbyBrowser = new LobbyBrowser;
	waitingRoom = new WaitingRoom;

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
		break;
	case A_CHOOSE_MAP_SETTINGS:
		break;
	case A_WAITING_ROOM:
		break;
	case A_READY:
		break;
	}
}

void CustomMatchManager::CreateCustomLobby()
{
	assert(action == A_LOBBY_BROWSER);

	SetAction(A_CHOOSE_MAP);
}

void CustomMatchManager::BrowseCustomLobbies()
{
	SetAction(A_LOBBY_BROWSER);
	lobbyBrowser->OpenPopup();
}

bool CustomMatchManager::Update()
{
	switch (action)
	{
	case A_LOBBY_BROWSER:
		lobbyBrowser->Update();

		if (lobbyBrowser->action == LobbyBrowser::A_IN_LOBBY)
		{
			SetAction(A_WAITING_ROOM);
		}
		break;
	case A_CHOOSE_MAP:
		break;
	case A_CHOOSE_MAP_SETTINGS:
		break;
	case A_WAITING_ROOM:
		break;
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
		break;
	case A_CHOOSE_MAP_SETTINGS:
		break;
	case A_WAITING_ROOM:
		break;
	case A_READY:
		break;
	}
}