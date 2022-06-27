#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__

#include "SFML/Graphics.hpp"
#include "Input.h"
#include "SoundManager.h"
#include "Tileset.h"
#include "GUI.h"
#include "UIWindow.h"
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <list>
#include <string>
#include <map>
#include "ItemSelector.h"
#include "Movement.h"
#include <boost/filesystem.hpp>
#include "GCC/USBDriver.h"
#include "GCC/VJoyGCController.h"

struct MapHeader;

struct WorldMap;
struct Config;

struct SaveFile;
struct SaveMenuScreen;
struct MapBrowserScreen;
struct MainMenu;
struct PauseMenu;
struct LevelSelector;
struct MusicInfo;
struct AdventureMap;
struct GlobalSaveFile;
struct ControlSettingsMenu;

struct EffectPool;

struct CustomMapsHandler : GUIHandler
{
	//LevelSelector &ls;
	MainMenu *menu;
	bool showNamePopup;
	bool showDownloadPopup;
	bool showLoginPopup;
	bool showRemovePopup;

	CustomMapsHandler( MainMenu *menu );
	void ButtonCallback( Button *b, const std::string & e );
	void TextBoxCallback( TextBox *tb, const std::string & e );
	void GridSelectorCallback( GridSelector *gs, const std::string & e );
	void CheckBoxCallback( CheckBox *cb, const std::string & e );
	void SliderCallback(Slider *slider);
	void DropdownCallback(Dropdown *dropdown, const std::string & e);
};

struct ControlProfile;
struct ControlProfileMenu;
struct MapSelectionMenu;
struct ControlProfileManager;
struct UIControlGrid;
struct FillRing;

struct KinBoostScreen;

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

	MultiSelectionSection( MainMenu *p_mainMenu,
		MapSelectionMenu *parent,
		int p_playerIndex,
		sf::Vector2f &topMid );
	~MultiSelectionSection();
	bool ButtonEvent( UIEvent eType,
		ButtonEventParams *param );
	Team team;
	int skinIndex;
	void Update();
	ControlProfile *profile;
	ControlProfileMenu *profileSelect;
	void Draw( sf::RenderTarget *target );

	void SetTopMid( sf::Vector2f &tm );
	sf::Sprite playerSprite;
	bool active;
	int playerIndex;
	MapSelectionMenu *parent;
	MainMenu *mainMenu;
	//bool isReady;
	bool IsReady();
	bool ShouldGoBack();
	bool holdingB;

	int bHoldFrames;
	int bHoldThresh;

	sf::Vector2f topMid;
	FillRing *backLoader;
	sf::Vector2f backLoaderOffset;

	sf::RectangleShape offRect;
	sf::Sprite bgSprite;
	//Tileset *ts_bg;
};

struct LoadingMapProgressDisplay;
struct MultiLoadingScreen
{
	MultiLoadingScreen( MainMenu *p_mainMenu );
	~MultiLoadingScreen();
	void Reset(boost::filesystem::path path );
	void Update();
	void Draw( sf::RenderTarget *target );
	MultiSelectionSection *playerSection[4];
	boost::filesystem::path filePath;
	sf::Sprite previewSprite;
	bool AllPlayersReady();
	int GetNumActivePlayers();
	MainMenu *mainMenu;
	
	boost::thread *loadThread;
	GameSession *gs;
	sf::Vector2f menuOffset;
	LoadingMapProgressDisplay *progressDisplay;

	
};

#define ColorGL( c ) sf::Glsl::Vec4( c )// c.r, c.g, c.b, c.a )
//#define _WIN32_WINNT 0x0601

struct OptionsMenuScreen : UIEventHandlerBase
{
	OptionsMenuScreen(MainMenu *p_mainMenu);
	~OptionsMenuScreen();
	UIWindow *optionsWindow;
	void Update( 
	ControllerState &curr,
		ControllerState &prev);
	void Draw(sf::RenderTarget *target);
	MainMenu *mainMenu;
	void Load();

	bool ButtonEvent(UIEvent eType,
		ButtonEventParams *param);
	bool CheckboxEvent(UIEvent eType,
		CheckboxEventParams *param);
	bool SelectorEvent(UIEvent eType,
		SelectorEventParams *param);

