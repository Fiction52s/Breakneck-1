#ifndef __FREEPLAY_SCREEN_H__
#define __FREEPLAY_SCREEN_H__


#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "MatchParams.h"

struct MainMenu;
struct MapBrowser;
struct MapBrowserHandler;
struct WorkshopMapPopup;
struct FreeplayScreen;
struct MapNode;
struct MapOptionsPopup;
struct PlayerSkinShader;

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
	sf::Text skinNumberText;
	int skinIndex;

	sf::Vertex controllerIconQuad[4];
	sf::Vertex portIconQuad[4];

	sf::Vertex kinQuad[4];

	PlayerSkinShader *playerShader;

	ControllerDualStateQueue *controllerStates;

	sf::Sprite kinSprite;

	int action;

	FreeplayPlayerBox(FreeplayScreen *p_fps, int index);
	~FreeplayPlayerBox();
	void Update();
	void SetSkin(int index);
	void Draw(sf::RenderTarget *target);
	void SetName(const std::string &name);
	void SetControllerStates(ControllerDualStateQueue *conStates, int p_skinIndex );
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

	Tileset *ts_controllerIcons;
	Tileset *ts_portIcons;
	Tileset *ts_kin;

	Panel *panel;

	int playerBoxWidth;
	int playerBoxHeight;
	int playerBoxSpacing;

	MapNode *selectedMap;

	FreeplayPlayerBox *playerBoxes[4];
	MapOptionsPopup *mapOptionsPopup;

	MatchParams currParams;
	//int gccHeldStartFrames[4];
	//int windowsHeldStartFrames[4];
	//int joinHoldFrames;

	MapBrowserScreen *mapBrowserScreen;

	FreeplayScreen(MainMenu *mm);
	~FreeplayScreen();

	void Start();
	void Quit();
	bool HandleEvent(sf::Event ev);

	int GetFirstAvailableSkinIndex();
	bool IsSkinAvailable(int p_skinIndex);
	void NextSkin(int playerBoxIndex);
	void PrevSkin(int playerBoxIndex);

	void TryControllerJoin(ControllerDualStateQueue *conStates);
	bool IsFull();
	bool AlreadyJoined(ControllerDualStateQueue *conStates);
	FreeplayPlayerBox *GetNextOpenBox();
	void StartBrowsing();
	void TryActivateOptionsPanel(MapNode *mp);

	void DrawPopupBG(sf::RenderTarget *target);

	int NumActivePlayers();

	const MatchParams &GetMatchParams();
	void SetFromMatchParams(MatchParams &mp);

	void Update();
	void Draw(sf::RenderTarget *target);

	void CancelCallback(Panel *p);
	void SetAction(int a);
};

#endif