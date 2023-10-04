#ifndef __SINGLE_PLAYER_CONTROLLER_JOIN_SCREEN_H__
#define __SINGLE_PLAYER_CONTROLLER_JOIN_SCREEN_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "MatchParams.h"
#include "GUI.h"
#include "PlayerBox.h"

struct MainMenu;
struct MapBrowser;
struct MapBrowserHandler;
struct WorkshopMapPopup;
struct FreeplayScreen;
struct MapNode;
struct MapOptionsPopup;
struct PlayerSkinShader;

struct ControlProfileMenu;
struct PlayerBoxGroup;

struct SinglePlayerControllerJoinScreen;
struct StartBox
{
	sf::Vertex startButtonQuad[4];
	sf::Text startText;
	sf::Vertex bgQuad[4];
	MainMenu *mm;
	Tileset *ts_buttons;
	float width;
	float height;

	StartBox();
	void Update();
	void SetTopLeft(sf::Vector2f pos);
	void SetCenter(sf::Vector2f pos);
	void SetControllerType(int cType);
	void Draw(sf::RenderTarget *target);
};

struct SinglePlayerControllerJoinScreen : TilesetManager, GUIHandler
{
	enum Action
	{
		A_WAITING_FOR_PLAYER,
		A_READY,
		A_CONTROL_PROFILE,
		A_START,
		A_BACK,
	};

	StartBox startBox;
	int action;
	int frame;
	MainMenu *mainMenu;
	sf::Vertex bgQuad[4];

	sf::Vertex startButtonQuad[4];
	sf::Text startText;

	Panel *panel;

	MapNode *selectedMap;

	PlayerBoxGroup *playerBoxGroup;
	MapOptionsPopup *mapOptionsPopup;

	Tileset *ts_bg;

	MatchParams currParams;

	SinglePlayerControllerJoinScreen(MainMenu *mm);
	~SinglePlayerControllerJoinScreen();

	void Start();
	void Quit();
	bool HandleEvent(sf::Event ev);

	void SetMode(int m);

	void TryControllerJoin(ControllerDualStateQueue *conStates);

	void DrawPopupBG(sf::RenderTarget *target);

	const MatchParams &GetMatchParams();
	void SetFromMatchParams(MatchParams &mp);

	void Update();
	void Draw(sf::RenderTarget *target);

	void CancelCallback(Panel *p);
	void SetAction(int a);
};

#endif