#ifndef __LOBBY_BROWSER_H__
#define __LOBBY_BROWSER_H__

#include <string>
#include <SFML/Graphics.hpp>
#include "GUI.h"


struct NetplayManager;

struct LobbyNode
{
	std::string lobbyName;
	std::string hostName;
};

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
	std::string playingSongName;
	CheckBox *playOriginalCheckbox;

	LobbyChooseRect *currPlayingRect;
	LobbyChooseRect *currPlayingMyRect;
	//Button *okButton;
	//Button *cancelButton;


	LobbySelector( int rows);
	~LobbySelector();
	void SetPlayingColor(const std::string &str);
	void SetStoppedColor();
	void OpenPopup();
	void ClosePopup();
	void Draw(sf::RenderTarget *target);
	void MouseScroll(int delta);
	void PopulateRects();

	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
};

struct LobbyBrowser
{
	LobbySelector *lobbySelector;

	enum Action
	{
		A_GET_LOBBIES,
		A_IDLE,
		A_AUTO_REFRESH_LOBBIES,
	};

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
};

#endif