#ifndef __FREEPLAY_SCREEN_H__
#define __FREEPLAY_SCREEN_H__


#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct MainMenu;
struct MapBrowser;
struct MapBrowserHandler;
struct WorkshopMapPopup;
struct FreeplayScreen;
struct MapNode;
struct MapOptionsPopup;

struct FreeplayPlayerBox
{
	enum Action
	{
		A_WAITING_FOR_JOIN,
		A_HAS_PLAYER,
	};

	sf::Text playerNameText;
	int index;
	sf::Vector2i topLeft;
	sf::Vertex bgQuad[4];
	FreeplayScreen *fps;
	sf::Text numberText;
	sf::Text pressText;

	ControllerDualStateQueue *controllerStates;

	int action;

	FreeplayPlayerBox(FreeplayScreen *p_fps, int index);
	void Draw(sf::RenderTarget *target);
	void SetName(const std::string &name);
	void SetControllerStates(ControllerDualStateQueue *conStates);
	void Show();
	void Hide();
	void SetTopLeft(sf::Vector2i &pos);
	void ClearInfo();
private:
	bool show;
	std::string playerNameStr;
};

struct FreeplayScreen : TilesetManager, GUIHandler
{
	enum Action
	{
		A_WAITING_FOR_PLAYERS,
		A_CHOOSE_MAP,
		A_START,
		A_BACK,
		A_DOWNLOADING_WORKSHOP_MAP,
		A_CHOOSE_MAP_OPTIONS,
	};

	int action;
	int frame;
	MainMenu *mainMenu;
	sf::Vertex bgQuad[4];

	Panel *panel;

	int playerBoxWidth;
	int playerBoxHeight;
	int playerBoxSpacing;

	MapNode *selectedMap;

	FreeplayPlayerBox *playerBoxes[4];
	MapOptionsPopup *mapOptionsPopup;
	//int gccHeldStartFrames[4];
	//int windowsHeldStartFrames[4];
	//int joinHoldFrames;

	MapBrowserScreen *mapBrowserScreen;

	FreeplayScreen(MainMenu *mm);
	~FreeplayScreen();

	void Start();
	void Quit();
	bool HandleEvent(sf::Event ev);

	void TryControllerJoin(ControllerDualStateQueue *conStates);
	bool IsFull();
	bool AlreadyJoined(ControllerDualStateQueue *conStates);
	FreeplayPlayerBox *GetNextOpenBox();
	void StartBrowsing();
	void TryActivateOptionsPanel(MapNode *mp);

	void DrawPopupBG(sf::RenderTarget *target);

	void Update();
	void Draw(sf::RenderTarget *target);

	void CancelCallback(Panel *p);
	void SetAction(int a);
};

#endif