#ifndef __LOBBY_BROWSER_H__
#define __LOBBY_BROWSER_H__

#include <string>
#include <SFML/Graphics.hpp>
#include "GUI.h"


struct LobbyManager;

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

struct LobbyListChooser;
struct LobbyListChooserHandler : GUIHandler
{
	LobbyListChooserHandler(int rows);
	virtual ~LobbyListChooserHandler();
	virtual void ClickText(ChooseRect *cr) = 0;
	//virtual void Cancel() = 0;
	//virtual void Confirm() = 0;
	//virtual void ClickFile(ChooseRect *cr) = 0;
	//virtual void FocusFile(ChooseRect *cr) = 0;
	//virtual void UnfocusFile(ChooseRect *cr) = 0;
	virtual	bool MouseUpdate() { return true; }
	virtual void Draw(sf::RenderTarget *target) {}
	//virtual void ChangePath() {}
	virtual void LateDraw(sf::RenderTarget *target) {}

	////guihandler functions
	//virtual void ChooseRectEvent(ChooseRect *cr, int eventType);
	//virtual void ButtonCallback(Button *b, const std::string & e);
	//virtual void SliderCallback(Slider *slider) {}
	//---------
	//virtual void PanelCallback(Panel *p, const std::string & e);

	LobbyListChooser *chooser;
};

struct LobbyChooserHandler : LobbyListChooserHandler
{
	enum State
	{
		BROWSE,
	};

	State state;

	LobbyChooserHandler(int rows);
	~LobbyChooserHandler();
	bool MouseUpdate();
	void Draw(sf::RenderTarget *target);

	void ClickText(ChooseRect *cr);
	//virtual void ChangePath() {}
	void LateDraw(sf::RenderTarget *target);

	////guihandler functions
	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	//virtual void SliderCallback(Slider *slider) {}
	//---------
	//void DropdownCallback(Dropdown *dropdown, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
};

struct LobbyListChooser : PanelUpdater
{
	int topRow;
	int maxTopRow;
	int numEntries;
	int totalRects;
	LobbyChooseRect **lobbyRects;
	Panel *panel;
	LobbyListChooserHandler *handler;
	std::string playingSongName;
	CheckBox *playOriginalCheckbox;

	LobbyManager *lobbyManager;

	LobbyChooseRect *currPlayingRect;
	LobbyChooseRect *currPlayingMyRect;
	//Button *okButton;
	//Button *cancelButton;


	LobbyListChooser(LobbyListChooserHandler *handler, int rows);
	~LobbyListChooser();
	void SetPlayingColor(const std::string &str);
	void SetStoppedColor();
	void OpenPopup();
	void ClosePopup();
	bool MouseUpdate();
	void Draw(sf::RenderTarget *target);
	void Deactivate();
	void MouseScroll(int delta);
	void LateDraw(sf::RenderTarget *target);
	void PopulateRects();
};

struct LobbyBrowser
{
	LobbyChooserHandler *lobbyChooserHandler;

	enum Action
	{
		A_GET_LOBBIES,
		A_IDLE,
	};

	Action action;

	LobbyBrowser();
	~LobbyBrowser();
	void Update();
	void Draw(sf::RenderTarget *target);
	void OpenPopup();
	void ClosePopup();
};

#endif