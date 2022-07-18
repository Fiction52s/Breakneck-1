#ifndef __CUSTOM_MATCH_MANAGER_H__
#define __CUSTOM_MATCH_MANAGER_H__

#include <SFML/Graphics.hpp>

struct LobbyBrowser;
struct WaitingRoom;

struct CustomMatchManager
{
	enum Action
	{
		A_IDLE,
		A_LOBBY_BROWSER,
		A_CHOOSE_MAP,
		A_CHOOSE_MAP_SETTINGS,
		A_WAITING_ROOM,
		A_READY,
	};

	Action action;
	int frame = 0;

	LobbyBrowser *lobbyBrowser;
	WaitingRoom *waitingRoom;

	CustomMatchManager();
	~CustomMatchManager();
	void CreateCustomLobby();
	void BrowseCustomLobbies();
	void SetAction(Action a);
	void HandleEvent(sf::Event ev);
	bool Update();
	void Draw(sf::RenderTarget *target);

};

#endif