	void Center( sf::Vector2f & windowSize);
	UIHorizSelector<sf::Vector2i> *horizResolution;
	UIHorizSelector<int> *horizWindowModes;
	UIHorizSelector<int> *musicVolume;
	UIHorizSelector<int> *soundVolume;
	UIButton *defaultButton;
	UIButton *applyButton;
	UIButton *checkForControllerButton;
	UIHorizSelector<int> *horizDefaultController;
};



struct MapSelectionItem;
struct MapCollection
{
	enum Tags
	{
		SINGLE = 1 << 0,
		DOUBLE = 1 << 1,
	};

	int tags;

	MapCollection();
	~MapCollection();
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
	Tileset *ts_preview;
};

struct LoadingMapProgressDisplay
{

	enum ProgressString
	{

	};

	~LoadingMapProgressDisplay();

	const static int NUM_LOAD_THREADS;
	LoadingMapProgressDisplay(MainMenu *mainMenu,
		sf::Vector2f &topLeft );
	void SetProgressString(const std::string &str,
		int threadIndex = 0);
	void UpdateText();
	void Draw(sf::RenderTarget *target);
	void Reset();

	sf::Text *text;
	MainMenu *mainMenu;
	boost::mutex stringLock;
	std::string *currString;
	//std::string *
	int currStringThreadIndex;
};



struct MapIndexInfo
{
	MapCollection*coll;
	MapSelectionItem *item;
};

struct SingleAxisSelector;
struct UIVerticalControlList;

//you should retain the last 
struct MusicSelector;
struct RecordGhostMenu;

struct MapSelectionMenu
{
	enum State
	{
		S_MAP_SELECTOR,
		S_MAP_OPTIONS,
		S_MUSIC_SELECTOR,
		S_MUSIC_OPTIONS,
		S_GHOST_SELECTOR,
		S_GHOST_OPTIONS,
		S_SELECTING_SKIN,
		S_TO_MULTI_TRANS,
		S_FROM_MULTI_TRANS,
		S_LOADING,
		S_MULTI_SCREEN
	};

	struct OptionWindow
	{
		sf::RectangleShape optionRect;
		sf::Vector2f startPos; //top left
		sf::Vector2f endPos;
		CubicBezier slideInBez;
		CubicBezier slideOutBez;
	};

	State state;
	//TODO scrollbar to show how far in to the names you are
	static const int NUM_BOXES = 24;
	static const int BOX_WIDTH;
	static const int BOX_HEIGHT;
	static const int BOX_SPACING;

	struct TInfo
	{
		GameSession *gsession;
		boost::thread *loadThread;
	};

	MapSelectionMenu( MainMenu *p_mainMenu,
		sf::Vector2f &p_pos );
	~MapSelectionMenu();
	MainMenu *mainMenu;
	void SetupBoxes();
	void LoadItems();

	bool IsMultiMusicOn();
	bool IsMultiGhostOn();
	bool IsMusicSelectorVisible();
	bool IsGhostSelectorVisible();
	void Update(ControllerState &currInput,
		ControllerState &prevInput);
	void MoveUp();
	void MoveDown();
	void UpdateItemText();
	void UpdateBoxesDebug();
	void Draw(sf::RenderTarget *target);
	static bool WriteMapHeader(std::ofstream &of, MapHeader *mh);
	static bool ReplaceHeader(boost::filesystem::path &p,
		MapHeader *mh);
	void CleanupStopThreads();
	bool AllPlayersReady();
	int NumPlayersReady();
	static void sStopLoadThread(MapSelectionMenu *mapMenu,
		TInfo &ti );
	void StopLoadThread( TInfo &ti );

	LoadingMapProgressDisplay *progressDisplay;
	UIVerticalControlList *filterOptions;

	sf::Vertex boxes[NUM_BOXES * 4];
	sf::Text itemName[NUM_BOXES];

	void LoadMap();
	boost::thread *loadThread;
	
	std::list<boost::thread*> stopThreads;

	sf::Vector2f topMid;
	GameSession *gs;

	int oldCurrIndex;
	int topIndex;
	State oldState;

	std::list<boost::filesystem::path> items;
	std::list<MapCollection*> collections;
	void LoadPath( boost::filesystem::path & p);
	sf::Font &font;
	SingleAxisSelector *saSelector;

