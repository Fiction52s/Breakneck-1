#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__

#include "SFML/Graphics.hpp"
#include "Input.h"
#include "SoundManager.h"
#include "Tileset.h"
#include "WorldMap.h"
#include "GUI.h"

struct Config;

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

struct MultiSelectionInfo
{
	
	enum Team
	{
		T_RED,
		T_BLUE,
		T_Count
	};

	enum Skin
	{
		S_STANDARD,
		S_ALT0,
		S_Count
	};

	Team team;
	Skin skin;
	//[ControllerSettings::Count];
};


struct Parallax;
struct MainMenu
{
	//int [ControllerTypes::Count]

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
		SPLASH,
		SPLASH_TRANS,
		MAINMENU,
		WORLDMAP,
		SAVEMENU,
		TRANS_MAIN_TO_SAVE,
		TRANS_SAVE_TO_MAIN,
		TRANS_SAVE_TO_WORLDMAP,
		TRANS_MAIN_TO_MULTIPREVIEW,
		TRANS_MULTIPREVIEW_TO_MAIN,
		DEBUG_RACEFIGHT_RESULTS,
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
	void ResizeWindow( int windowWidth, 
		int windowHeight, int style );
	//void Deactivate();
	void Run();
	void GameEditLoop( const std::string &fileName );
	void GameEditLoop2( const std::string &filename);
	void CustomMapsOption();
	sf::RenderWindow *window;
	sf::View v;
	sf::View uiView;
	//GameController controller;
	//GameController controller2;
	GameController *controllers[4];
	GameController &GetController( int index );
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
	static sf::RenderTexture *saveTexture;
	static sf::RenderTexture *mapPreviewTexture;
	
	sf::Texture worldMapTex;
	sf::Sprite worldMapSpr;
	Tileset *ts_worldMap;
	Tileset *ts_saveMenuBG;
	Tileset *ts_saveMenuSelect;
	Tileset *ts_saveMenuKinFace;
	Tileset *ts_kinTitle[7];
	Tileset *ts_breakneckTitle;
	Tileset *ts_backgroundTitle;

	sf::Sprite splashSprite;
	Tileset *ts_splashScreen;

	MultiSelectionInfo multiInfo[4];

	void CustomMapOption();
	
	Mode menuMode;

	SaveFile *files[6];

	//ControllerState currInput;
	//ControllerState prevInput;
	ControllerState prevInput[4];
	ControllerState currInput[4];
	ControllerState &GetPrevInput( int index );
	ControllerState &GetCurrInput( int index );

	ControllerState menuPrevInput;
	ControllerState menuCurrInput;

	sf::Sprite saveBG;
	sf::Sprite saveSelect;
	sf::Sprite saveKinFace;
	sf::Sprite backgroundTitleSprite;
	sf::Sprite breakneckTitleSprite;
	sf::Sprite kinTitleSprite;
	
	static sf::Font arial;
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

	int style;

	static int masterVolume;

	Tileset *ts_saveKinJump1;
	Tileset *ts_saveKinJump2;
	Tileset *ts_saveKinClouds;
	Tileset *ts_saveKinWindow;
	Tileset *ts_saveKinSky;

	sf::Sprite saveKinClouds;
	sf::Sprite saveKinJump;
	sf::Sprite saveKinWindow;
	sf::Sprite saveKinSky;

	Tileset *ts_saveStarBackground;
	sf::Sprite saveStarBackground;

	Tileset *ts_saveWorld;
	sf::Sprite saveWorld;

	//Tileset *ts_asteroid0;
	//Tileset *ts_asteroid1;
	//sf::Sprite asteroid0;
	//sf::Sprite asteroid1;
	//sf::Sprite asteroid2;
	Parallax *parBack;
	Parallax *parFront;
	int asteroidScrollFrames0;
	int asteroidScrollFrames1;
	int asteroidScrollFrames2;
	int asteroidFrameBack;
	int asteroidFrameFront;

	sf::Sprite asteroid0;
	sf::Sprite asteroid1;
	sf::Sprite asteroid2;
	sf::Vector2f a0start;
	sf::Vector2f a0end;
	sf::Vector2f a1start;
	sf::Vector2f a1end;
	sf::Vector2f a2start;
	sf::Vector2f a2end;

	int transWorldMapFrame;
	sf::Uint8 transAlpha;


	void UpdateClouds();
	int cloudFrame;
	int cloudLoopLength;
	int cloudLoopFactor;

	int saveKinFaceFrame;
	int saveKinFaceTurnLength;

	int saveJumpFactor;
	int saveJumpLength;

	Config *config;

	int splashFadeFrame;
	int splashFadeOutLength;
	sf::RectangleShape fadeRect;
	//int saveKinFaceFactor;
};

#endif