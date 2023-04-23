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
#include "steam/steam_api.h"

struct AdventureManager;
struct MapHeader;
struct ClosedBetaScreen;

struct Config;

struct SaveFile;
struct MapBrowserScreen;
struct WorkshopBrowser;
struct WorkshopManager;
struct MainMenu;
struct PauseMenu;
struct LevelSelector;
struct MusicInfo;
struct AdventureMap;
struct GlobalSaveFile;

struct EffectPool;

struct MatchResultsScreen;
struct FreeplayScreen;
struct SinglePlayerControllerJoinScreen;

struct ControlProfile;
struct ControlProfileMenu;
struct ControlProfileManager;
struct UIControlGrid;
struct FillRing;

struct SingleAxisSelector;
struct UIVerticalControlList;

//you should retain the last 
struct MusicSelector;
struct RecordGhostMenu;

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

struct MessagePopup;

struct GameSettingsScreen;

struct RemoteStorageManager;

#define ColorGL( c ) sf::Glsl::Vec4( c )// c.r, c.g, c.b, c.a )
//#define _WIN32_WINNT 0x0601

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
struct CustomMatchManager;
struct OnlineMenuScreen;
struct EditorMenuScreen;

struct CustomCursor;
struct MatchParams;

struct MainMenu
{
	//int [ControllerTypes::Count]

	enum MainMenuOptions
	{
		M_ADVENTURE,
		M_FREE_PLAY,
		M_ONLINE,
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
		TITLEMENU_INFOPOP,
		WORLDMAP,
		LOAD_ADVENTURE_MAP,
		SETUP_PRACTICE_ADVENTURE_MAP,
		SETUP_PRACTICE_ADVENTURE_MAP_BOOST,
		LOADINGMENUSTART,
		LOADINGMENULOOP,
		LOADINGMENUEND,
		KINBOOSTLOADINGMAP,
		RUN_ADVENTURE_MAP,
		RUN_EDITOR_MAP,
		SAVEMENU,
		TUTORIAL,
		ADVENTURETUTORIAL,
		TRANS_MAIN_TO_SAVE,
		TRANS_SAVE_TO_MAIN,
		TRANS_SAVE_TO_WORLDMAP,
		WORLDMAP_COLONY,
		TRANS_MAIN_TO_GAME_SETTINGS,
		GAME_SETTINGS,
		TRANS_MAIN_TO_CREDITS,
		CREDITS,
		TRANS_CREDITS_TO_MAIN,
		TRANS_WORLDMAP_TO_LOADING,
		DOWNLOAD_WORKSHOP_MAP,
		RUN_FREEPLAY_MAP,
		FREEPLAY,
		BROWSE_WORKSHOP,
		INTROMOVIE,
		THANKS_FOR_PLAYING,
		DOWNLOAD_WORKSHOP_MAP_START,
		DOWNLOAD_WORKSHOP_MAP_LOOP,
		QUICKPLAY_TEST,
		QUICKPLAY_PLAY,
		CUSTOM_MATCH_SETUP_FROM_PRACTICE,
		CUSTOM_MATCH_SETUP,
		CUSTOM_MATCH_SETUP_FROM_WORKSHOP_BROWSER,
		ONLINE_MENU,
		EDITOR_MENU,
		NETPLAY_MATCH_RESULTS,
		MATCH_RESULTS,
		POST_MATCH_OPTIONS,
		SINGLE_PLAYER_CONTROLLER_JOIN_TUTORIAL,
		SINGLE_PLAYER_CONTROLLER_JOIN_ADVENTURE,
		SINGLE_PLAYER_CONTROLLER_JOIN_ONLINE,
		CLOSED_BETA,
		QUICKPLAY_PRE_MATCH,
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
		GRT_TUTORIAL,
		GRT_FREEPLAY,
		GRT_QUICKPLAY,
	};


	RemoteStorageManager *remoteStorageManager;
	//testing
	CustomCursor *customCursor;
	NetplayManager *netplayManager;
	CustomMatchManager *customMatchManager;
	OnlineMenuScreen *onlineMenuScreen;
	EditorMenuScreen *editorMenuScreen;
	MatchResultsScreen *matchResultsScreen;
	FreeplayScreen *freeplayScreen;
	ClosedBetaScreen *closedBetaScreen;
	SinglePlayerControllerJoinScreen *singlePlayerControllerJoinScreen;

	MatchParams *menuMatchParams;

	std::string editMapName;
	Mode preFreeplayMode;
	Mode preEditMode;

	Mode modeLoadingFrom;
	Mode modeToLoad;
	void LoadMode(Mode m);
	void RunEditor(Mode preMode, const std::string &editMapName);
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
	

	//void RunFreePlayMap(const MatchParams *params);
	void DownloadAndRunWorkshopMap();
	void SetToMatchResults(GameSession *p_game);

	void UnlockSkin(int skinIndex);
	bool IsSkinUnlocked(int skinIndex);
	GlobalSaveFile *globalFile;


	std::vector<void(MainMenu::*)()> updateModeFuncs;

	sf::Vector2i GetPixelPos();

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

	ControlProfileManager *cpm;
	SoundInfo *soundInfos[SoundType::S_Count];
	GameSettingsScreen *gameSettingsScreen;
	SingleAxisSelector *saSelector;
	MapBrowserScreen *mapBrowserScreen;
	WorkshopBrowser *workshopBrowser;
	WorkshopManager *workshopManager;

	sf::Vector2i mousePixelPos;
	
	void UpdateMenuOptionText();
	void DrawMenuOptionText(sf::RenderTarget *target);
	
	MusicManager *musicManager;
	FillRing *testRing;

	AdventureManager *adventureManager;

	
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
	void CustomMapsOption();
	void GGPOOption();
	void SetModeAdventureLoadingMap( int wIndex );
	void SetModeKinBoostLoadingMap(int variation);

	sf::IntRect GetButtonIconTile(ControllerSettings::ButtonType button, ControlProfile *profile);
	sf::IntRect GetButtonIconTileForMenu(int controllerType, XBoxButton button);
	Tileset * GetButtonIconTileset(int controllerType);

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
	SoundManager soundManager;
	SoundNodeList * soundNodeList;
	TilesetManager tilesetManager;
	
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
	static sf::RenderTexture *mapPreviewThumbnailTexture;
	static sf::RenderTexture *brushPreviewTexture;
	static sf::RenderTexture *extraScreenTexture;
	static sf::RenderTexture *auraCheckTexture;
	
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
	boost::thread *loadThread;
	boost::thread *deadThread;

	EditSession *currEditSession;
	GameSession *currTutorialSession;
	GameSession *currFreePlaySession;
	GameSession *currLevel;
	GameSession *deadLevel;
	void HandleMenuMode();

	static void sGoToNextLevel(MainMenu *m,
		AdventureMap *am,
		Level *lev);
	void GoToNextLevel(AdventureMap *am,
		Level *lev);

	static void copyDirectoryRecursively(
		const boost::filesystem::path& sourceDir,
		const boost::filesystem::path& destinationDir);

	//singleton
	static MainMenu *GetInstance();
	static MainMenu *currInstance;
	
	static sf::Font arial;
	static sf::Font consolas;
	sf::Clock gameClock;

	LevelSelector *levelSelector; 
	
	MessagePopup *messagePopup;

	int kinTitleSpriteFrame;
	int kinTotalFrames;
	int currentMenuSelect;

	int windowWidth;
	int windowHeight;

	int style;

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

	STEAM_CALLBACK(MainMenu, OnGameLobbyJoinRequestedCallback, GameLobbyJoinRequested_t);


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