	MusicSelector *musicSelector;
	void UpdateMultiInput();

	Tileset *ts_buttonIcons;

	int numTotalItems;
	std::pair<std::string,MapIndexInfo> *allItems;
	int GetPairIndex(int index);

	sf::Sprite previewSprite;
	bool previewBlank;
	sf::RectangleShape blankTest;

	sf::Text descriptionText;
	
	sf::Sprite bg;
	Tileset *ts_bg;

	//sf::Sprite previewSprite;
	//sf::Texture previewTex;

	//void SetPreview();
	RecordGhostMenu *ghostSelector;

	MultiSelectionSection *singleSection;

	int multiTransFrame;
	int toMultiTransLength;
	int fromMultiTransLength;

	

	ControllerState multiMusicPrev;
	ControllerState multiMusicCurr;
	ControllerState multiGhostPrev;
	ControllerState multiGhostCurr;

	sf::Vector2f menuOffset;

	MultiSelectionSection *multiPlayerSection[4];

	enum MultiSelectorState
	{
		MS_NEUTRAL,
		MS_MUSIC,
		MS_GHOST,
		MS_MUSIC_OPTIONS,
		MS_GHOST_OPTIONS
	};

	MultiSelectorState multiSelectorState;
	Tileset *ts_multiProfileRow;
	sf::Sprite multiProfileRow;

	Tileset *ts_multiSelect;
	sf::Sprite multiSelect;

	sf::Vector2f multiRowOnPos;
	sf::Vector2f multiRowOffPos;

