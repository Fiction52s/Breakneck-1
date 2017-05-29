#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__

#include "SFML/Graphics.hpp"
#include "Input.h"
#include "SoundManager.h"
#include "Tileset.h"
#include "WorldMap.h"
#include "GUI.h"
#include "UIWindow.h"
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


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

struct ControlProfile;
struct ControlProfileMenu;
struct MultiLoadingScreen;
struct ControlProfileManager;
struct UIControlGrid;
struct MultiSelectionSection : UIEventHandlerBase
{
	
	enum Team
	{
		T_NOT_CHOSEN,
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

	MultiSelectionSection(MultiLoadingScreen *parent,
		int p_playerIndex );
	bool ButtonEvent( UIEvent eType,
		ButtonEventParams *param );
	Team team;
	int skinIndex;
	void Update();
	ControlProfile *profile;
	ControlProfileMenu *profileSelect;
	void Draw( sf::RenderTarget *target );
	sf::Sprite playerSprite;
	bool active;
	int playerIndex;
	MultiLoadingScreen *parent;
	bool isReady;
};

struct MultiLoadingScreen
{
	MultiLoadingScreen( MainMenu *p_mainMenu );
	void Reset(boost::filesystem::path path );
	void Update();
	void Draw( sf::RenderTarget *target );
	MultiSelectionSection *playerSection[4];
	boost::filesystem::path filePath;
	sf::Sprite previewSprite;
	sf::Texture previewTex;
	
	void SetPreview();
	bool AllPlayersReady();
	int GetNumActivePlayers();
	MainMenu *mainMenu;
	ControlProfileManager *cpm;
	boost::thread *loadThread;
	GameSession *gs;
};

#define ColorGL( c ) sf::Glsl::Vec4( c.r, c.g, c.b, c.a )
//#define _WIN32_WINNT 0x0601

struct OptionsMenuScreen
{
	OptionsMenuScreen(MainMenu *p_mainMenu);
	UIWindow *optionsWindow;
	void Update();
	void Draw(sf::RenderTarget *target);
	MainMenu *mainMenu;
};

struct MapHeaderInfo
{
	std::string description;
	std::string collectionName;
	int verA;
	int verB;
};

struct MapSelectionItem;
struct MapHeader;
struct MapCollection
{
	enum Tags
	{
		SINGLE = 1 << 0,
		DOUBLE = 1 << 1,
	};

	int tags;

	MapCollection();
	//bool Load(boost::filesystem::path path);

	std::string collectionName;
	std::list<MapSelectionItem*> maps;
	bool expanded;
};

struct MapSelectionItem
{
	MapSelectionItem(boost::filesystem::path p_path,
		MapHeader *mh)
		:path(p_path), headerInfo(mh)
	{
	}

	~MapSelectionItem();

	boost::filesystem::path path;
	MapCollection *collection;
	MapHeader *headerInfo;
};

struct MapHeader
{
	int ver1;
	int ver2;
	std::string collectionName;
	std::string description;
};

struct MapIndexInfo
{
	MapCollection*coll;
	MapSelectionItem *item;
};

struct SingleAxisSelector;
struct MapSelectionMenu
{
	//TODO scrollbar to show how far in to the names you are
	static const int NUM_BOXES = 24;
	static const int BOX_WIDTH;
	static const int BOX_HEIGHT;
	static const int BOX_SPACING;

	MapSelectionMenu( MainMenu *p_mainMenu,
		sf::Vector2f &p_pos );
	MainMenu *mainMenu;
	void SetupBoxes();
	void LoadItems();
	void Update(ControllerState &currInput,
		ControllerState &prevInput);
	void MoveUp();
	void MoveDown();
	void UpdateItemText();
	void UpdateBoxesDebug();
	void Draw(sf::RenderTarget *target);
	static MapHeader * ReadMapHeader(std::ifstream &is);

	sf::Vertex boxes[NUM_BOXES * 4];
	sf::Text itemName[NUM_BOXES];

	sf::Vector2f topMid;

	int oldCurrIndex;
	int topIndex;

	std::list<boost::filesystem::path> items;
	std::list<MapCollection*> collections;
	void LoadPath( boost::filesystem::path & p);
	sf::Font &font;
	SingleAxisSelector *saSelector;
	std::list<MapSelectionItem*>::iterator currItemIt;


	int numTotalItems;
	std::pair<std::string,MapIndexInfo> *allItems;
	int GetPairIndex(int index);

	sf::Sprite previewSprite;
	sf::Texture *previewTex;

	sf::Text descriptionText;
	
	//sf::Sprite previewSprite;
	//sf::Texture previewTex;

	//void SetPreview();
};

struct SaveMenuScreen
{
	SaveMenuScreen(MainMenu *p_mainMenu);
	Tileset *ts_background;//ts_saveMenuBG;
	Tileset *ts_selectSlot;//ts_saveMenuSelect;
	Tileset *ts_kinFace;//ts_saveMenuKinFace;
	void Update();
	void Draw(sf::RenderTarget *target);
	void Reset();
	MainMenu *mainMenu;
	sf::Vector2f menuOffset;
	sf::Sprite background;
	sf::Sprite selectSlot;
	sf::Sprite kinFace;
	int selectedSaveIndex;

