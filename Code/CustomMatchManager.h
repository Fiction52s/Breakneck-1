#ifndef __CUSTOM_MATCH_MANAGER_H__
#define __CUSTOM_MATCH_MANAGER_H__

#include <SFML/Graphics.hpp>


struct LobbyBrowser;
struct WaitingRoom;
struct MapBrowserScreen;
struct MapOptionsPopup;
struct MapNode;
struct MessagePopup;
struct PostMatchOptionsPopup;

struct CustomMatchManager
{
	enum Action
	{
		A_IDLE,
		A_LOBBY_BROWSER,
		A_CHOOSE_MAP,
		A_DOWNLOADING_WORKSHOP_MAP,
		A_CHOOSE_MAP_OPTIONS,
		A_CREATING_LOBBY,
		A_WAITING_ROOM,
		A_CONNECT_TO_HOST,
		A_WAIT_FOR_PREVIEW,
		A_READY,
		A_ERROR_MESSAGE,
	};

	Action action;
	int frame;

	LobbyBrowser *lobbyBrowser;
	WaitingRoom *waitingRoom;
	MapBrowserScreen * mapBrowserScreen;
	MapOptionsPopup *mapOptionsPopup;
	MessagePopup *messagePopup;
	PostMatchOptionsPopup *postMatchPopup;
	int preErrorAction;
	bool fromWorkshopBrowser;

	MapNode *selectedMap;

	CustomMatchManager();
	~CustomMatchManager();
	void CreateCustomLobby();
	void CreateCustomLobbyFromWorkshopBrowser();
	void BrowseCustomLobbies();
	void TryActivateOptionsPanel( MapNode *mp );
	void SetAction(Action a);
	void HandleEvent(sf::Event ev);
	bool Update();
	void Draw(sf::RenderTarget *target);

};

#endif