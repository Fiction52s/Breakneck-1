#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__

#include "SFML/Graphics.hpp"
#include "Input.h"
#include "SoundManager.h"
#include "Tileset.h"
#include "WorldMap.h"
#include "GUI.h"

struct SaveFile;

struct MainMenu;
struct CustomMapsHandler : GUIHandler
{
	//LevelSelector &ls;
	MainMenu *menu;
	bool optionChosen;
	bool showNamePopup;

	CustomMapsHandler( MainMenu *menu );
	void ButtonCallback( Button *b, const std::string & e );
	void TextBoxCallback( TextBox *tb, const std::string & e );
	void GridSelectorCallback( GridSelector *gs, const std::string & e );
	void CheckBoxCallback( CheckBox *cb, const std::string & e );
};

struct MainMenu
{
	enum MainMenuOptions
	{
		M_NEW_GAME,
		M_CONTINUE,
		M_CUSTOM_MAPS,
		M_LEVEL_EDITOR,
		M_OPTIONS,
		M_CREDITS,
		M_EXIT,
		M_Count
	};

	enum Mode
	{
		MAINMENU,
		WORLDMAP,
		SAVEMENU,
		TRANS_MAIN_TO_SAVE,
		TRANS_SAVE_TO_MAIN,
		TRANS_SAVE_TO_WORLDMAP
	};

	enum SoundType
	{
		S_DOWN,
		S_UP,
		S_SELECT,
		S_Count
	};
	sf::SoundBuffer *soundBuffers[SoundType::S_Count];

	MainMenu();
	~MainMenu();
	void Init();
	//void Deactivate();
	void Run();
	void GameEditLoop( const std::string &fileName );
	void GameEditLoop2( const std::string &filename);
	void CustomMapsOption();
	sf::RenderWindow *window;
	sf::View v;
	sf::View uiView;
	GameController controller;
	SoundManager soundManager;
	SoundNodeList * soundNodeList;
	TilesetManager tilesetManager;

	sf::Text betaText;

	static sf::RenderTexture *preScreenTexture;
	static sf::RenderTexture *postProcessTexture;
	static sf::RenderTexture *postProcessTexture1;
	static sf::RenderTexture *postProcessTexture2;
	static sf::RenderTexture *minimapTexture;
	static sf::RenderTexture *mapTexture;
	static sf::RenderTexture *pauseTexture;
	
	sf::Texture worldMapTex;
	sf::Sprite worldMapSpr;
	Tileset *ts_worldMap;
	Tileset *ts_saveMenuBG;
	Tileset *ts_saveMenuSelect;
	Tileset *ts_saveMenuKinFace;
	Tileset *ts_kinTitle[7];
	Tileset *ts_breakneckTitle;
	Tileset *ts_backgroundTitle;

	void CustomMapOption();
	
	Mode menuMode;

	SaveFile *files[6];

	ControllerState currInput;
	ControllerState prevInput;

	sf::Sprite saveBG;
	sf::Sprite saveSelect;
	sf::Sprite saveKinFace;
	sf::Sprite backgroundTitleSprite;
	sf::Sprite breakneckTitleSprite;
	sf::Sprite kinTitleSprite;
	int saveKinFaceFrame;
	int saveKinFaceTurnLength;
	sf::Font arial;
	sf::Clock gameClock;

	LevelSelector *levelSelector;
	WorldMap *worldMap;

	int selectedSaveIndex;
	bool selectCreateNew;

	int kinTitleSpriteFrame;
	int kinTotalFrames;
	int currentMenuSelect;

	int windowWidth;
	int windowHeight;
};

#endif