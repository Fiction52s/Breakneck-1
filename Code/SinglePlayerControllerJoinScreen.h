#ifndef __SINGLE_PLAYER_CONTROLLER_JOIN_SCREEN_H__
#define __SINGLE_PLAYER_CONTROLLER_JOIN_SCREEN_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "MatchParams.h"
#include "GUI.h"

struct MainMenu;
struct MapBrowser;
struct MapBrowserHandler;
struct WorkshopMapPopup;
struct FreeplayScreen;
struct MapNode;
struct MapOptionsPopup;
struct PlayerSkinShader;

struct SinglePlayerBox
{
	enum Action
	{
		A_WAITING_FOR_JOIN,
		A_HAS_PLAYER,
	};

	enum Mode
	{
		MODE_DEFAULT,
		MODE_CONTROLLER_ONLY,
	};

	sf::Text playerNameText;
	sf::Vector2i topLeft;
	sf::Vertex bgQuad[4];
	SinglePlayerControllerJoinScreen *joinScreen;
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
	int mode;

	SinglePlayerBox(SinglePlayerControllerJoinScreen *p_joinScreen);
	~SinglePlayerBox();

	void SetMode(int m);
	void Update();
	void SetSkin(int index);
	void Draw(sf::RenderTarget *target);
	void SetName(const std::string &name);
	void SetControllerStates(ControllerDualStateQueue *conStates, int p_skinIndex);
	void Show();
	void Hide();
	void SetTopLeft(sf::Vector2i &pos);
	void ClearInfo();
private:
	bool show;
	std::string playerNameStr;
};

struct SinglePlayerControllerJoinScreen : TilesetManager, GUIHandler
{
	enum Action
	{
		A_WAITING_FOR_PLAYER,
		A_READY,
		A_START,
		A_BACK,
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

	SinglePlayerBox *playerBox;
	MapOptionsPopup *mapOptionsPopup;

	MatchParams currParams;

	SinglePlayerControllerJoinScreen(MainMenu *mm);
	~SinglePlayerControllerJoinScreen();

	void Start();
	void Quit();
	bool HandleEvent(sf::Event ev);

	void SetMode(int m);

	void NextSkin();
	void PrevSkin();

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