	sf::Vector2f multiSelectOnPos;
	sf::Vector2f multiSelectOffPos;
	// more transition information later
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
struct MusicManager;
struct FillRing;
struct Level;
struct TitleScreen;
struct IntroMovie;
struct MusicPlayer;
struct Fader;
struct Swiper;
struct LoadingBackpack;

struct LevelLoadParams
{
	LevelLoadParams()
		:bestTimeGhostOn(false),
		bestReplayOn(false),
		loadingScreenOn( false ),
		world(0),
		level(NULL),
		adventureMap(NULL)
	{

	}
	bool bestTimeGhostOn;
	bool bestReplayOn;
	bool loadingScreenOn;
	int world;
	Level *level;
	AdventureMap *adventureMap;
};



std::string GetTimeStr(int numFrames);

struct NetplayManager;

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
		M_TUTORIAL,
		M_CREDITS,
		M_EXIT,
		M_Count
	};

	enum Mode
	{
		SPLASH,
		SPLASH_TRANS,
		TITLEMENU,
		WORLDMAP,
		LOADINGMAP,
		LOADINGMENUSTART,
		LOADINGMENULOOP,
		LOADINGMENUEND,
		KINBOOSTLOADINGMAP,
		RUNNINGMAP,
		RUNNINGEDITOR,
		SAVEMENU,
		TUTORIAL,
		ADVENTURETUTORIAL,
		TRANS_MAIN_TO_SAVE,
		TRANS_SAVE_TO_MAIN,
		TRANS_SAVE_TO_WORLDMAP,
		WORLDMAP_COLONY,
		MULTIPREVIEW,
		TRANS_MAPSELECT_TO_MULTIPREVIEW,
		TRANS_MULTIPREVIEW_TO_MAPSELECT,
		DEBUG_RACEFIGHT_RESULTS,
		TRANS_MAIN_TO_MAPSELECT,
		MAPSELECT,
		TRANS_MAPSELECT_TO_MAIN,
		TRANS_MAIN_TO_OPTIONS,
		OPTIONS,
		TRANS_OPTIONS_TO_MAIN,
		TRANS_MAIN_TO_CREDITS,
		CREDITS,
		TRANS_CREDITS_TO_MAIN,
		TRANS_WORLDMAP_TO_LOADING,
		DOWNLOAD_WORKSHOP_MAP,
		RUN_WORKSHOP_MAP,
		BROWSE_WORKSHOP,
		INTROMOVIE,
		THANKS_FOR_PLAYING,
		DOWNLOADSTART,
		DOWNLOADLOOP,
		QUICKPLAY_TEST,
		EXITING,
	};

	enum SoundType
	{
		S_DOWN,
		S_UP,
		S_SELECT,
		S_Count
	};

	enum GameRunType
	{
		GRT_ADVENTURE,
		GRT_FREEPLAY
	};

	//testing

	NetplayManager *netplayManager;

	std::string currWorkshopMap;

	Mode modeLoadingFrom;
	Mode modeToLoad;
	void LoadMode(Mode m);
	void StartLoadModeScreen();
	static void sLevelLoad(MainMenu *mm, GameSession *gs);
	void LevelLoad(GameSession *gs);
	static void sTransitionMode(MainMenu *mm, Mode fromMode, Mode toMode);
	void TransitionMode(Mode fromMode, Mode toMode);
	void TitleMenuModeUpdate();
	void DrawMode(Mode m);

	void LoadAndResaveMap(const std::string &path);
	void CreateRenderTextures();
	void SetupWindow();
	void UpdateMenuMode();
	void CheckForControllers();
	void UpdateMenuInput();

	void RunWorkshopMap(const std::string &path);
	void DownloadAndRunWorkshopMap();

	void UnlockSkin(int skinIndex);
	bool IsSkinUnlocked(int skinIndex);
	GlobalSaveFile *globalFile;


	std::vector<void(MainMenu::*)()> updateModeFuncs;

	GCC::USBDriver *gccDriver;
	GCC::VJoyGCControllers *joys;
	bool gccDriverEnabled;

	PS5ControllerManager ps5ControllerManager;

	sf::Vector2i GetPixelPos();

	bool IsKeyPressed(int key);
	bool IsMousePressed(int m);

	EffectPool *indEffectPool;
	void DrawEffects( sf::RenderTarget *target );
	void UpdateEffects();
	void ActivateIndEffect(
			Tileset *ts,
			sf::Vector2<double> pos,
			bool pauseImmune,
			double angle,
			int frameCount,
			int animationFactor,
			bool right,
			int startFrame = 0 );
	Fader *fader;
	Swiper *swiper;
	GameRunType gameRunType;
	SaveFile *GetCurrentProgress();
	ControlProfileManager *cpm;
	SoundInfo *soundInfos[SoundType::S_Count];
	MapSelectionMenu *mapSelectionMenu;
	OptionsMenuScreen *optionsMenu;
	CreditsMenuScreen *creditsMenu;
	SaveMenuScreen *saveMenu;
	SingleAxisSelector *saSelector;
	MapBrowserScreen *mapBrowserScreen;

	sf::Vector2i mousePixelPos;
	

	SaveFile *currSaveFile;
	void UpdateMenuOptionText();
	void DrawMenuOptionText(sf::RenderTarget *target);
	
	MusicManager *musicManager;
	FillRing *testRing;

	
	MainMenu();
	~MainMenu();
	void Init();
	MusicPlayer *musicPlayer;
	double accumulator;
	double currentTime;
	int modeFrame;

	void SetMode(Mode m);
	void ReturnToWorldAfterLevel();
	int transFrame;
	int transLength;
	void ResizeWindow( int windowWidth, 
		int windowHeight, int style );
	//void Deactivate();
	void Run();
	void GameEditLoop( const std::string &fileName );
	void GameEditLoop2( const std::string &filename);
	void CustomMapsOption();
	void GGPOOption();
	void SetModeLoadingMap( int wIndex );
	void SetModeKinBoostLoadingMap(int variation);

	sf::IntRect GetButtonIconTile(int controllerIndex, int baseButtonIndex);
	Tileset * GetButtonIconTileset(int controllerIndex);

	sf::RenderWindow *window;
	sf::View v;
	sf::View uiView;

	
	sf::Sprite loadingBGSpr;
	Tileset *ts_loadBG;
	LoadingBackpack *loadingBackpack;

	Tileset *ts_buttonIcons;
	Tileset *ts_keyboardIcons;
	Tileset *ts_thanksForPlaying;
	sf::Vertex thanksQuad[4];
	//GameController controller;
	//GameController controller2;
	GameController *controllers[4];
	GameController &GetController( int index );
	SoundManager soundManager;
	SoundNodeList * soundNodeList;
	TilesetManager tilesetManager;
	MultiLoadingScreen *multiLoadingScreen;
	
	bool quit;
	bool doneLoading;
	bool changedMode;

	sf::Text betaText; 

	static sf::RenderTexture *preScreenTexture;
	static sf::RenderTexture *postProcessTexture2;
	static sf::RenderTexture *minimapTexture;
	static sf::RenderTexture *mapTexture;
	static sf::RenderTexture *pauseTexture;
	static sf::RenderTexture *saveTexture;
	static sf::RenderTexture *mapPreviewTexture;
	static sf::RenderTexture *brushPreviewTexture;
	static sf::RenderTexture *extraScreenTexture;
	static sf::RenderTexture *auraCheckTexture;

	sf::Texture worldMapTex;
	sf::Sprite worldMapSpr;
	Tileset *ts_worldMap;

	ControlSettingsMenu *controlSettingsMenu;
	
	//Tileset *ts_kinTitle[7];
	TitleScreen *titleScreen;
	IntroMovie *introMovie;

	MusicInfo *menuMusic;

	sf::Sprite splashSprite;
	Tileset *ts_splashScreen;

	void CustomMapOption();
	void PlayIntroMovie();
	Mode menuMode;
	void AdventureLoadLevel(LevelLoadParams &loadParams);
	void AdventureNextLevel(Level *lev);
	boost::thread *loadThread;
	boost::thread *deadThread;

	EditSession *currEditSession;
	GameSession *currTutorialSession;
	GameSession *currWorkshopSession;
	GameSession *currLevel;
	GameSession *deadLevel;
	void HandleMenuMode();

	static void sGoToNextLevel(MainMenu *m,
		AdventureMap *am,
		Level *lev);



	//ControllerState currInput;
	//ControllerState prevInput;
	ControllerState prevInput[4];
	ControllerState prevInputUnfiltered[4];
	ControllerState currInput[4];
	ControllerState currInputUnfiltered[4];
	ControllerState &GetPrevInput( int index );
	ControllerState &GetCurrInput( int index );
	ControllerState &GetPrevInputUnfiltered(int index);
	ControllerState &GetCurrInputUnfiltered(int index);

	ControlProfile *GetCurrSelectedProfile();
	bool SetCurrProfileByName(const std::string &name);
	
	static MapHeader * ReadMapHeader(std::ifstream &is);

	//singleton
	static MainMenu *GetInstance();
	static MainMenu *currInstance;

	ControllerState menuPrevInput;
	ControllerState menuCurrInput;
	
	
	sf::Sprite titleBGSprite;
	sf::Sprite breakneckTitleSprite;
	sf::Sprite emergenceTitleSprite;
	sf::Sprite kinTitleSprite;
	
	static sf::Font arial;
	static sf::Font consolas;
	sf::Clock gameClock;

	LevelSelector *levelSelector; 
	WorldMap *worldMap;


	int kinTitleSpriteFrame;
	int kinTotalFrames;
	int currentMenuSelect;

	int windowWidth;
	int windowHeight;

	int style;

	bool mouseGrabbed;
	bool mouseVisible;
	void SetMouseGrabbed(bool grabbed);
	void SetMouseVisible(bool vis);
	bool GetMouseGrabbed();
	bool GetMouseVisible();

	static int masterVolume;

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

	void CopyMap(CustomMapsHandler *cmp,
		Panel *namePop);


	sf::Vertex mainMenuOptionQuads[M_Count * 4];
	sf::Vertex mainMenuOptionHighlight[M_Count *4];
	bool activatedMainMenuOptions[M_Count];
	
	Tileset *ts_mainOption;
	Tileset *ts_menuSelector;
	sf::RectangleShape menuOptionsBG;
	sf::Sprite selectorSprite;
	int selectorAnimFrame;
	int selectorAnimDuration;
	int selectorAnimFactor;
	float selectorSpriteXPos;
	float selectorSpriteYPosBase;
	float selectorSpriteYPosInterval;
	bool lastMovementDown;
	
	

	Config *config;

	int numSlideFrames;
	int slideCurrFrame;
	sf::Vector2f slideStart;
	sf::Vector2f slideEnd;

	sf::Vector2f doubleLeftCenter;
	sf::Vector2f leftCenter;
	sf::Vector2f rightCenter;
	sf::Vector2f trueCenter;
	
	sf::Vector2f topCenter;
	sf::Vector2f bottomCenter;
	//int saveKinFaceFactor;
};

#endif
