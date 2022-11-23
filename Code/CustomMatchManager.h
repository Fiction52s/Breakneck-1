#ifndef __CUSTOM_MATCH_MANAGER_H__
#define __CUSTOM_MATCH_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "steam/steam_api.h"

struct LobbyBrowser;
struct WaitingRoom;
struct MapBrowserScreen;
struct MapOptionsPopup;
struct MapNode;
struct MessagePopup;
struct PostMatchOptionsPopup;
struct PostMatchClientPopup;
struct LobbyInvitePopup;

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
		A_READY,
		A_ERROR_MESSAGE,
		A_POST_MATCH_HOST,
		A_POST_MATCH_CLIENT,
		A_POST_MATCH_CLIENT_LEAVE,
		A_POST_MATCH_HOST_REMATCH,
		A_POST_MATCH_HOST_CHOOSE_MAP,
		A_POST_MATCH_HOST_LEAVE
	};

	Action action;
	int frame;

	int currMapIndex;
	bool nextMapMode;

	LobbyBrowser *lobbyBrowser;
	WaitingRoom *waitingRoom;
	MapBrowserScreen * mapBrowserScreen;
	MapOptionsPopup *mapOptionsPopup;
	MessagePopup *messagePopup;
	PostMatchOptionsPopup *postMatchPopup;
	PostMatchClientPopup *postMatchClientPopup;
	LobbyInvitePopup *lobbyInvitePopup;
	int preErrorAction;
	bool fromWorkshopBrowser;

	MapNode *selectedMap;

	CustomMatchManager();
	~CustomMatchManager();
	void OpenPostMatchPopup();
	void CreateCustomLobby();
	void BrowseForNextMap();
	void CreateCustomLobbyFromWorkshopBrowser();
	void BrowseCustomLobbies();
	void TryActivateOptionsPanel( MapNode *mp );
	void SetAction(Action a);
	void HandleEvent(sf::Event ev);
	bool Update();
	void StartClientWaitingRoomForNextMap();
	void Draw(sf::RenderTarget *target);
	void DrawPopupBG(sf::RenderTarget *target);

};

#endif