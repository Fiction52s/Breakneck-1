#ifndef __FREEPLAY_SCREEN_H__
#define __FREEPLAY_SCREEN_H__


#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct MainMenu;
struct MapBrowser;
struct MapBrowserHandler;
struct WorkshopMapPopup;
struct FreeplayScreen;

struct FreeplayPlayerBox
{
	sf::Text *playerName;
	int index;
	sf::Vector2i topLeft;
	sf::Vertex bgQuad[4];
	FreeplayScreen *fps;
	sf::Text *numberText;

	int controllerType;
	int controllerIndex; //might be 2nd xbox controller but 4th player etc.
	int joinOrder;

	FreeplayPlayerBox(FreeplayScreen *p_fps, int index);
	void Draw(sf::RenderTarget *target);
	void SetName(const std::string &name);
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
		A_READY,
		A_BACK,
	};

	int action;
	int frame;
	MainMenu *mainMenu;
	sf::Vertex bgQuad[4];

	Panel *panel;

	int playerBoxWidth;
	int playerBoxHeight;
	int playerBoxSpacing;

	FreeplayPlayerBox *playerBoxes[4];
	int gccHeldStartFrames[4];
	int windowsHeldStartFrames[4];
	int joinHoldFrames;

	std::vector<GameController*> joinedControllers;

	FreeplayScreen(MainMenu *mm);
	~FreeplayScreen();

	void Start();
	void Quit();
	bool HandleEvent(sf::Event ev);

	void TryControllerJoin(GameController *con);

	void Update();
	void Draw(sf::RenderTarget *target);

	void CancelCallback(Panel *p);
	void SetAction(int a);
};

#endif