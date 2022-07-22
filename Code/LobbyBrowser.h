#ifndef __LOBBY_BROWSER_H__
#define __LOBBY_BROWSER_H__

#include <string>
#include <SFML/Graphics.hpp>
#include "GUI.h"

struct LobbyChooseRect : ChooseRect
{
	LobbyChooseRect(ChooseRectIdentity ident,
		sf::Vertex *v, sf::Vector2f &position,
		const std::string &str, sf::Vector2f &boxSize,
		Panel *p);

	void UpdatePanelPos();
	void Draw(sf::RenderTarget *target);
	void SetSize(sf::Vector2f &bSize);
	void SetText(const std::string &str);
	void UpdateTextPosition();
	void SetTextHeight(int height);
	void SetName(const std::string &name);
};

struct LobbySelector : PanelUpdater, GUIHandler
{
	int topRow;
	int maxTopRow;
	int numEntries;
	int totalRects;
	LobbyChooseRect **lobbyRects;
	Panel *panel;
	LobbyChooseRect *currSelectedRect;

	LobbySelector( int rows);
	~LobbySelector();
	void Start();
	void ClosePopup();
	void Draw(sf::RenderTarget *target);
	void MouseScroll(int delta);
	void PopulateRects();

	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
};

struct LobbyBrowser : PanelUpdater, GUIHandler
{
	enum Action
	{
		A_GET_LOBBIES,
		A_IDLE,
		A_AUTO_REFRESH_LOBBIES,
		A_TRY_JOIN_LOBBY,
		A_IN_LOBBY,
		A_RETURN_TO_MENU,
	};

	int topRow;
	int maxTopRow;
	int numEntries;
	int totalRects;
	LobbyChooseRect **lobbyRects;
	Panel *panel;
	LobbyChooseRect *currSelectedRect;


	Action action;
	int frame;

	LobbyBrowser();
	~LobbyBrowser();
	void Update();
	void Draw(sf::RenderTarget *target);
	void OpenPopup();
	void ClosePopup();
	void SetAction(Action a);
	bool HandleEvent(sf::Event ev);

	void MouseScroll(int delta);
	void PopulateRects();
	void ClearLobbyRects();

	void ClearSelection();

	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
};

#endif