	Tileset *ts_kinJump1;
	Tileset *ts_kinJump2;
	Tileset *ts_kinClouds;
	Tileset *ts_kinWindow;
	Tileset *ts_kinSky;

	sf::Sprite kinClouds;
	sf::Sprite kinJump;
	sf::Sprite kinWindow;
	sf::Sprite kinSky;

	Tileset *ts_starBackground;
	sf::Sprite starBackground;

	Tileset *ts_world;
	sf::Sprite world;
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
	void UpdateClouds();
	int cloudFrame;
	int cloudLoopLength;
	int cloudLoopFactor;

	int kinFaceFrame;
	int kinFaceTurnLength;

	int saveJumpFactor;
	int saveJumpLength;

	SaveFile *files[6];

	int moveDelayCounter;
};

struct CreditsMenuScreen
{
	sf::Sprite testSprite;
	Tileset *ts_test;
	CreditsMenuScreen(MainMenu *p_mainMenu);
	void Draw(sf::RenderTarget *target);
	void Update();
	MainMenu *mainMenu;
	sf::Vector2f menuOffset;
};



struct Parallax;
struct SingleAxisSelector;
struct MainMenu
{
	//int [ControllerTypes::Count]

	enum MainMenuOptions
	{
		M_ADVENTURE,
		M_FREE_PLAY,
		M_LOCAL_MULTIPLAYER,
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
		MULTIPREVIEW,
		TRANS_MAIN_TO_MULTIPREVIEW,
		TRANS_MULTIPREVIEW_TO_MAIN,
		DEBUG_RACEFIGHT_RESULTS,
		TRANS_MAIN_TO_MAPSELECT,
		MAPSELECT,
		TRANS_MAPSELECT_TO_MAIN,
		TRANS_MAIN_TO_OPTIONS,
		OPTIONS,
		TRANS_OPTIONS_TO_MAIN,
		TRANS_MAIN_TO_CREDITS,
		CREDITS,
		TRANS_CREDITS_TO_MAIN
	};

	enum SoundType
	{
		S_DOWN,
		S_UP,
		S_SELECT,
		S_Count
	};
	sf::SoundBuffer *soundBuffers[SoundType::S_Count];
	MapSelectionMenu *mapSelectionMenu;
	OptionsMenuScreen *optionsMenu;
	CreditsMenuScreen *creditsMenu;
	SaveMenuScreen *saveMenu;
	SingleAxisSelector *saSelector;
	sf::Text menuOptions[MainMenuOptions::M_Count];
	void UpdateMenuOptionText();
	

	MainMenu();
	~MainMenu();
	void Init();

	void SetMode(Mode m);

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
	MultiLoadingScreen *multiLoadingScreen;
	

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
	
	Tileset *ts_kinTitle[7];
	Tileset *ts_breakneckTitle;
	Tileset *ts_backgroundTitle;

	sf::Sprite splashSprite;
	Tileset *ts_splashScreen;

	void CustomMapOption();
	
	Mode menuMode;

	

	//ControllerState currInput;
	//ControllerState prevInput;
	ControllerState prevInput[4];
	ControllerState currInput[4];
	ControllerState &GetPrevInput( int index );
	ControllerState &GetCurrInput( int index );

	ControllerState menuPrevInput;
	ControllerState menuCurrInput;

	
	sf::Sprite backgroundTitleSprite;
	sf::Sprite breakneckTitleSprite;
	sf::Sprite kinTitleSprite;
	
	static sf::Font arial;
	sf::Clock gameClock;

	LevelSelector *levelSelector;
	WorldMap *worldMap;


	int kinTitleSpriteFrame;
	int kinTotalFrames;
	int currentMenuSelect;

	int windowWidth;
	int windowHeight;

	int style;

	static int masterVolume;

;
	//Tileset *ts_asteroid0;
	//Tileset *ts_asteroid1;
	//sf::Sprite asteroid0;
	//sf::Sprite asteroid1;
	//sf::Sprite asteroid2;
	Parallax *parBack;
	Parallax *parFront;
	
	void Slide();
	

	int transWorldMapFrame;
	sf::Uint8 transAlpha;


	

	Config *config;

	int splashFadeFrame;
	int splashFadeOutLength;
	sf::RectangleShape fadeRect;

	int numSlideFrames;
	int slideCurrFrame;
	sf::Vector2f slideStart;
	sf::Vector2f slideEnd;

	sf::Vector2f leftCenter;
	sf::Vector2f rightCenter;
	sf::Vector2f trueCenter;
	
	sf::Vector2f topCenter;
	sf::Vector2f bottomCenter;
	//int saveKinFaceFactor;
};

#endif
