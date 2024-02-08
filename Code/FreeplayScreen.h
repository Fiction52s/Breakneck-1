#ifndef __FREEPLAY_SCREEN_H__
#define __FREEPLAY_SCREEN_H__


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
struct PlayerBoxGroup;

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

	Tileset *ts_bg;

	Panel *panel;

	MapNode *selectedMap;

	PlayerBoxGroup *playerBoxGroup;
	MapOptionsPopup *mapOptionsPopup;

	MatchParams currParams;
	//int gccHeldStartFrames[4];
	//int windowsHeldStartFrames[4];
	//int joinHoldFrames;

	MapBrowserScreen *mapBrowserScreen;

	FreeplayScreen();
	~FreeplayScreen();

	void Start();
	void Quit();
	bool HandleEvent(sf::Event ev);

	void StartBrowsing();
	void TryActivateOptionsPanel(MapNode *mp);

	void DrawPopupBG(sf::RenderTarget *target);


	const MatchParams &GetMatchParams();
	void SetFromMatchParams(MatchParams &mp);

	void Update();
	void Draw(sf::RenderTarget *target);

	void CancelCallback(Panel *p);
	void SetAction(int a);
};

#endif