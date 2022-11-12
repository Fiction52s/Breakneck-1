#include "MainMenu.h"
#include <boost/filesystem.hpp>
#include "EditSession.h"
#include "GameSession.h"
#include "SaveFile.h"
#include <iostream>
#include <sstream>
#include "Config.h"
#include "ControlProfile.h"
#include "UIWindow.h"
#include "sfeMovie/StreamSelection.hpp"
#include "MusicSelector.h"
#include "PlayerRecord.h"
#include "PowerOrbs.h"
#include "Enemy_Shard.h"
#include "SaveMenuScreen.h"
#include "PauseMenu.h"
#include "TitleScreen.h"
#include "IntroMovie.h"
#include "WorldMap.h"
#include "LevelSelector.h"
#include "KinBoostScreen.h"
#include "MusicPlayer.h"
#include "Fader.h"
#include "VisualEffects.h"
#include "MapHeader.h"
#include "ButtonHolder.h"
#include "ControlSettingsMenu.h"

#include <Windows.h>
#include "public.h"
#include "vjoyinterface.h"
#include "GCC/USBDriver.h"
#include "GCC/VJoyGCController.h"

#include "CustomMapClient.h"
#include "LoadingBackpack.h"

#include "MapBrowserScreen.h"
#include "steam/steam_api.h"

#include "MapBrowser.h"
#include "NetplayManager.h"

#include "CustomCursor.h"
#include "MenuPopup.h"

#include "CustomMatchManager.h"
#include "OnlineMenuScreen.h"
#include "MatchResultsScreen.h"
#include "WorkshopMapPopup.h"
#include "WorkshopBrowser.h"
#include "FreeplayScreen.h"
#include "WorkshopManager.h"

#include "PostMatchOptionsPopup.h"
#include "UIMouse.h"
#include "UIController.h"

using namespace std;
using namespace sf;
using namespace boost::filesystem;

#define TIMESTEP (1.0 / 60.0)


const int LoadingMapProgressDisplay::NUM_LOAD_THREADS = 5;

sf::RenderTexture *MainMenu::preScreenTexture = NULL;
sf::RenderTexture *MainMenu::postProcessTexture2 = NULL;
sf::RenderTexture *MainMenu::minimapTexture = NULL;
sf::RenderTexture *MainMenu::mapTexture = NULL;
sf::RenderTexture *MainMenu::pauseTexture = NULL;
sf::RenderTexture *MainMenu::saveTexture = NULL;
sf::RenderTexture *MainMenu::mapPreviewTexture = NULL;
sf::RenderTexture *MainMenu::brushPreviewTexture = NULL;
sf::RenderTexture *MainMenu::extraScreenTexture = NULL;
sf::RenderTexture *MainMenu::auraCheckTexture = NULL;

MainMenu * MainMenu::currInstance = NULL;

const int MapSelectionMenu::BOX_WIDTH = 580;
const int MapSelectionMenu::BOX_HEIGHT = 40;
const int MapSelectionMenu::BOX_SPACING = 0;

sf::Font MainMenu::arial;
sf::Font MainMenu::consolas;
int MainMenu::masterVolume = 100;

std::string GetTimeStr(int numFrames)
{
	stringstream ss;

	int seconds = numFrames / 60;
	int remain = numFrames % 60;
	int centiSecond = floor((double)remain * (1.0 / 60.0 * 100.0) + .5);

	if (seconds < 10)
	{
		ss << "0";
	}
	ss << seconds << " : ";

	if (centiSecond < 10)
	{
		ss << "0";
	}
	ss << centiSecond << endl;

	return ss.str();
}


void MainMenu::LevelLoad(GameSession *gs)
{
	/*delete worldMap;
	worldMap = NULL;
	delete saveMenu;
	saveMenu = NULL;*/

	GameSession::sLoad(gs);
}

void MainMenu::sLevelLoad(MainMenu *mm, GameSession *gs)
{
	mm->LevelLoad(gs);
}


void MainMenu::TransitionMode(Mode fromMode, Mode toMode)
{
	switch (fromMode)
	{
	case SAVEMENU:
		assert(worldMap != NULL);
		delete worldMap;
		worldMap = NULL;

		assert(saveMenu != NULL);
		currSaveFile = NULL;
		delete saveMenu;
		saveMenu = NULL;
		break;
	case TITLEMENU:
	{
		assert(titleScreen != NULL);
		delete titleScreen;
		titleScreen = NULL;
		break;
	}
	case RUN_EDITOR_MAP:
	{
		assert(currEditSession != NULL);
		delete currEditSession;
		currEditSession = NULL;
		break;
	}
	case TUTORIAL:
	case ADVENTURETUTORIAL:
	{
		assert(currTutorialSession != NULL);
		delete currTutorialSession;
		currTutorialSession = NULL;
		break;
	}
	case WORLDMAP_COLONY:
	{
		worldMap->CurrSelector()->DestroyBGs();
		worldMap->CurrSelector()->FocusedSector()->DestroyMapPreview();
		break;
	}
	case WORLDMAP:
	{
		SaveFile *saveFile = GetCurrentProgress();
		saveFile->mostRecentWorldSelected = worldMap->selectedColony;
		saveFile->Save();
		worldMap->CurrSelector()->CreateBGs();

		if (worldMap->CurrSelector()->numSectors == 1)
		{
			worldMap->CurrSelector()->FocusedSector()->UpdateMapPreview();
		
		}
		break;
	}
	case RUN_ADVENTURE_MAP:
	{
		if (worldMap != NULL)
		{
			delete worldMap;
			worldMap = NULL;
		}

		if (saveMenu != NULL)
		{
			currSaveFile = NULL;
			delete saveMenu;
			saveMenu = NULL;
		}
		break;
	}
	case RUN_FREEPLAY_MAP:
	{
		if (currFreePlaySession != NULL)
		{
			delete currFreePlaySession;
			currFreePlaySession = NULL;
		}
		break;
	}
	case FREEPLAY:
	{
		if (freeplayScreen != NULL)
		{
			delete freeplayScreen;
			freeplayScreen = NULL;
		}
		break;
	}

	}

	switch (toMode)
	{
	case WORLDMAP:
	{
		if (fromMode == ADVENTURETUTORIAL)
		{
			assert(worldMap == NULL);
			assert(saveMenu == NULL);
			worldMap = new WorldMap(this);
			saveMenu = new SaveMenuScreen(this);
			saveMenu->Reset();
			worldMap->state = WorldMap::PLANET;//WorldMap::PLANET_AND_SPACE;
			worldMap->frame = 0;
			worldMap->InitSelectors();
			worldMap->SetDefaultSelections();
			worldMap->UpdateWorldStats();
		}
		break;
	}
	case SAVEMENU:
		assert(worldMap == NULL);
		assert(saveMenu == NULL);
		worldMap = new WorldMap(this);
		saveMenu = new SaveMenuScreen(this);
		saveMenu->Reset();
		break;
	case TITLEMENU:
		assert(titleScreen == NULL);
		titleScreen = new TitleScreen(this);
		titleScreen->Reset();
		break;
	case RUN_EDITOR_MAP:
	{
		if (fromMode == BROWSE_WORKSHOP)
		{
			workshopBrowser->ClearAllPreviewsButSelected();
		}

		assert(currEditSession == NULL);
		currEditSession = new EditSession(this, editMapName);
		break;
	}
	
	case TUTORIAL:
	case ADVENTURETUTORIAL:
	{
		assert(currTutorialSession == NULL);
		MatchParams mp;
		mp.saveFile = currSaveFile;
		mp.mapPath = "Resources/Maps/Beta3/tut1.brknk";

		currTutorialSession = new GameSession(&mp);
		GameSession::sLoad(currTutorialSession);
		break;
	}
	case RUN_FREEPLAY_MAP:
	{
		assert(currFreePlaySession == NULL);

		MatchParams mp;
		mp.saveFile = NULL;//currSaveFile;
		mp.mapPath = freeplayMapName;

		currFreePlaySession = new GameSession(&mp);
		GameSession::sLoad(currFreePlaySession);
		break;
	}
	case FREEPLAY:
	{
		if (fromMode == BROWSE_WORKSHOP)
		{
			workshopBrowser->ClearAllPreviewsButSelected();
		}


		assert(freeplayScreen == NULL);

		freeplayScreen = new FreeplayScreen(this);
		freeplayScreen->Start();
	}
	}
}




void MainMenu::sTransitionMode(MainMenu *mm, Mode fromMode, Mode toMode )
{
	mm->TransitionMode(fromMode, toMode);
}


GameController &MainMenu::GetController( int index )
{
	return *controllers[index];
}

void MainMenu::UpdateMenuOptionText()
{
	int breatheFrames = 180;
	int breatheWaitFrames = 0;
	int bTotal = breatheFrames + breatheWaitFrames;
	float halfBreathe = breatheFrames / 2;
	int f = titleScreen->frame % (bTotal);
	float alpha;
	if (f <= halfBreathe)
	{
		alpha = f / halfBreathe;
	}
	else if (f <= breatheFrames)
	{
		f -= halfBreathe;
		alpha = 1.f - f / halfBreathe;
	}
	else
	{
		alpha = 0;
	}

	float baseAlpha = 180;
	SetRectColor(mainMenuOptionHighlight + saSelector->currIndex * 4, Color( 255, 255, 255, baseAlpha + alpha * (255.f - baseAlpha ) ));

	selectorSprite.setPosition(selectorSpriteXPos, selectorSpriteYPosBase + selectorSpriteYPosInterval * saSelector->currIndex);
}

void MainMenu::CheckForControllers()
{
	//currently only checks for gamecube, xbox doesnt need it.
	//cout << "start checking" << endl;

	ps5ControllerManager.CheckForControllers();

	ps5ControllerManager.InitControllers(this);
	

	gccDriver = new GCC::USBDriver;
	if (gccDriver->getStatus() == GCC::USBDriver::Status::READY)
	{
		//cout << "ready" << endl;
		gccDriverEnabled = true;
		joys = new GCC::VJoyGCControllers(*gccDriver);
		{
			auto controllers = gccDriver->getState();
			//for (int i = 0; i < 4; ++i)
			//{
			//	//GameController &c = GetController(i);

			//	//c.gcDefaultControl.x = controllers[i].enabled
			//}
		}
	}
	else
	{
		//cout << "failing" << endl;
		joys = NULL;
		gccDriverEnabled = false;
		delete gccDriver;
		gccDriver = NULL;
	}

	//cout << "end checking" << endl;
}

MainMenu::MainMenu()
	:windowWidth(1920), windowHeight(1080)
{
	assert(currInstance == NULL);
	currInstance = this;

	selectorAnimFrame = 0;
	selectorAnimDuration = 21;
	selectorAnimFactor = 3;
	
	

	globalFile = new GlobalSaveFile;
	if (!globalFile->Load())
	{
		globalFile->SetToDefaults();
		globalFile->Save();
	}

	currEditSession = NULL;
	currTutorialSession = NULL;
	currFreePlaySession = NULL;
	freeplayScreen = NULL;

	currSaveFile = NULL;

	arial.loadFromFile("Resources/Fonts/Breakneck_Font_01.ttf");
	consolas.loadFromFile("Resources/Fonts/Courier New.ttf");

	transLength = 60;
	transFrame = 0;

	for (int i = 0; i < 4; ++i)
	{
		controllers[i] = new GameController(i);
	}

	CheckForControllers();

	cpm = new ControlProfileManager;
	cpm->LoadProfiles();

	infoPopup = new MenuInfoPopup(this);

	//MusicManager mm(this);
	musicManager = new MusicManager(this);
	musicManager->LoadMusicNames();

	controlSettingsMenu = new ControlSettingsMenu(this, &tilesetManager);

	deadThread = NULL;
	loadThread = NULL;
	
	ts_mainOption = tilesetManager.GetSizedTileset("Menu/mainmenu_text_512x64.png");
	ts_menuSelector = tilesetManager.GetSizedTileset("Menu/menu_selector_64x64.png");

	ts_buttonIcons = tilesetManager.GetSizedTileset("Menu/button_icon_128x128.png");
	ts_keyboardIcons = tilesetManager.GetSizedTileset("Menu/keyboard_icons_64x64.png");
	ts_thanksForPlaying = NULL;

	selectorSprite.setTexture(*ts_menuSelector->texture);

	activatedMainMenuOptions[0] = true;		//adventure
	activatedMainMenuOptions[1] = true;		//freeplay
	activatedMainMenuOptions[2] = true;//false;	//local multiplayer
	activatedMainMenuOptions[3] = true;		//level editor
	activatedMainMenuOptions[4] = true;		//options
	activatedMainMenuOptions[5] = true;		//tutorial
	activatedMainMenuOptions[6] = true;	//credits
	activatedMainMenuOptions[7] = true;		//exit

	for (int i = 0; i < MainMenuOptions::M_Count; ++i)
	{
		if (activatedMainMenuOptions[i])
		{
			SetRectSubRect(mainMenuOptionQuads + i * 4, ts_mainOption->GetSubRect(i));
		}
		else
		{
			SetRectSubRect(mainMenuOptionQuads + i * 4, ts_mainOption->GetSubRect(i + M_Count*2));
		}

		if (activatedMainMenuOptions[i])
		{
			SetRectSubRect(mainMenuOptionHighlight + i * 4, ts_mainOption->GetSubRect(i + M_Count));
		}
	}

	introMovie = new IntroMovie(this);

	netplayManager = new NetplayManager;

	customMatchManager = new CustomMatchManager;

	workshopManager = new WorkshopManager; //needs to be made before any mapbrowsers

	if (musicManager->songMap.empty())
	{
		cout << "music manager is empty. no songs loaded" << endl;
		//assert(0);
		
	}

  	int wholeX = 1920;
	int wholeY = 1080;
	int halfX = wholeX / 2;
	int halfY = wholeY / 2;
	doubleLeftCenter = Vector2f(-halfX - wholeX, halfY);
	trueCenter = Vector2f(halfX, halfY);
	leftCenter = Vector2f(halfX - wholeX, halfY);
	rightCenter = Vector2f(halfX + wholeX, halfY);
	topCenter = Vector2f(halfX, halfY - wholeY);
	bottomCenter = Vector2f(halfX, halfY + wholeY);


	config = new Config();
	config->WaitForLoad();
	windowWidth = config->GetData().resolutionX;
	windowHeight = config->GetData().resolutionY;
	style = config->GetData().windowStyle;

	musicPlayer = new MusicPlayer(this);

	//clean this up for when someone doesn't have the music maybe? shouldn't crash at least.
	menuMusic = musicManager->GetMusicInfo("w0_2_Breakneck_Menu_01");
	if (menuMusic != NULL)
	{
		menuMusic->Load();
	}
	

	//cout << "DDDDDD " << endl;

	titleScreen = new TitleScreen(this);
	//cout << "start mm constfr" << endl;
	//load a preferences file at some point for window resolution and stuff

	//could be bad cuz its static
	

	uiView = View(sf::Vector2f(960, 540), sf::Vector2f(1920, 1080));
	v = View(Vector2f(1920 / 2, 1080 / 2), Vector2f(1920, 1080));

	sf::Text t;
	t.setFont(arial);
	t.setCharacterSize(40);
	t.setFillColor(Color::White);
	

	int waitFrames[] = { 60, 30, 20 };
	int waitModeThresh[] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, M_Count, 0);


	
	Vector2f textBase(100, 300);
	int textOptionSpacing = 6;
	int charHeight = 40;

	selectorSpriteXPos = textBase.x - 32;//+ 512 + 50;
	selectorSpriteYPosBase = textBase.y + 32;
	selectorSpriteYPosInterval = 64 + textOptionSpacing;

	for (int i = 0; i < MainMenuOptions::M_Count; ++i)
	{
		SetRectCenter(mainMenuOptionQuads + i * 4, 512, 64, textBase + Vector2f(512/2, 32 + i * (64 + textOptionSpacing) ) );
		SetRectCenter(mainMenuOptionHighlight + i * 4, 512, 64, textBase + Vector2f(512 / 2, 32 + i * (64 + textOptionSpacing)));
	}

	menuOptionsBG.setFillColor(Color( 0, 0, 0, 70 ));
	menuOptionsBG.setPosition(textBase);
	menuOptionsBG.setSize(Vector2f(600, (64 + textOptionSpacing) * M_Count));

	soundNodeList = new SoundNodeList( 10 );
	soundNodeList->SetSoundVolume(config->GetData().soundVolume);
	//soundNodeList->SetGlobalVolume(100);//config->GetData().soundVolume );


	CreateRenderTextures();

	transWorldMapFrame = 0;
	

	titleScreen->Draw(preScreenTexture);

	worldMap = NULL;
	saveMenu = NULL;
	//worldMap = new WorldMap( this );

	

	levelSelector = new LevelSelector( this );

	

	kinTitleSprite.setPosition( 512, 1080 );
	
	kinTitleSpriteFrame = 0;
	kinTotalFrames = 76 * 2 + 50;

	currentMenuSelect = 0;

	SetupWindow();

	betaText.setString( " Press any button to start \n For help and info check \n - beta_info.txt\n Breakneck Beta\n Updated 9/2/2016");
	betaText.setCharacterSize( 20 );
	betaText.setFillColor( Color::Red );
	betaText.setPosition( 50, 200 );
	betaText.setFont( arial );

	slideCurrFrame = 0;
	numSlideFrames = 60;

	Init();

	multiLoadingScreen = new MultiLoadingScreen( this );

	mapSelectionMenu = NULL;
	//mapSelectionMenu = new MapSelectionMenu(this, Vector2f(0, 100));

	optionsMenu = new OptionsMenuScreen(this);

	creditsMenu = new CreditsMenuScreen(this);

	mapBrowserScreen = new MapBrowserScreen(this);

	workshopBrowser = NULL;

	onlineMenuScreen = new OnlineMenuScreen(this);

	matchResultsScreen = NULL;

	loadingBackpack = new LoadingBackpack(&tilesetManager);
}

void MainMenu::SetupWindow()
{
	window = NULL;
	/*windowWidth = 960;
	windowHeight = 540;*/
	window = new RenderWindow(sf::VideoMode(windowWidth, windowHeight), "Breakneck",
		config->GetData().windowStyle, sf::ContextSettings(0, 0, 0, 0, 0));
	window->setKeyRepeatEnabled(false);

	MOUSE.SetRenderWindow(window);
	UICONTROLLER.SetRenderWindow(window);

	customCursor = new CustomCursor;
	//Tileset *ts_cursor = tilesetManager.GetSizedTileset("arrow_editor_36x36.png");
	customCursor->Init(window);

	mouseGrabbed = true;//false;//true;
	mouseVisible = true;//false;//true;//false;

	SetMouseGrabbed(mouseGrabbed);
	SetMouseVisible(mouseVisible);


	for (int i = 0; i < 4; ++i)
	{
		controllers[i]->window = window;
		controllers[i]->UpdateState();
	}

	assert(window != NULL);
	window->setVerticalSyncEnabled(true);
	//window->setFramerateLimit(120);
	std::cout << "opened window" << endl;

	window->setView(v);
}


void MainMenu::CreateRenderTextures()
{
	if (preScreenTexture == NULL)
	{
		preScreenTexture = new RenderTexture;
		preScreenTexture->create(1920, 1080);
		preScreenTexture->clear();
	}

	if (extraScreenTexture == NULL)
	{
		extraScreenTexture = new RenderTexture;
		extraScreenTexture->create(1920, 1080);
		extraScreenTexture->clear();
	}

	if (postProcessTexture2 == NULL)
	{
		postProcessTexture2 = new RenderTexture;
		postProcessTexture2->create(1920, 1080);
		postProcessTexture2->clear();
	}

	if (minimapTexture == NULL)
	{
		minimapTexture = new RenderTexture;
		//minimapTexture->setSmooth( true );
		minimapTexture->create(500, 500);
		minimapTexture->clear();
	}

	if (mapTexture == NULL)
	{
		mapTexture = new RenderTexture;
		mapTexture->create(1720, 880);
		mapTexture->clear();
	}

	if (pauseTexture == NULL)
	{
		pauseTexture = new RenderTexture;
		pauseTexture->create(1820, 980);
		pauseTexture->clear();
	}

	if (saveTexture == NULL)
	{
		saveTexture = new RenderTexture;
		saveTexture->create(1920, 1080);
		saveTexture->clear();
	}

	if (mapPreviewTexture == NULL)
	{
		mapPreviewTexture = new RenderTexture;
		mapPreviewTexture->create(960 - 48, 540 - 48);
		mapPreviewTexture->clear();
	}

	if (brushPreviewTexture == NULL)
	{
		brushPreviewTexture = new RenderTexture;
		brushPreviewTexture->create(512, 512);
		brushPreviewTexture->clear();
	}

	if (auraCheckTexture == NULL)
	{
		auraCheckTexture = new RenderTexture;
		auraCheckTexture->create(256, 256);
		auraCheckTexture->clear();
	}
}

SaveFile *MainMenu::GetCurrentProgress()
{
	return currSaveFile;
	/*if (saveMenu == NULL)
	{
		return NULL;
	}
	else
	{
		return saveMenu->files[saveMenu->selectedSaveIndex];
	}*/
}

MainMenu::~MainMenu()
{
	assert(currInstance == this);
	currInstance = NULL;

	window->close();

	delete customCursor;

	delete loadingBackpack;
	delete cpm;
	delete musicManager;

	delete infoPopup;

	delete controlSettingsMenu;

	if( introMovie != NULL)
		delete introMovie;

	if (netplayManager != NULL)
		delete netplayManager;

	if (customMatchManager != NULL)
	{
		delete customMatchManager;
	}

	if (workshopManager != NULL)
	{
		delete workshopManager;
	}

	for (int i = 0; i < 4; ++i)
	{
		delete controllers[i];
	}
	delete config;

	delete musicPlayer;
	delete titleScreen;
	delete saSelector;
	delete soundNodeList;

	delete preScreenTexture;
	delete extraScreenTexture;
	delete postProcessTexture2;
	delete minimapTexture;
	delete mapTexture;

	delete pauseTexture;
	delete saveTexture;
	delete mapPreviewTexture;
	delete auraCheckTexture;
	delete brushPreviewTexture;


	if (worldMap != NULL)
	{
		delete worldMap;
	}

	if (saveMenu != NULL)
	{
		currSaveFile = NULL;
		delete saveMenu;
	}

	
	delete levelSelector;
	delete window;

	delete multiLoadingScreen;

	if (mapSelectionMenu != NULL)
	{
		delete mapSelectionMenu;
	}
	
	delete optionsMenu;
	delete creditsMenu;
	delete mapBrowserScreen;
	delete onlineMenuScreen;

	if (workshopBrowser != NULL)
	{
		delete workshopBrowser;
	}
	

	delete fader;
	delete swiper;
	delete indEffectPool;

	if (joys != NULL)
		delete joys;

	if( gccDriver != NULL )
		delete gccDriver;

	delete globalFile;
}

//singleton
MainMenu *MainMenu::GetInstance()
{
	return currInstance;
}

void MainMenu::Init()
{	
	ts_buttonIcons = tilesetManager.GetSizedTileset("Menu/button_icon_128x128.png");

	ts_splashScreen = tilesetManager.GetTileset( "Menu/splashscreen_1920x1080.png", 1920, 1080 );
	splashSprite.setTexture( *ts_splashScreen->texture );

	/*ts_kinTitle[0] = tilesetManager.GetTileset( "Title/kin_title_1_1216x1080.png", 1216, 1080 );
	ts_kinTitle[1] = tilesetManager.GetTileset( "Title/kin_title_2_1216x1080.png", 1216, 1080 );
	ts_kinTitle[2] = tilesetManager.GetTileset( "Title/kin_title_3_1216x1080.png", 1216, 1080 );
	ts_kinTitle[3] = tilesetManager.GetTileset( "Title/kin_title_4_1216x1080.png", 1216, 1080 );
	ts_kinTitle[4] = tilesetManager.GetTileset( "Title/kin_title_5_1216x1080.png", 1216, 1080 );
	ts_kinTitle[5] = tilesetManager.GetTileset( "Title/kin_title_6_1216x1080.png", 1216, 1080 );
	ts_kinTitle[6] = tilesetManager.GetTileset( "Title/kin_title_7_1216x1080.png", 1216, 1080 );*/

	fader = new Fader;
	swiper = new Swiper();

	Swiper::LoadSwipeType( this, Swiper::W1);
	
	indEffectPool = new EffectPool(EffectType::FX_IND, 4);
	indEffectPool->Reset();


	soundInfos[S_DOWN] = soundManager.GetSound( "menu_down" );
	soundInfos[S_UP] = soundManager.GetSound( "menu_up" );
	soundInfos[S_SELECT] = soundManager.GetSound( "menu_select" );

	ts_loadBG = NULL;

	cout << "init finished" << endl;
}

bool MainMenu::IsKeyPressed(int k)
{
	Keyboard::Key key = (Keyboard::Key)k;

	if (window->hasFocus())
	{
		return Keyboard::isKeyPressed(key);
	}
	
	return false;
}

bool MainMenu::IsMousePressed(int m)
{
	Mouse::Button but = (Mouse::Button)m;

	if (window->hasFocus())
	{
		return Mouse::isButtonPressed(but);
	}
}

void MainMenu::DrawEffects( RenderTarget *target )
{
	indEffectPool->Draw(target);
}

void MainMenu::UpdateEffects()
{
	indEffectPool->Update();
}

void MainMenu::ActivateIndEffect(
	Tileset *ts,
	sf::Vector2<double> pos,
	bool pauseImmune,
	double angle,
	int frameCount,
	int animationFactor,
	bool right,
	int startFrame)
{
	EffectInstance params;
	Transform tr = sf::Transform::Identity;

	params.SetParams(Vector2f(pos), tr, frameCount, animationFactor, startFrame );
	indEffectPool->ActivateIndEffect(&params, ts);
}

void MainMenu::LoadAndResaveMap(const std::string &path)
{
	EditSession *es = new EditSession(this, path);
	es->LoadAndResave();
	delete es;
}

void MainMenu::GameEditLoop( const std::string &p_path )
{
	int result = 0;

	Vector2f lastViewSize( 0, 0 );
	Vector2f lastViewCenter( 0, 0 );
	while( result == 0 )
	{
		EditSession *es = new EditSession(this, p_path );
		es->SetInitialView(lastViewCenter, lastViewSize);
		result = es->Run();
		delete es;
		if( result > 0 )
			break;

		//v.setSize( 1920, 1080 );
		window->setView( v );

		MatchParams mp;
		mp.mapPath = p_path;

		GameSession *gs = new GameSession(&mp);
		GameSession::sLoad(gs);
		
		gameRunType = GRT_FREEPLAY;
		result = gs->Run();
	

		lastViewCenter = gs->lastViewCenter;
		lastViewSize = gs->lastViewSize;
		delete gs;
	}
}

void MainMenu::GameEditLoop2( const std::string &p_path )
{
	int result = 0;

	Vector2f lastViewSize( 0, 0 );
	Vector2f lastViewCenter( 0, 0 );
	while( result == 0 )
	{
		gameRunType = MainMenu::GRT_FREEPLAY;
		window->setView( v );

		MatchParams mp;
		mp.mapPath = p_path;

		GameSession *gs = new GameSession(&mp);
		GameSession::sLoad(gs);
		
		result = gs->Run();
		lastViewCenter = gs->lastViewCenter;
		lastViewSize = gs->lastViewSize;
		delete gs;
		if( result > 0 )
			break;

		EditSession *es = new EditSession(this, p_path );
		es->SetInitialView(lastViewCenter, lastViewSize);
		result = es->Run();
		delete es;
	}
}

void MainMenu::SetMouseGrabbed(bool grabbed)
{
	mouseGrabbed = grabbed;
	window->setMouseCursorGrabbed(mouseGrabbed);
}

void MainMenu::SetMouseVisible(bool vis)
{
	mouseVisible = vis;
	window->setMouseCursorVisible(mouseVisible);
}

bool MainMenu::GetMouseGrabbed()
{
	return mouseGrabbed;
}

bool MainMenu::GetMouseVisible()
{
	return mouseVisible;
}

void MainMenu::SetMode(Mode m)
{
	transFrame = 0;
	
	modeFrame = 0;

	/*if (menuMode == TITLEMENU && m != TITLEMENU)
	{
		musicPlayer->TransitionMusic(menuMusic, 60);
	}
	else if (menuMode != TITLEMENU && m == TITLEMENU)
	{
		musicPlayer->TransitionMusic(titleScreen->titleMusic, 60);
	}*/

	int oldMode = menuMode;

	menuMode = m;
	//only need this because the transition is seamless so inputs can
	//get buffered
	if (menuMode == TITLEMENU || menuMode == TRANS_MAIN_TO_SAVE || menuMode == TRANS_MAIN_TO_MAPSELECT)//|| menuMode == WORLDMAP)
	{
		//TerrainRender::CleanupLayers(); //saves a little time?
		changedMode = false;
	}
	else
	{
		changedMode = true;
	}

	if (menuMode == RUN_EDITOR_MAP)
		fader->Clear();

	if (menuMode == KINBOOSTLOADINGMAP)
	{
		worldMap->kinBoostScreen->Reset();
	}

	if (menuMode == TITLEMENU)
	{
		selectorAnimFrame = 0;
	}

	if (menuMode == ONLINE_MENU)
	{
		onlineMenuScreen->Start();
	}
	else if (menuMode == MATCH_RESULTS)
	{
		matchResultsScreen->Reset();
	}
	else if (menuMode == POST_MATCH_OPTIONS)
	{
		customMatchManager->postMatchPopup->Start();
	}
	else if (menuMode == SAVEMENU)
	{
		if (oldMode != WORLDMAP)
		{
			musicPlayer->PlayMusic(menuMusic);
		}
		saveMenu->SetSkin(GetCurrentProgress()->defaultSkinIndex);
	}
	else if (menuMode == BROWSE_WORKSHOP)
	{
		SetMouseGrabbed(true);
		SetMouseVisible(true);
	}

	if (menuMode == WORLDMAP_COLONY)
	{
		worldMap->state = WorldMap::COLONY;
		worldMap->frame = 0;
	}
	if (menuMode == WORLDMAP && worldMap->state == WorldMap::COLONY)
	{
		soundNodeList->ActivateSound(soundManager.GetSound("world_zoom_out"));
		worldMap->state = WorldMap::COLONY_TO_PLANET;
		worldMap->frame = 0;
		worldMap->UpdateWorldStats();
	}

	if (menuMode == THANKS_FOR_PLAYING)
	{
		ts_thanksForPlaying = tilesetManager.GetTileset("Story/Ship_04.png", 1920, 1080);
		ts_thanksForPlaying->SetQuadSubRect(thanksQuad, 0);
		SetRectTopLeft(thanksQuad, 1920, 1080, Vector2f(0, 0));
	}
	else if (oldMode == THANKS_FOR_PLAYING)
	{
		tilesetManager.DestroyTileset(ts_thanksForPlaying);
		ts_thanksForPlaying = NULL;
	}
}

void MainMenu::DrawMenuOptionText(sf::RenderTarget *target)
{
	target->draw(menuOptionsBG);
	for (int i = 0; i < MainMenuOptions::M_Count; ++i)
	{
		target->draw(mainMenuOptionQuads, M_Count * 4, sf::Quads, ts_mainOption->texture);
		//preScreenTexsetture->draw(menuOptions[i]);
	}

	if (activatedMainMenuOptions[saSelector->currIndex])
	{
		target->draw(mainMenuOptionHighlight + saSelector->currIndex * 4, 4, sf::Quads,
			ts_mainOption->texture);
	}

	target->draw(selectorSprite);
}

sf::Vector2i MainMenu::GetPixelPos()
{
	Vector2i pPos = Mouse::getPosition(*window);
	pPos.x *= 1920.f / window->getSize().x;
	pPos.y *= 1080.f / window->getSize().y;

	return pPos;
}

void MainMenu::CustomMapsOption()
{
	LevelSelector &ls = *levelSelector;
	sf::Event ev;

	sf::View oldPreView = preScreenTexture->getView();

	sf::View oldView = window->getView();
	window->setView( uiView );
	bool customMapQuit = false;

	CustomMapsHandler customMapHandler( this );

	Panel p( "custom maps", 1920 - ls.width, 1080, &customMapHandler );
	p.pos.x += ls.width;
	p.AddButton( "Play", Vector2i( 100, 0 ), Vector2f( 100, 50 ), "PLAY" );
	p.AddButton( "Edit", Vector2i( 100, 100 ), Vector2f( 100, 50 ), "EDIT" );
	p.AddButton( "Create New", Vector2i( 100, 200 ), Vector2f( 175, 50 ), "CREATE NEW" );
	p.AddButton( "Delete", Vector2i( 500, 10), Vector2f( 150, 50 ), "DELETE" );

	//TextBox *test = p.AddTextBox("description", Vector2i(500, 500), 5, 12, 20, 100, "0123456789\n0123456789");//"hello world here\nI am doing my thing");

	//TextBox *test2 = p.AddTextBox("description2", Vector2i(500, 750), 5, 10, 20, 100, "0123456789\n0123456789");//"hello world here\nI am doing my thing");

	p.AddButton("Quickplay", Vector2i(1150, 300), Vector2f(300, 50), "QUICKPLAY");
	//p.AddButton( "Upload", Vector2i(1150, 400), Vector2f(300, 50), "UPLOAD TO SERVER");
	//p.AddButton( "List", Vector2i(1150, 500), Vector2f(300, 50), "PRINT SERVER MAPS");
	//p.AddButton("Download", Vector2i(1150, 600), Vector2f(300, 50), "DOWNLOAD A MAP");
	//p.AddButton("Remove", Vector2i(1150, 900), Vector2f(300, 50), "REMOVE A MAP");
	
	//p.AddButton("Remove", Vector2i(600, 300), Vector2f(300, 50), "REMOVE FROM SERVER");

	Panel namePopup( "name popup", 500, 200, &customMapHandler );
	namePopup.pos = Vector2i( 960 - 250, 540 );
	namePopup.AddButton( "ok", Vector2i( 300, 0 ), Vector2f( 100, 50 ), "OK" );	
	namePopup.AddTextBox( "name", Vector2i( 10, 10 ), 300, 40, "test" );

	

	Panel downloadPopup("remove popup", 500, 200, &customMapHandler);
	downloadPopup.pos = Vector2i(960 - 250, 540);
	downloadPopup.AddButton("downloadok", Vector2i(300, 0), Vector2f(100, 50), "OK");
	downloadPopup.AddTextBox("index", Vector2i(10, 10), 300, 40, "0");

	Panel loginPopup("login popup", 800, 200, &customMapHandler);
	loginPopup.pos = Vector2i(960 - 400, 540);
	loginPopup.AddButton("loginok", Vector2i(630, 0), Vector2f(100, 50), "OK");
	loginPopup.AddTextBox("pass", Vector2i(10, 10), 300, 40, "");
	loginPopup.AddTextBox("user", Vector2i(320, 10), 300, 40, "");

	Panel removePopup("remove popup", 500, 200, &customMapHandler);
	removePopup.pos = Vector2i(960 - 250, 540);
	removePopup.AddButton("removeok", Vector2i(300, 0), Vector2f(100, 50), "OK");
	removePopup.AddTextBox("index", Vector2i(10, 10), 300, 40, "0");
	//Panel loginPopup( "login popup")
	//bool showNamePopup = false;

	ls.UpdateMapList();

	//ls.LoadAndRewriteAllMaps();

	//empty map. fix this later
	string empty = "5\n0 0\nmat\n5\n-209 78\n286 78\n286 132\n60 132\n-201 132\n0\n0\n0\n1\n-- 1\ngoal -air 0 0 76";

	SetMouseGrabbed(true);
	SetMouseVisible(true);

	UIMouse::GetInstance();
	UIController::GetInstance();
	//window->setMouseCursorVisible(true);
	//window->setMouseCursorGrabbed(true);

	while( !customMapQuit)
	{
		window->clear();
		
		
		Vector2i mousePos = sf::Mouse::getPosition( *window );
		//mousePos /= 2;
		

		//window.setView( uiView );
		Vector2f uiMouse = window->mapPixelToCoords( mousePos );
		MOUSE.Update(GetPixelPos());
		//MOUSE.Update( Mouse::isButtonPressed( Mouse::Left ), Mouse::isButtonPressed)
		ls.MouseUpdate( uiMouse );
		//Vector2f uiMouse = Vector2f( mousePos.x   window->getSize().x, mousePos.y ) * Vector2f(, window->getSize().y );

		//if( !customMapHandler.showNamePopup )
		
		while( window->pollEvent( ev ) )
		{
		//	cout << "some input" << endl;
			switch( ev.type )
			{
			case sf::Event::KeyPressed:
				{
					if( ev.key.code == Keyboard::Escape )
					{
						window->setView( v );
						customMapQuit = true;
					}
					else
					{
						if( customMapHandler.showNamePopup )
						{
							//ls.newLevelName = "";
							ls.newLevelName = "";
							namePopup.SendKey( ev.key.code, ev.key.shift );
							CopyMap(&customMapHandler, &namePopup);
						}
						else if (customMapHandler.showDownloadPopup)
						{
							downloadPopup.SendKey(ev.key.code, ev.key.shift);
						}
						else if (customMapHandler.showLoginPopup)
						{
							loginPopup.SendKey(ev.key.code, ev.key.shift);
						}
						else if (customMapHandler.showRemovePopup)
						{
							removePopup.SendKey(ev.key.code, ev.key.shift);
						}
						else
						{
							/*if (test->focused)
							{
								test->SendKey(ev.key.code, ev.key.shift);
							}

							if (test2->focused)
							{
								test2->SendKey(ev.key.code, ev.key.shift);
							}*/
						}
						
						
					}
					break;
				}
			case sf::Event::MouseButtonPressed:
				{
					if( ev.mouseButton.button == Mouse::Button::Left )
					{
						if( customMapHandler.showNamePopup )
						{
							namePopup.MouseUpdate();
						}
						else if (customMapHandler.showDownloadPopup)
						{
							downloadPopup.MouseUpdate();
						}
						else if (customMapHandler.showLoginPopup)
						{
							loginPopup.MouseUpdate();
						}
						else if (customMapHandler.showRemovePopup)
						{
							removePopup.MouseUpdate();
						}
						else
						{
							p.MouseUpdate();
							//cout << "blah: " << mousePos.x << ", " << mousePos.y << endl;
							ls.LeftClick( true, uiMouse );
						}
					}
					break;
				}
			case sf::Event::MouseButtonReleased:
				{
					if( ev.mouseButton.button == Mouse::Button::Left )
					{
						if (customMapHandler.showNamePopup)
						{
							ls.newLevelName = "";
							namePopup.MouseUpdate();
							CopyMap(&customMapHandler, &namePopup);
						}
						else if (customMapHandler.showDownloadPopup)
						{
							downloadPopup.MouseUpdate();
						}
						else if (customMapHandler.showLoginPopup)
						{
							loginPopup.MouseUpdate();
						}
						else if (customMapHandler.showRemovePopup)
						{
							removePopup.MouseUpdate();
						}
						else
						{
							p.MouseUpdate();
							ls.LeftClick( false, uiMouse );
							if( ls.text[ls.selectedIndex].getFillColor() == Color::Red )
							{
								p.buttons["Create New"]->text.setString( "CREATE NEW" );
							}
							else
							{
								p.buttons["Create New"]->text.setString( "CREATE COPY" );
							}
						}
					}
					break;
				}
			case sf::Event::MouseWheelMoved:
			 {
				 //std::cout << event.mouseWheel.delta << '\n';
				ls.ChangeViewOffset(-ev.mouseWheel.delta);
				break;
			 }
			}
		}

		p.Draw( window );
		ls.Draw( window );

		if( customMapHandler.showNamePopup )
		{
			namePopup.Draw( window );
		}
		else if (customMapHandler.showDownloadPopup)
		{
			downloadPopup.Draw(window);
		}
		else if (customMapHandler.showLoginPopup)
		{
			loginPopup.Draw(window);
		}
		else if (customMapHandler.showRemovePopup)
		{
			removePopup.Draw(window);
		}
		

		window->display();
		
		
	}
	//window->setView( v );

	window->setMouseCursorVisible(false);
	//window->setMouseCursorGrabbed(false);

	window->setView(oldView);
	SetMouseGrabbed(true);
	//SetMouseVisible(false);
	//preScreenTexture->setView(oldPreView);
}

void MainMenu::GGPOOption()
{
	EditSession *edit = new EditSession(this, "Resources\\Maps\\W2\\gateblank91.brknk");
	edit->Run();
	delete edit;
}

sf::IntRect MainMenu::GetButtonIconTile(int controllerIndex, int baseButtonIndex)
{
	ControllerType controllerType = GetController(controllerIndex).GetCType();

	/*CTYPE_XBOX,
		CTYPE_GAMECUBE,
		CTYPE_PS4,
		CTYPE_PS5,
		CTYPE_KEYBOARD,
		CTYPE_NONE,*/

	switch (controllerType)
	{
	case CTYPE_XBOX:
		return ts_buttonIcons->GetSubRect(baseButtonIndex);
	case CTYPE_GAMECUBE:
		return ts_buttonIcons->GetSubRect(baseButtonIndex + 16 * 2);
	case CTYPE_PS5:
	case CTYPE_PS4:
		return ts_buttonIcons->GetSubRect(baseButtonIndex + 16);
	case CTYPE_KEYBOARD:
		return ts_keyboardIcons->GetSubRect(baseButtonIndex);
	}
}

Tileset * MainMenu::GetButtonIconTileset(int controllerIndex )
{
	ControllerType controllerType = GetController(controllerIndex).GetCType();

	if (controllerType == CTYPE_KEYBOARD)
	{
		return ts_keyboardIcons;
	}
	else
	{
		return ts_buttonIcons;
	}
}

ControllerState &MainMenu::GetPrevInput( int index )
{
	return prevInput[index];
}


ControllerState &MainMenu::GetCurrInput( int index )
{
	return currInput[index];
}

ControllerState &MainMenu::GetPrevInputUnfiltered(int index)
{
	return prevInputUnfiltered[index];
}


ControllerState &MainMenu::GetCurrInputUnfiltered(int index)
{
	return currInputUnfiltered[index];
}


void MainMenu::CopyMap( CustomMapsHandler *cmh, Panel *namePop )
{
	Panel &namePopup = *namePop;
	CustomMapsHandler &customMapHandler = *cmh;
	LevelSelector &ls = *levelSelector;
	//if (customMapHandler.showNamePopup)
	{
		if (ls.newLevelName != "")
		{

			if (ls.text[ls.selectedIndex].getFillColor() == Color::Red)
			{
				path from("Resources/Maps/empty.brknk");

				std::stringstream ssPath;
				ssPath << ls.localPaths[ls.selectedIndex] << ls.newLevelName << ".brknk";
				string toString = ssPath.str();
				path to(toString);

				try
				{
					boost::filesystem::copy_file(from, to, copy_option::fail_if_exists);

					//GameEditLoop( ls.GetSelectedPath() );

					ls.UpdateMapList();

					window->setView(uiView);
				}
				catch (const boost::system::system_error &err)
				{
					cout << "file already exists!" << endl;
				}
			}
			else
			{
				path from(ls.GetSelectedPath());
				TreeNode * toNode = ls.dirNode[ls.selectedIndex];

				std::stringstream ssPath;
				ssPath << "Resources/" << toNode->GetLocalPath() << ls.newLevelName << ".brknk";
				string toString = ssPath.str();

				path to(toString);

				try
				{
					boost::filesystem::copy_file(from, to, copy_option::fail_if_exists);

					//cout << "copying to: " << to.string() << endl;
					//GameEditLoop( toNode->GetLocalPath() );

					ls.UpdateMapList();

					window->setView(uiView);
				}
				catch (const boost::system::system_error &err)
				{
					cout << "file already exists!" << endl;
				}
			}
		}
	}

}

void MainMenu::UpdateMenuInput()
{
	menuPrevInput = menuCurrInput;
	menuCurrInput.Set(ControllerState());

	//int upCount = 0;
	//int downCount = 0;

	vector<GCC::GCController> controllers;
	if (gccDriverEnabled)
		controllers = gccDriver->getState();


	for (int i = 0; i < 4; ++i)
	{
		ControllerState &prevInput = GetPrevInputUnfiltered(i);
		ControllerState &currInput = GetCurrInputUnfiltered(i);
		GameController &c = GetController(i);

		prevInput = currInput;


		if (gccDriverEnabled)
			c.gcController = controllers[i];
		bool active = c.UpdateState();

		if (active)
		{
			currInput = c.GetUnfilteredState();

			menuCurrInput.A |= (currInput.A && !prevInput.A);
			menuCurrInput.B |= (currInput.B && !prevInput.B);
			menuCurrInput.X |= (currInput.X && !prevInput.X);
			menuCurrInput.Y |= (currInput.Y && !prevInput.Y);
			menuCurrInput.rightShoulder |= (currInput.rightShoulder && !prevInput.rightShoulder);
			menuCurrInput.leftShoulder |= (currInput.leftShoulder && !prevInput.leftShoulder);
			menuCurrInput.start |= (currInput.start && !prevInput.start);
			menuCurrInput.leftTrigger = max(menuCurrInput.leftTrigger, currInput.leftTrigger);
			menuCurrInput.rightTrigger = max(menuCurrInput.rightTrigger, currInput.rightTrigger);
			menuCurrInput.back |= (currInput.back && !prevInput.back);
			menuCurrInput.leftStickPad |= currInput.leftStickPad;

			/*menuCurrInput.A |= currInput.A;
			menuCurrInput.B |= currInput.B;
			menuCurrInput.X |= currInput.X;
			menuCurrInput.Y |= currInput.Y;
			menuCurrInput.rightShoulder |= currInput.rightShoulder;
			menuCurrInput.leftShoulder |= currInput.leftShoulder;
			menuCurrInput.start |= currInput.start;
			menuCurrInput.leftTrigger = max(menuCurrInput.leftTrigger, currInput.leftTrigger);
			menuCurrInput.rightTrigger= max(menuCurrInput.rightTrigger, currInput.rightTrigger);
			menuCurrInput.back |= currInput.back;
			menuCurrInput.leftStickPad |= currInput.leftStickPad;*/
		}
		else
		{
			currInput.Set(ControllerState());
		}

	}
}


#include <sfeMovie/Movie.hpp>

void MainMenu::Slide()
{
	CubicBezier bez(0, 0, 1, 1);
	if (slideCurrFrame == 0)
		v.setCenter(slideStart);
	else if (slideCurrFrame == numSlideFrames)
		v.setCenter(slideEnd);
	else
	{
		float f = bez.GetValue((double)slideCurrFrame / numSlideFrames);
		v.setCenter(slideStart * (1 - f) + slideEnd * f);
	}

	slideCurrFrame++;
}

//#define USE_MOVIE_TEST
void MainMenu::Run()
{

	sf::Event ev;

	quit = false;
	currentTime = 0;
	accumulator = TIMESTEP + .1;

	

#if defined( USE_MOVIE_TEST )
	sfe::Movie m;
	assert( m.openFromFile("Resources/Movie/crawler_slash.ogv") );
	m.setVolume(100);
	m.fit(sf::FloatRect(0, 0, 1920, 1080));

	
	m.play();
#endif
	/*int moveDownFrames = 0;
	int moveUpFrames = 0;
	int moveLeftFrames = 0;
	int moveRightFrames = 0;*/
	

	gameClock.restart();

	

	//SetMode(TRANS_MAIN_TO_OPTIONS);
	//SetMode(TRANS_MAIN_TO_SAVE);
	//SetMode(TRANS_MAIN_TO_CREDITS);
	//SetMode(TRANS_MAIN_TO_MAPSELECT);
	//SetMode(SPLASH);
	menuMode = TITLEMENU;

	vector<ControllerState> controllerStatesForMouse;
	controllerStatesForMouse.reserve(4);
	
#if defined( USE_MOVIE_TEST )
	sf::Shader sh;
	assert( sh.loadFromFile("Resources/Shader/test.frag", sf::Shader::Fragment ) );
	
	sf::Vertex ff[4] = {
		sf::Vertex( Vector2f( 0, 0 ) ),
		sf::Vertex(Vector2f(500, 0)),
		sf::Vertex(Vector2f(500, 500)),
		sf::Vertex(Vector2f(0, 500))
	};
#endif
	while( !quit )
	{
		//GGPOOption();
		//continue;

		double newTime = gameClock.getElapsedTime().asSeconds();
		double frameTime = newTime - currentTime;

		if ( frameTime > 0.25 )
		{
			frameTime = 0.25;	
		}
        currentTime = newTime;

		accumulator += frameTime;

		

		preScreenTexture->clear(Color::Transparent);
		window->clear(Color::Transparent);
		
	#if defined( USE_MOVIE_TEST )
		if (m.getStatus() == sfe::Status::Stopped)
		{
			m.setPlayingOffset(sf::Time::Zero);
			m.play();
		}

		m.update();
			//const sf::Texture &currImage = m.getCurrentImage();

			//sh.setUniform("texture", currImage);
	#endif

		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Srcreen And Depth Buffer
		
		while ( accumulator >= TIMESTEP  )
        {
			//gotta be honest i dont know what this does.
			//does it loop like this to fix some kind of drawing issue?
			do
			{
				UpdateMenuInput();

				mousePixelPos = GetPixelPos();

				controllerStatesForMouse.clear();
				for (int i = 0; i < 4; ++i)
				{
					controllerStatesForMouse.push_back(GetCurrInputUnfiltered(i));
				}

				MOUSE.Update(mousePixelPos, controllerStatesForMouse);

				UICONTROLLER.Update();

				//added this so going back from the thanks screen
				//doesnt inta select a level. carrying over inputs
				//between menus makes no sense.

				changedMode = false;
				HandleMenuMode();
			} while (changedMode);
			
			musicPlayer->Update();

			SteamAPI_RunCallbacks();

			fader->Update();
			swiper->Update();

			soundNodeList->Update();

			UpdateEffects();

			accumulator -= TIMESTEP;
		}

		if( quit )
		{
			break;
		}
		

		if (MOUSE.IsMouseDownLeft() )
		{
			customCursor->SetClicked();
		}
		else
		{
			customCursor->SetNormal();
		}

		DrawMode(menuMode);

#if defined( USE_MOVIE_TEST )
		preScreenTexture->draw(m);// , &sh);
#endif
		preScreenTexture->display();
		sf::Sprite pspr;
		pspr.setTexture( preScreenTexture->getTexture() );
		window->draw( pspr );
		window->display();
	}
}

void DispLoadTest( MainMenu *mm )
{
	RenderTexture *pTex = mm->preScreenTexture;
	RenderWindow * win = mm->window;
	win->setActive(true);
	while (!mm->doneLoading)
	{		
		pTex->clear(Color::Black);
		win->clear(Color::Red);
		
		mm->loadingBackpack->Update();

		pTex->draw(mm->loadingBGSpr);

		mm->loadingBackpack->Draw(pTex);

		mm->fader->Draw( EffectLayer::IN_FRONT_OF_UI, pTex);
		//mm->swiper->Draw( EffectLayer::IN_FRONT_OF_UI, pTex);
		pTex->display();
		sf::Sprite spr;
		spr.setTexture(pTex->getTexture());
		win->draw(spr);
		win->display();
	}

	win->setActive(false);
}

void MainMenu::ResizeWindow( int p_windowWidth, 
		int p_windowHeight, int p_style )
{
	if( windowWidth == p_windowWidth && windowHeight == p_windowHeight && style == p_style )
	{
		return;
	}
	
	windowWidth = p_windowWidth;
	windowHeight = p_windowHeight;

	if (style != p_style)
	{
		//sf::ContextSettings contextSettings( 0, 0, 0, 0)

		window->create( sf::VideoMode(windowWidth, windowHeight), "Breakneck",
			config->GetData().windowStyle, sf::ContextSettings());
		style = p_style;
	}
	else
	{
		window->setSize(Vector2u(windowWidth, windowHeight));
		window->setPosition(Vector2i(0, 0));
	}
}

void MainMenu::SetModeAdventureLoadingMap( int wIndex )
{
	musicPlayer->FadeOutCurrentMusic(30);
	SetMode(LOAD_ADVENTURE_MAP);
	//wIndex = min(wIndex, 1); //because there are only screens for 2 worlds

	stringstream ss;
	ss << "Menu/Load/load_w" << (wIndex + 1) << ".png";//"_1.png";

	if (ts_loadBG != NULL)
	{
		tilesetManager.DestroyTileset(ts_loadBG);
		ts_loadBG = NULL;
	}

	ts_loadBG = tilesetManager.GetTileset(ss.str(), 1920, 1080);

	loadingBackpack->SetScale(1.f);
	loadingBackpack->SetPosition(Vector2f(1920 - 260, 1080 - 200));
	//ts_loadBG[0] = tilesetManager.GetTileset("Menu/load_w1_1.png", 1920, 1080);
	//ts_loadBG[1] = tilesetManager.GetTileset("Menu/load_w2_1.png", 1920, 1080);
	//loadingIconBackpack[i].setPosition(1920 - 260, 1080 - 200);
	loadingBGSpr.setTexture(*ts_loadBG->texture);
}

void MainMenu::SetModeKinBoostLoadingMap(int variation)
{
	//fader->Clear();
	//fader->
	//swiper->Swipe(Swiper::W1, 15, true);
	SetMode(KINBOOSTLOADINGMAP);
	//preScreenTexture->setView(v);
	//wIndex = min(wIndex, 1); //because there are only screens for 2 worlds
	//loadingBGSpr.setTexture(*ts_loadBG[wIndex]->texture);


}

void MainMenu::AdventureLoadLevel(LevelLoadParams &loadParams)
{
	//window->setVerticalSyncEnabled(false);
	//window->setFramerateLimit(60);
	string levelPath = loadParams.adventureMap->GetMapPath();//lev->GetFullName();// name;
	//View oldView = window->getView();

	

	//preScreenTexture->setActive(false);
	//window->setActive(false);
	doneLoading = false;

	int wIndex = loadParams.world;
	gameRunType = GameRunType::GRT_ADVENTURE;
	SetModeAdventureLoadingMap(wIndex);

	//doneLoading = false;


	//loadThread = new boost::thread(DispLoadTest, this);// , currLevel);

	//sf::sleep(sf::milliseconds(5000));

	MatchParams mp;
	mp.saveFile = saveMenu->files[saveMenu->selectedSaveIndex];
	mp.mapPath = levelPath;

	currLevel = new GameSession(&mp);
	currLevel->bestTimeGhostOn = loadParams.bestTimeGhostOn;
	currLevel->bestReplayOn = loadParams.bestReplayOn;
	currLevel->level = loadParams.level;


	loadThread = new boost::thread(MainMenu::sLevelLoad, this, currLevel);
	//loadThread = new boost::thread(GameSession::sLoad, currLevel);

	accumulator = 0;//TIMESTEP + .1;
	currentTime = 0;
	gameClock.restart();
	//window->setActive(true);
	//currLevel->Load();

	//doneLoading = true;

	

	//loadThread->join();
	//doneLoading = true;

	//loadThread->join();
	//delete loadThread;
	//loadThread = NULL;

	//preScreenTexture->setActive(true);
	

	
	//delete loadThread;
	//loadThread = NULL;
	//SetMode(RUNNINGMAP);
	//if (loadingScreen)
	//{
	//	int wIndex = lev->sec->world->index;
	//	//SetModeKinBoostLoadingMap(wIndex);
	
	//}

	//loadThread = new boost::thread(GameSession::sLoad, currLevel);
}

void MainMenu::AdventureNextLevel(Level *lev)
{
	//window->setVerticalSyncEnabled(false);
	//window->setFramerateLimit(60);
	//string levelPath = lev->GetFullName();// name;
										  //View oldView = window->getView();
	worldMap->kinBoostScreen->level = lev;
	SetModeKinBoostLoadingMap(0);
	
	//kinBoostScreen->levName = levelPath;

	//deadThread = new boost::thread(MainMenu::sGoToNextLevel, this, levelPath);
}

void MainMenu::PlayIntroMovie()
{
	worldMap->state = WorldMap::COLONY;
	worldMap->selectedColony = 0;
	worldMap->selectedLevel = 0;
	//worldMap->testSelector->UpdateAllInfo();

	SetMode(INTROMOVIE);
	introMovie->Play();

	//musicPlayer->FadeOutCurrentMusic(30);
	MusicInfo *info = musicManager->songMap["w0_0_Film"];
	musicPlayer->TransitionMusic(info, 60, sf::seconds( 60 ));
	
	Level *lev = &(worldMap->planet->worlds[0].sectors[0].levels[0]);
	AdventureMap &am = worldMap->adventureFile.GetMap(lev->index);
	string levelPath = am.GetMapPath();//lev->GetFullName();
	//window->setActive(false);
	doneLoading = false;

	//int wIndex = lev->sec->world->index;
	gameRunType = GameRunType::GRT_ADVENTURE;
	//SetModeLoadingMap(wIndex);


	MatchParams mp;
	mp.saveFile = saveMenu->files[saveMenu->selectedSaveIndex];
	mp.mapPath = levelPath;
	currLevel = new GameSession(&mp);
	currLevel->level = lev;

	loadThread = new boost::thread(GameSession::sLoad, currLevel);

	accumulator = 0;//TIMESTEP + .1;
	currentTime = 0;
	gameClock.restart();
	//window->setActive(true);

	//AdventureLoadLevel(, false);
}

void MainMenu::sGoToNextLevel(MainMenu *m, AdventureMap *am, Level *lev )//const std::string &levName)
{

	//sf::sleep(sf::milliseconds(1000));

	//m->window->setVerticalSyncEnabled(false);

	

	SaveFile *currFile = m->GetCurrentProgress();
	currFile->Save();

	//AdventureMap &am = worldMap->
	string levName = am->GetMapPath();
	//Level *lev = &(currFile->worlds[0].sectors[0].levels[0]);
	//delete m->currLevel;

	GameSession *old = m->currLevel;
	////m->deadLevel = m->currLevel;

	delete old;

	
	MatchParams mp;
	mp.saveFile = m->saveMenu->files[m->saveMenu->selectedSaveIndex];
	mp.mapPath = levName;

	m->currLevel = new GameSession(&mp);
	m->currLevel->level = lev;
	m->currLevel->boostEntrance = true;
	//
	
	//
	GameSession::sLoad(m->currLevel);

	//sf::sleep(sf::milliseconds(50000));
	//m->loadThread = new boost::thread(GameSession::sLoad, m->currLevel);
	//m->loadThread->join();

	//delete m->loadThread;
	//m->loadThread = NULL;

	//delete	m->loadThread;
	//m->loadThread = NULL;

	//cout << "deleting deadLevel: " << m->deadLevel << endl;
	//delete m->deadLevel;

	//m->deadLevel = NULL;
}

void MainMenu::UpdateMenuMode()
{
	(this->*updateModeFuncs[menuMode])();
}

void MainMenu::ReturnToWorldAfterLevel()
{
	SingleAxisSelector *sa = worldMap->selectors[worldMap->selectedColony]->FocusedSector()->mapSASelector;
	int numLevels = worldMap->GetCurrSectorNumLevels();//worldMap->selectors[worldMap->selectedColony]->sectors[secIndex]->numLevels;

	

	int numWorlds = worldMap->planet->numWorlds;
	if (worldMap->selectedColony == numWorlds - 1)
	{
		if (sa->currIndex == numLevels - 1)
		{
			if (currSaveFile->GetNumCompleteWorlds(worldMap->planet) == numWorlds)
			{
				SetMode(THANKS_FOR_PLAYING);
				//return;
			}
		}
	}

	if (menuMode != THANKS_FOR_PLAYING)
	{
		worldMap->selectors[worldMap->selectedColony]->ReturnFromMap();
		SetMode(WORLDMAP_COLONY);
		worldMap->CurrSelector()->FocusedSector()->UpdateLevelStats();
		worldMap->CurrSelector()->FocusedSector()->UpdateStats();
		worldMap->CurrSelector()->FocusedSector()->UpdateMapPreview();

		worldMap->Update(menuPrevInput, menuCurrInput);

		musicPlayer->TransitionMusic(menuMusic, 60);
	}
	
}

void MainMenu::HandleMenuMode()
{

	//important note:

	//every mode MUST HANDLE EVENTS or the program will crash outside of the dev environment randomly.

	//while (window->pollEvent(ev))
	//{

	//}

	sf::Event ev;
	switch (menuMode)
	{
	case SPLASH:
	{
		while (window->pollEvent(ev))
		{

		}
		bool A = menuCurrInput.A && !menuPrevInput.A;
		bool B = menuCurrInput.B && !menuPrevInput.B;
		bool X = menuCurrInput.X && !menuPrevInput.X;
		bool Y = menuCurrInput.Y && !menuPrevInput.Y;
		bool r = menuCurrInput.rightShoulder && !menuPrevInput.rightShoulder;
		bool l = menuCurrInput.leftShoulder && !menuPrevInput.leftShoulder;

		if (A || B || X || Y || r || l)
		{
			SetMode(SPLASH_TRANS);
			changedMode = true;
		}
		break;
	}
	case SPLASH_TRANS:
	{
		while (window->pollEvent(ev))
		{

		}

		SetMode(TITLEMENU);

		break;
	}
	case TITLEMENU:
	{
		TitleMenuModeUpdate();
		break;
	}
	case TITLEMENU_INFOPOP:
	{
		while (window->pollEvent(ev))
		{

		}

		titleScreen->Update();
		if (infoPopup->Update(menuCurrInput, menuPrevInput))
		{
			cout << "exited info popup" << endl;
			//exited the info popup
			SetMode(TITLEMENU);
		}
		break;
	}
	case WORLDMAP:
	case WORLDMAP_COLONY:
	{
		while (window->pollEvent(ev))
		{
			switch (ev.type)
			{
			case sf::Event::KeyPressed:
			{
				if (ev.key.code == Keyboard::Escape)
				{
					//quit = true;
				}
			}
			}
		}

		worldMap->Update(menuPrevInput, menuCurrInput);

		break;
	}
	case LOAD_ADVENTURE_MAP:
	{
		//havent tested but should be necessary here
		while (window->pollEvent(ev))
		{
		}

		loadingBackpack->Update();

		if (loadThread != NULL)
		{
			if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
			{
				//window->setVerticalSyncEnabled(true);
				delete loadThread;
				loadThread = NULL;
				SetMode(RUN_ADVENTURE_MAP);
				//fader->CrossFade(30, 0, Color::Black);
				//return HandleMenuMode();
				//cout << "RUNNING MAP" << endl;
			}
			else
			{
				loadingBackpack->Update();
				//loadingIconBackpack[1].rotate(-1);
				//loadingIconBackpack[2].rotate(2);
			}
		}
		break;
	}
	case LOADINGMENUSTART:
	{
		//havent tested but should be necessary here
		while (window->pollEvent(ev))
		{
		}

		if (fader->IsFullyFadedOut())
		{
			StartLoadModeScreen();
		}
		break;
	}
	case LOADINGMENULOOP:
		//havent tested but should be necessary here
		while (window->pollEvent(ev))
		{
		}

		loadingBackpack->Update();

		if (loadThread != NULL)
		{
			if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
			{
				delete loadThread;
				loadThread = NULL;
				SetMode(LOADINGMENUEND);
				fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
			}
			else
			{
				loadingBackpack->Update();
			}
		}
		break;
	case LOADINGMENUEND:
	{
		//havent tested but should be necessary here
		while (window->pollEvent(ev))
		{
		}

		if (fader->IsFullyFadedOut())
		{
			fader->Fade(true, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
			SetMode(modeToLoad);
		}
		break;
	}
	case KINBOOSTLOADINGMAP:
	{
		while (window->pollEvent(ev))
		{

		}

		if (deadThread != NULL)
		{
			if (deadThread->try_join_for(boost::chrono::milliseconds(0)))
			{
				delete deadThread;
				deadThread = NULL;
			}
			else
			{
			}
		}

		if (deadThread == NULL)
		{
			if (loadThread != NULL)//loadThread->try_join_for(boost::chrono::milliseconds(0)))
			{
				//loadThread->join();

			}
		}

		if (worldMap->kinBoostScreen->IsEnded())//swiper->IsPostWipe())
		{
			//mainMenu->fader->Fade(true, 30, Color::Black, true);
			fader->Fade(true, 30, Color::Black, true, EffectLayer::IN_FRONT_OF_UI);
			gameRunType = GRT_ADVENTURE;
			SetMode(RUN_ADVENTURE_MAP);
		}
		else if (worldMap->kinBoostScreen->level == NULL && loadThread == NULL && deadThread == NULL && worldMap->kinBoostScreen->IsBoosting())
		{
			//gameRunType = GRT_ADVENTURE;
			//SetMode(RUNNINGMAP);
			worldMap->kinBoostScreen->End();

			//gameRunType = GRT_ADVENTURE;
			//SetMode(RUNNINGMAP);
			//window->setVerticalSyncEnabled(true);
			//kinBoostScreen->
			//kinBoostScreen->End();


			//swiper->Swipe(Swiper::W1, 15);
			//return HandleMenuMode();
		}
		else
		{
			//kinBoostScreen->Update();

			if (worldMap->kinBoostScreen->frame == 60 && worldMap->kinBoostScreen->IsBoosting())
			{
				//window->setVerticalSyncEnabled(false);
				//window->setFramerateLimit(60);
				//string levelPath = kinBoostScreen->level->GetFullName();//kinBoostScreen->levName;
				Level *lev = worldMap->kinBoostScreen->level;
				deadThread = new boost::thread(MainMenu::sGoToNextLevel, this, &worldMap->adventureFile.GetMap(lev->index), lev);
				worldMap->kinBoostScreen->level = NULL;
			}

		}
		worldMap->kinBoostScreen->Update();
		break;
	}

	case RUN_ADVENTURE_MAP:
	{
		while (window->pollEvent(ev))
		{

		}
		View oldView = window->getView();
		//cout << "running currLevel: " << currLevel << endl;
		GameSession::GameResultType result =
			(GameSession::GameResultType)currLevel->Run();



		SaveFile *currFile = GetCurrentProgress();
		if (result == GameSession::GR_WIN || result == GameSession::GR_WINCONTINUE)
		{
			worldMap->CompleteCurrentMap(currLevel->level, currLevel->totalFramesBeforeGoal);
		}
		switch (result)
		{
		case GameSession::GR_EXITLEVEL:
			//currFile->Save();
			break;
		case GameSession::GR_EXITTITLE:
			//currFile->Save();
			break;
		case GameSession::GR_EXITGAME:
			//currFile->Save();
			break;
		}

		window->setView(oldView);

		//JUST FOR TESTING
		//worldMap = new WorldMap(this);
		//saveMenu = new SaveMenuScreen(this);
		//saveMenu->Reset();
		//----ENDING JUST FOR TESTING

		SingleAxisSelector *sa = worldMap->selectors[worldMap->selectedColony]->FocusedSector()->mapSASelector;
		int numLevels = worldMap->GetCurrSectorNumLevels();//worldMap->selectors[worldMap->selectedColony]->sectors[secIndex]->numLevels;
		if (result == GameSession::GR_WIN)
		{
			currFile->Save();

			delete currLevel;
			currLevel = NULL;
			fader->Clear();

			ReturnToWorldAfterLevel();
		}
		else if (result == GameSession::GR_WINCONTINUE)
		{
			if (sa->currIndex < numLevels - 1)
			{
				sa->currIndex++;

				AdventureNextLevel(&(worldMap->GetCurrSector().levels[sa->currIndex]));
			}
			else
			{
				currFile->Save();

				delete currLevel;
				currLevel = NULL;

				fader->Clear();

				ReturnToWorldAfterLevel();
			}

		}
		else if (result == GameSession::GR_EXITTITLE)
		{
			Sector &sec = worldMap->GetCurrSector();
			for (int i = 0; i < sec.numLevels; ++i)
			{
				currFile->SetLevelNotJustBeaten(&sec.levels[i]);
			}
			//fix this later for other options

			currFile->Save();

			delete currLevel;
			currLevel = NULL;

			LoadMode(TITLEMENU);
		}
		else if (result == GameSession::GR_EXITGAME)
		{
			Sector &sec = worldMap->GetCurrSector();
			for (int i = 0; i < sec.numLevels; ++i)
			{
				currFile->SetLevelNotJustBeaten(&sec.levels[i]);
			}
			//fix this later for other options

			currFile->Save();

			delete currLevel;
			currLevel = NULL;

			SetMode(EXITING);
			//LoadMode(TITLEMENU);

			quit = true;
		}
		else
		{
			Sector &sec = worldMap->GetCurrSector();
			for (int i = 0; i < sec.numLevels; ++i)
			{
				currFile->SetLevelNotJustBeaten(&sec.levels[i]);
			}
			//fix this later for other options

			currFile->Save();

			delete currLevel;
			currLevel = NULL;

			ReturnToWorldAfterLevel();

			//fader->CrossFade(30, 0, Color::Black);
		}
		break;

	}
	case RUN_EDITOR_MAP:
	{
		while (window->pollEvent(ev))
		{

		}

		View oldView = window->getView();

		int result = currEditSession->Run();

		window->setView(oldView);
		SetMouseVisible(false);

		//LoadMode(TITLEMENU);
		LoadMode(preEditMode);
		break;
	}
	case TUTORIAL:
	{
		while (window->pollEvent(ev))
		{

		}

		View oldView = window->getView();

		int result = currTutorialSession->Run();

		window->setView(oldView);

		if (result == GameSession::GR_EXITGAME)
		{
			SetMode(EXITING);
			quit = true;
		}
		else
		{
			LoadMode(TITLEMENU);
		}

		break;
	}
	case ADVENTURETUTORIAL:
	{
		while (window->pollEvent(ev))
		{

		}

		View oldView = window->getView();

		int result = currTutorialSession->Run();

		if (result == GameSession::GR_EXITTITLE)
		{
			LoadMode(TITLEMENU);

		}
		else if (result == GameSession::GR_EXITGAME)
		{
			SetMode(EXITING);
			quit = true;
		}
		else
		{
			LoadMode(WORLDMAP);
		}

		window->setView(oldView);

		//worldMap->state = WorldMap::PLANET;
		//worldMap->frame = 0;




		break;
	}
	case SAVEMENU:
	{
		while (window->pollEvent(ev))
		{

		}
		worldMap->Update(menuPrevInput, menuCurrInput);
		if (!saveMenu->Update())
		{
			musicPlayer->FadeOutCurrentMusic(30);
			LoadMode(TITLEMENU);
		}

		break;
	}
	case TRANS_MAIN_TO_SAVE:
	{
		while (window->pollEvent(ev))
		{

		}
		if (transFrame == transLength)
		{
			SetMode(SAVEMENU);
			break;
		}


		if (transFrame * 2 == transLength)
		{
			saveMenu->Reset();
		}

		if (transFrame < transLength / 2)
		{
			titleScreen->Update();
		}
		else
		{
			saveMenu->Update();
			worldMap->Update(menuPrevInput, menuCurrInput);
		}
		++transFrame;
		break;
	}
	case TRANS_SAVE_TO_MAIN:
	{
		while (window->pollEvent(ev))
		{

		}
		UpdateMenuOptionText();


		if (transFrame == transLength)
		{
			SetMode(TITLEMENU);
			break;
		}


		if (transFrame * 2 == transLength)
		{
			saveMenu->Reset();
		}

		if (transFrame < transLength / 2)
		{
			saveMenu->Update();
			worldMap->Update(menuPrevInput, menuCurrInput);
		}
		else
		{
			titleScreen->Update();
		}
		++transFrame;
		break;
	}
	case TRANS_SAVE_TO_WORLDMAP:
	{
		while (window->pollEvent(ev))
		{

		}
		SetMode(WORLDMAP);

		break;
	}
	case MULTIPREVIEW:
	{
		while (window->pollEvent(ev))
		{

		}
		multiLoadingScreen->Update();
		break;
	}
	case TRANS_MAPSELECT_TO_MULTIPREVIEW:
	{
		while (window->pollEvent(ev))
		{

		}
		SetMode(MULTIPREVIEW);
		/*if (slideCurrFrame > numSlideFrames)
		{
		menuMode = MULTIPREVIEW;
		}
		else
		{
		Slide();
		}*/
		break;
	}
	case TRANS_MULTIPREVIEW_TO_MAPSELECT:
	{
		while (window->pollEvent(ev))
		{

		}
		SetMode(MAPSELECT);
		/*if (slideCurrFrame > numSlideFrames)
		{
		menuMode = MAPSELECT;
		}
		else
		{
		Slide();
		}*/
		break;
	}
	case TRANS_MAIN_TO_MAPSELECT:
	{
		while (window->pollEvent(ev))
		{

		}
		SetMode(MAPSELECT);
		/*if (slideCurrFrame > numSlideFrames)
		{
		menuMode = MAPSELECT;
		}
		else
		{
		Slide();
		}*/
		break;
	}
	case MAPSELECT:
	{
		while (window->pollEvent(ev))
		{

		}
		mapSelectionMenu->Update(menuCurrInput, menuPrevInput);
		break;
	}
	case TRANS_MAPSELECT_TO_MAIN:
	{
		while (window->pollEvent(ev))
		{

		}
		SetMode(TITLEMENU);
		break;
	}
	case TRANS_MAIN_TO_OPTIONS:
	{
		while (window->pollEvent(ev))
		{

		}
		SetMode(OPTIONS);
		config->WaitForLoad();

		optionsMenu->Load();
		optionsMenu->Center(Vector2f(1920, 1080));
		/*if (slideCurrFrame > numSlideFrames)
		{
		menuMode = OPTIONS;
		config->WaitForLoad();

		optionsMenu->Load();
		}
		else
		{
		Slide();
		}*/
		break;
	}
	case OPTIONS:
	{
		while (window->pollEvent(ev))
		{

		}

		optionsMenu->Update(menuCurrInput, menuPrevInput);
		break;
	}
	case TRANS_OPTIONS_TO_MAIN:
	{
		while (window->pollEvent(ev))
		{

		}
		SetMode(TITLEMENU);
		break;
	}
	case TRANS_MAIN_TO_CREDITS:
	{
		while (window->pollEvent(ev))
		{

		}
		SetMode(CREDITS);
		/*if (slideCurrFrame > numSlideFrames)
		{
		menuMode = CREDITS;
		}
		else
		{
		Slide();
		}*/
		break;
	}
	case CREDITS:
	{
		while (window->pollEvent(ev))
		{

		}
		
		creditsMenu->Update();
		break;
	}
	case TRANS_CREDITS_TO_MAIN:
	{
		while (window->pollEvent(ev))
		{

		}
		SetMode(TITLEMENU);
		break;
	}
	case TRANS_WORLDMAP_TO_LOADING:
	{
		while (window->pollEvent(ev))
		{

		}
		if (modeFrame == 1)
		{
			fader->CrossFade(30, 0, 30, Color::Black);
		}

		if (modeFrame == 30)
		{
			worldMap->RunSelectedMap();
		}
		else if (modeFrame < 30)
		{
			menuPrevInput = ControllerState();
			menuCurrInput = ControllerState();
			worldMap->Update(menuPrevInput, menuCurrInput);
		}
		break;
	}
	case INTROMOVIE:
	{
		while (window->pollEvent(ev))
		{

		}
		ControllerState &introInput = GetCurrInputUnfiltered(0);
		introMovie->skipHolder->Update(introInput.A);
		if (!introMovie->Update())
		{
			//fader->CrossFade(30, 0, 30, Color::Black);
			introMovie->Stop();

			if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
			{
				delete loadThread;
				loadThread = NULL;
				SetMode(RUN_ADVENTURE_MAP);
				gameRunType = GameRunType::GRT_ADVENTURE;
			}
			else
			{
				//menuMode = LOADINGMAP;
				gameRunType = GameRunType::GRT_ADVENTURE;
				SetModeAdventureLoadingMap(0);
			}

		}

		break;
	}
	case THANKS_FOR_PLAYING:
	{
		while (window->pollEvent(ev))
		{

		}

		if (menuCurrInput.A)
		{
			worldMap->CurrSelector()->ReturnFromMap();
			SetMode(WORLDMAP_COLONY);
			worldMap->CurrSelector()->FocusedSector()->UpdateLevelStats();
			worldMap->CurrSelector()->FocusedSector()->UpdateStats();
			worldMap->CurrSelector()->FocusedSector()->UpdateMapPreview();

			//menuCurrInput.Clear();
			//worldMap->Update(menuPrevInput, menuCurrInput);

			musicPlayer->TransitionMusic(menuMusic, 60);
			//SetMode(WORLDMAP);
		}
		break;
	}
	case RUN_FREEPLAY_MAP:
	{
		while (window->pollEvent(ev))
		{

		}

		View oldView = window->getView();

		int result = currFreePlaySession->Run();

		window->setView(oldView);

		if (result == GameSession::GR_EXITGAME)
		{
			SetMode(EXITING);
			quit = true;
		}
		else
		{
			LoadMode(BROWSE_WORKSHOP);
		}

		break;
	}
	case BROWSE_WORKSHOP:
	{
		while (window->pollEvent(ev))
		{
			workshopBrowser->HandleEvent(ev);//browserHandler->chooser->panel->HandleEvent(ev);
		}

		workshopBrowser->Update();

		if (workshopBrowser->action == WorkshopBrowser::A_BACK)
		{
			delete workshopBrowser;
			workshopBrowser = NULL;

			SetMode(ONLINE_MENU);
			break;
		}
		else if (workshopBrowser->workshopMapPopup->action == WorkshopMapPopup::A_HOST)
		{
			workshopBrowser->workshopMapPopup->action = WorkshopMapPopup::A_ACTIVE;
			SetMode(CUSTOM_MATCH_SETUP_FROM_WORKSHOP_BROWSER);
			customMatchManager->CreateCustomLobbyFromWorkshopBrowser();
			//delete workshopBrowser;
			//workshopBrowser = NULL;
		}
		else if (workshopBrowser->workshopMapPopup->action == WorkshopMapPopup::A_EDIT)
		{
			workshopBrowser->workshopMapPopup->action = WorkshopMapPopup::A_ACTIVE;
			RunEditor(BROWSE_WORKSHOP, workshopBrowser->savedPath);
		}

		//workshopBrowser->Update();

		/*if (mapBrowserScreen->browserHandler->chooser->action == MapBrowser::A_CANCELLED)
		{
			SetMode(ONLINE_MENU);
		}
		else if (mapBrowserScreen->browserHandler->chooser->selectedRect != NULL)
		{
			MapNode *mn = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;

			mapBrowserScreen->workshopPopup->Activate(mn);
		}*/

		
		break;
	}
	case ONLINE_MENU:
	{
		while (window->pollEvent(ev))
		{
			onlineMenuScreen->HandleEvent(ev);
		}
		onlineMenuScreen->Update();

		switch (onlineMenuScreen->action)
		{
		case OnlineMenuScreen::A_WORKSHOP:

			//mapBrowserScreen = MainMenu::GetInstance()->mapBrowserScreen;

			//mapBrowserScreen->StartWorkshopBrowsing( MapBrowser::WORKSHOP );

			assert(workshopBrowser == NULL);
			workshopBrowser = new WorkshopBrowser;

			workshopBrowser->Start();

			SetMode(BROWSE_WORKSHOP);

			break;
		case OnlineMenuScreen::A_QUICKPLAY:
			netplayManager->FindQuickplayMatch();
			SetMode(QUICKPLAY_TEST);
			break;
		case OnlineMenuScreen::A_CREATE_LOBBY:
			customMatchManager->CreateCustomLobby();
			SetMode(CUSTOM_MATCH_SETUP);		
			break;
		case OnlineMenuScreen::A_JOIN_LOBBY:
			customMatchManager->BrowseCustomLobbies();
			SetMode(CUSTOM_MATCH_SETUP);
			break;
		case OnlineMenuScreen::A_CANCELLED:
			SetMode(TITLEMENU);
			break;
		}
		
		break;
	}
	case CUSTOM_MATCH_SETUP:
	{
		while (window->pollEvent(ev))
		{
			customMatchManager->HandleEvent(ev);
		}

		if (!customMatchManager->Update())
		{
			SetMode(ONLINE_MENU);
			break;
		}

		if (customMatchManager->action == CustomMatchManager::A_READY)
		{
			SetMode(QUICKPLAY_TEST);
		}

		break;
	}
	case CUSTOM_MATCH_SETUP_FROM_WORKSHOP_BROWSER:
	{
		while (window->pollEvent(ev))
		{
			customMatchManager->HandleEvent(ev);
		}

		if (!customMatchManager->Update())
		{
			SetMode(BROWSE_WORKSHOP);
			break;
		}

		if (customMatchManager->action == CustomMatchManager::A_READY)
		{
			SetMode(QUICKPLAY_TEST);
		}
		break;
	}
	case DOWNLOAD_WORKSHOP_MAP_START:
	{
		while (window->pollEvent(ev))
		{

		}

		if (fader->IsFullyFadedOut())
		{
			fader->Fade(true, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
			SetMode(DOWNLOAD_WORKSHOP_MAP_LOOP);
			
			//loadThread = new boost::thread(MainMenu::sTransitionMode, this, modeLoadingFrom, modeToLoad);
			//StartLoadModeScreen();
		}
		break;
	}
	case DOWNLOAD_WORKSHOP_MAP_LOOP:
		while (window->pollEvent(ev))
		{

		}

		loadingBackpack->Update();

		//put the code here to download if not responding etc just like in the workshop

		if ( fader->IsFullyFadedIn() && mapBrowserScreen->browserHandler->CheckIfSelectedItemInstalled())
		{
			cout << "map download complete" << endl;

			modeToLoad = FREEPLAY;//RUN_FREEPLAY_MAP;

			MapNode *selectedNode = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;

			if (selectedNode == NULL)
				assert(0);

			freeplayMapName = selectedNode->filePath.string();
			loadThread = new boost::thread(MainMenu::sTransitionMode, this, modeLoadingFrom, modeToLoad);
			SetMode(LOADINGMENULOOP);
		}
		break;
	case FREEPLAY:
	{
		while (window->pollEvent(ev))
		{

		}

		freeplayScreen->Update();

		if (freeplayScreen->action == FreeplayScreen::A_READY)
		{
			LoadMode(RUN_FREEPLAY_MAP);
		}
		break;
	}
	case QUICKPLAY_TEST:
		while (window->pollEvent(ev))
		{
			switch (ev.type)
			{
			case sf::Event::KeyPressed:
			{
				if (ev.key.code == Keyboard::Escape)
				{
					SetMode(ONLINE_MENU);
					netplayManager->Abort();
					//quit = true;
				}
			}
			}
		}

		loadingBackpack->Update();
		
		netplayManager->Update();

		if (netplayManager->action == NetplayManager::A_WAITING_FOR_START_MESSAGE
			|| netplayManager->action == NetplayManager::A_READY_TO_RUN)
		{
			SetMode(QUICKPLAY_PLAY);
			fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
		}
		break;
	case QUICKPLAY_PLAY:
	{
		while (window->pollEvent(ev))
		{

		}

		loadingBackpack->Update();

		netplayManager->Update();

		if (netplayManager->action == NetplayManager::A_MATCH_COMPLETE)
		{
			
			/*MatchParams mp;
			mp.netplayManager = netplayManager;
			mp.numPlayers = 2;
			mp.filePath = "Resources/Maps/W2/afighting1.brknk";
			netplayManager->RunMatch(&mp);*/
		}
		else if (!fader->IsFading())
		{
			if (netplayManager->action == NetplayManager::A_READY_TO_RUN)
			{
				netplayManager->RunMatch();

				//results screen instead...

				if (netplayManager->action == NetplayManager::A_DISCONNECT)
				{
					//fix this so when the opponent disconnects after the game has ended that everything is still fine.
					cout << "EXITED ON DISCONNECT" << endl;
					infoPopup->Pop("Opponent disconnected", 60);
					SetMode(TITLEMENU_INFOPOP);

					netplayManager->CleanupMatch();
					netplayManager->Abort();
				}
				else
				{
					//netplayManager->CleanupMatch();
					//cout << "action test: " << (int)netplayManager->action << endl;
					//SetMode(TITLEMENU);

					//netplayManager->CleanupMatch();

					matchResultsScreen = netplayManager->resultsScreen;
					SetMode(MATCH_RESULTS);
				}
			}
		}
		break;
	}
	case MATCH_RESULTS:
	{
		while (window->pollEvent(ev))
		{
			//matchResultsScreen->HandleEvent(ev);
		}
		if (!matchResultsScreen->Update())
		{
			//netplayManager->CleanupMatch();

			SetMode(POST_MATCH_OPTIONS);
			//SetMode(TITLEMENU);
		}
		break;
	}
	case POST_MATCH_OPTIONS:
	{
		while (window->pollEvent(ev))
		{
			customMatchManager->postMatchPopup->HandleEvent(ev);
			//matchResultsScreen->HandleEvent(ev);
		}

		customMatchManager->postMatchPopup->Update();

		if (customMatchManager->postMatchPopup->action != PostMatchOptionsPopup::A_IDLE)
		{
			delete matchResultsScreen;
			matchResultsScreen = NULL;
		}

		switch (customMatchManager->postMatchPopup->action)
		{
		case PostMatchOptionsPopup::A_REMATCH:
		{
			netplayManager->game->InitGGPO();
			netplayManager->action = NetplayManager::A_READY_TO_RUN;
			fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);

			SetMode(QUICKPLAY_PLAY);
			break;
		}
		case PostMatchOptionsPopup::A_CHOOSE_MAP:
		{
			netplayManager->CleanupMatch();
			netplayManager->Abort();

			SetMode(TITLEMENU);
			break;
		}
		case PostMatchOptionsPopup::A_LEAVE:
		{
			netplayManager->CleanupMatch();
			netplayManager->Abort();

			SetMode(TITLEMENU);

			break;
		}

		}
		break;
	}
	}

	
	++modeFrame;
}


void MainMenu::RunEditor(Mode preMode, const std::string &p_editMapName)
{
	preEditMode = preMode;
	editMapName = p_editMapName;
	LoadMode(RUN_EDITOR_MAP);
}

void MainMenu::LoadMode(Mode m)
{
	fader->Fade(false, 60, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
	modeLoadingFrom = menuMode;

	SetMode(LOADINGMENUSTART);
	
	modeToLoad = m;
}

void MainMenu::StartLoadModeScreen()
{
	fader->Fade(true, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
	SetMode(LOADINGMENULOOP);
	loadThread = new boost::thread(MainMenu::sTransitionMode, this, modeLoadingFrom, modeToLoad);
}

ControlProfile *MainMenu::GetCurrSelectedProfile()
{
	return controlSettingsMenu->pSel->currProfile;
}

void MainMenu::UnlockSkin(int skinIndex)
{
	globalFile->UnlockSkin(skinIndex);
}

bool MainMenu::IsSkinUnlocked(int skinIndex)
{
	return globalFile->IsSkinUnlocked(skinIndex);
}



bool MainMenu::SetCurrProfileByName(const std::string &name)
{
	return controlSettingsMenu->pSel->SetCurrProfileByName(name);
}

#include <fstream>
MapCollection::MapCollection()
{
	expanded = false;
	tags = 0;
}

MapCollection::~MapCollection()
{
	for (auto mit = maps.begin(); mit != maps.end(); ++mit)
	{
		delete (*mit);
	}
}


void MainMenu::TitleMenuModeUpdate()
{
	titleScreen->Update();
	ts_menuSelector->SetSubRect(selectorSprite, selectorAnimFrame / selectorAnimFactor, true);
	//selectorSprite.setTextureRect(ts_menuSelector->GetSubRect(selectorAnimFrame / selectorAnimFactor));
	selectorSprite.setOrigin(selectorSprite.getLocalBounds().width / 2, selectorSprite.getLocalBounds().height / 2);

	selectorAnimFrame++;
	if (selectorAnimFrame == selectorAnimDuration * selectorAnimFactor)
	{
		selectorAnimFrame = 0;
	}
	
	sf::Event ev;
	while (window->pollEvent(ev))
	{
		switch (ev.type)
		{
		case sf::Event::KeyPressed:
		{
			if (ev.key.code == Keyboard::Escape)
			{
				//quit = true;
			}
			else if (ev.key.code == Keyboard::M && ev.key.alt)
			{
				//turned off for the beta
				musicPlayer->StopCurrentMusic();
				CustomMapsOption();
			}
			else if (ev.key.code == Keyboard::P)
			{
				//GGPOOption();
			}
			else if (ev.key.code == Keyboard::Return || ev.key.code == Keyboard::Space)
			{
				//menuMode = WORLDMAP;
				//worldMap->state = WorldMap::PLANET_AND_SPACE;//WorldMap::PLANET_AND_SPACE;
				//worldMap->frame = 0;
				//worldMap->UpdateMapList();

			}
			else if (ev.key.code == Keyboard::Up)
			{
				/*currentMenuSelect--;
				if( currentMenuSelect < 0 )
				currentMenuSelect = 4;*/
			}
			else if (ev.key.code == Keyboard::Down)
			{
				/*currentMenuSelect++;
				if( currentMenuSelect > 4 )
				currentMenuSelect = 0;*/
			}
			else
			{
				//titleMusic.stop();
				//GameEditLoop2( "Maps/aba.brknk" );
				//window->setView( v );
				//titleMusic.play();
			}
			break;
		}
		case sf::Event::MouseButtonPressed:
		{
			if (ev.mouseButton.button == Mouse::Button::Left)
			{
				//		ls.LeftClick( true, sf::Mouse::getPosition( *window ) );
			}
			break;
		}
		case sf::Event::MouseButtonReleased:
		{
			if (ev.mouseButton.button == Mouse::Button::Left)
			{
				//		ls.LeftClick( false, sf::Mouse::getPosition( *window ) );
			}
			break;
		}
		case sf::Event::Resized:
		{
			windowWidth = window->getSize().x;
			windowHeight = window->getSize().y;
			cout << "window: " << windowWidth << ", " << windowHeight << endl;
			break;
		}

		}
	}

	bool isMouseClickedOnCurrentOption = false;

	//probably refine this later to capture the mouse etc zzz
	if (QuadContainsPoint(mainMenuOptionQuads + saSelector->currIndex * 4,
		MOUSE.GetFloatPos()) && MOUSE.IsMouseLeftClicked())
	{
		isMouseClickedOnCurrentOption = true;
	}
	

	if (isMouseClickedOnCurrentOption || menuCurrInput.A || menuCurrInput.back || menuCurrInput.Y || menuCurrInput.X ||
		menuCurrInput.rightShoulder || menuCurrInput.leftShoulder)
	{
		soundNodeList->ActivateSound(soundManager.GetSound("main_menu_select"));
		switch (saSelector->currIndex)
		{
		case M_ADVENTURE:
		{
			//SetMode(MATCH_RESULTS);
			//matchResultsScreen = netplayManager->CreateResultsScreen();
			musicPlayer->FadeOutCurrentMusic(30);
			LoadMode(SAVEMENU);
			break;
		}
		case M_FREE_PLAY:
		{
			SetMode(TRANS_MAIN_TO_MAPSELECT);
			break;
		}
		case M_LOCAL_MULTIPLAYER:
		{
			onlineMenuScreen->Start();
			SetMode(ONLINE_MENU);

			//netplayManager->isSyncTest = true;
			//netplayManager->FindQuickplayMatch();
			//SetMode(QUICKPLAY_TEST);
			//SetMode(TRANS_MAIN_TO_MAPSELECT);
			break;
		}
		case M_LEVEL_EDITOR:
		{
			musicPlayer->FadeOutCurrentMusic(30);
			RunEditor(TITLEMENU, "");
				//SetMode(TRANS_MAIN_TO_MAPSELECT);
			break;
		}
		case M_OPTIONS:
		{
			//config->Load();
			Config::CreateLoadThread(config);
			SetMode(TRANS_MAIN_TO_OPTIONS);
			break;
		}
		case M_TUTORIAL:
		{
			musicPlayer->FadeOutCurrentMusic(30);
			LoadMode(TUTORIAL);
			break;
		}
		case M_CREDITS:
		{
			
			//cout << "EXITED ON DISCONNECT" << endl;
			//infoPopup->Pop("Opponent disconnected", 60);
			//SetMode(TITLEMENU_INFOPOP);

			/*if (IsKeyPressed(Keyboard::LShift))
			{
				customMatchManager->CreateCustomLobby();
			}
			else
			{
				customMatchManager->BrowseCustomLobbies();
			}

			SetMode(CUSTOM_MATCH_SETUP);*/
			

			//mapBrowserScreen->Start();
			//SetMode(BROWSE_WORKSHOP);

			//SetMode(TRANS_MAIN_TO_CREDITS);
			break;
		}
		case M_EXIT:
		{
			quit = true;
			break;
		}
		}
	}
	else
	{
		//turned off for beta
		if (IsKeyPressed(Keyboard::Tilde))
		{
			quit = true;
			return;
		}

		int oldIndex = saSelector->currIndex;
		int res = saSelector->UpdateIndex(menuCurrInput.LUp(), menuCurrInput.LDown());

		//if mouse is enabled, lets you mouse over to select
		for (int i = 0; i < saSelector->totalItems; ++i)
		{
			if (activatedMainMenuOptions[i] && saSelector->currIndex != i )
			{
				if (QuadContainsPoint(mainMenuOptionQuads + i * 4,
					MOUSE.GetFloatPos()))
				{
					saSelector->SetIndex(i);
					break;
				}
			}
		}
		

		if (res != 0)
		{
			soundNodeList->ActivateSound(soundManager.GetSound("main_menu_change"));
		}

		while (!activatedMainMenuOptions[saSelector->currIndex])
		{
			if (saSelector->currIndex != oldIndex)
			{
				if (saSelector->currIndex > oldIndex || (saSelector->currIndex == 0 && oldIndex == saSelector->totalItems - 1))
				{
					//down
					++saSelector->currIndex;
					if (saSelector->currIndex == saSelector->totalItems)
					{
						saSelector->currIndex = 0;
					}
				}
				else
				{
					--saSelector->currIndex;
					if (saSelector->currIndex < 0)
					{
						saSelector->currIndex = saSelector->totalItems - 1;
					}
				}
			}
		}

	}

	UpdateMenuOptionText();
}

void MainMenu::DrawMode( Mode m )
{
	switch (m)
	{
	case SPLASH:
	{
		preScreenTexture->draw(splashSprite);
		break;
	}
	case SPLASH_TRANS:
	{
		preScreenTexture->draw(splashSprite);
		break;
	}
	case TITLEMENU:
	{
		titleScreen->Draw(preScreenTexture);
		break;
	}
	case TITLEMENU_INFOPOP:
	{
		titleScreen->Draw(preScreenTexture);
		infoPopup->Draw(preScreenTexture);
		break;
	}
	case WORLDMAP:
	case WORLDMAP_COLONY:
	{
		preScreenTexture->setView(v);
		worldMap->Draw(preScreenTexture);
	}
	break;
	case TRANS_SAVE_TO_WORLDMAP:
	case SAVEMENU:
	{
		worldMap->Draw(preScreenTexture);
		saveMenu->Draw(preScreenTexture);
		break;
	}
	case MULTIPREVIEW:
	{
		multiLoadingScreen->Draw(preScreenTexture);
		break;
	}
	case LOAD_ADVENTURE_MAP:
	{
		preScreenTexture->setView(v);
		preScreenTexture->draw(loadingBGSpr);

		loadingBackpack->Draw(preScreenTexture);

		break;
	}
	case LOADINGMENUSTART:
	{
		DrawMode(modeLoadingFrom);
		break;
	}
	case LOADINGMENULOOP:
	{
		preScreenTexture->setView(v);
		loadingBackpack->Draw(preScreenTexture);
		break;
	}
	case LOADINGMENUEND:
	{
		DrawMode(LOADINGMENULOOP);
		break;
	}
	case KINBOOSTLOADINGMAP:
	{
		preScreenTexture->setView(v);
		worldMap->kinBoostScreen->Draw(preScreenTexture);
		break;
	}

	case TRANS_MAIN_TO_MAPSELECT:
	{
		preScreenTexture->setView(v);
		mapSelectionMenu->Draw(preScreenTexture);
		break;
	}
	case MAPSELECT:
	{
		preScreenTexture->setView(v);
		mapSelectionMenu->Draw(preScreenTexture);
		break;
	}
	case TRANS_MAPSELECT_TO_MAIN:
	{
		preScreenTexture->setView(v);
		mapSelectionMenu->Draw(preScreenTexture);
		break;
	}
	case TRANS_MAIN_TO_SAVE:
	{
		/*if (worldMap == NULL)
		{
		worldMap = new WorldMap(this);

		saveMenu = new SaveMenuScreen(this);
		}*/

		int tFrame = transFrame - 1;
		if (tFrame < transLength / 2)
		{
			titleScreen->Draw(preScreenTexture);
		}
		else
		{
			worldMap->Draw(preScreenTexture);
			saveMenu->Draw(preScreenTexture);
		}
		break;
	}
	case TRANS_SAVE_TO_MAIN:
	{
		int tFrame = transFrame - 1;
		if (tFrame < transLength / 2)
		{
			worldMap->Draw(preScreenTexture);
			saveMenu->Draw(preScreenTexture);
		}
		else
		{
			titleScreen->Draw(preScreenTexture);
		}
		break;
	}
	case TRANS_OPTIONS_TO_MAIN:
	{
		preScreenTexture->setView(v);
		optionsMenu->Draw(preScreenTexture);
		break;
	}
	case TRANS_MAIN_TO_OPTIONS:
	{
		preScreenTexture->setView(v);
		optionsMenu->Draw(preScreenTexture);
		break;
	}
	case OPTIONS:
	{
		preScreenTexture->setView(v);
		optionsMenu->Draw(preScreenTexture);
		break;
	}
	case TRANS_CREDITS_TO_MAIN:
	{
		preScreenTexture->setView(v);
		creditsMenu->Draw(preScreenTexture);
		break;
	}
	case TRANS_MAIN_TO_CREDITS:
	{
		preScreenTexture->setView(v);
		creditsMenu->Draw(preScreenTexture);
		break;
	}
	case CREDITS:
	{
		preScreenTexture->setView(v);
		
		creditsMenu->Draw(preScreenTexture);
		break;
	}
	case TRANS_MAPSELECT_TO_MULTIPREVIEW:
	{
		preScreenTexture->setView(v);
		mapSelectionMenu->Draw(preScreenTexture);
		multiLoadingScreen->Draw(preScreenTexture);
		break;
	}
	case TRANS_MULTIPREVIEW_TO_MAPSELECT:
	{
		preScreenTexture->setView(v);
		mapSelectionMenu->Draw(preScreenTexture);
		multiLoadingScreen->Draw(preScreenTexture);
		break;
	}
	case TRANS_WORLDMAP_TO_LOADING:
	{
		if (modeFrame < 30)
		{
			worldMap->Draw(preScreenTexture);
		}
		break;
	}
	case INTROMOVIE:
		introMovie->Draw(preScreenTexture);
		break;
	case RUN_EDITOR_MAP:
	{
		break;
	}
	case TUTORIAL:
	{
		break;
	}
	case ADVENTURETUTORIAL:
	{
		break;
	}
	case THANKS_FOR_PLAYING:
	{
		preScreenTexture->setView(v);
		preScreenTexture->draw(thanksQuad, 4, sf::Quads, ts_thanksForPlaying->texture);
		break;
	}
	case RUN_ADVENTURE_MAP:
	{
		break;
	}
	case BROWSE_WORKSHOP:
	{
		preScreenTexture->setView(v);
		workshopBrowser->Draw(preScreenTexture);
		break;
	}
	case ONLINE_MENU:
	{
		preScreenTexture->setView(v);
		onlineMenuScreen->Draw(preScreenTexture);
		break;
	}
	case CUSTOM_MATCH_SETUP_FROM_WORKSHOP_BROWSER:
	case CUSTOM_MATCH_SETUP:
	{
		preScreenTexture->setView(v);
		customMatchManager->Draw(preScreenTexture);
		break;
	}
	case RUN_FREEPLAY_MAP:
	{
		break;
	}
	case FREEPLAY:
	{
		freeplayScreen->Draw(preScreenTexture);
		break;
	}
	case DOWNLOAD_WORKSHOP_MAP_START:
	{
		DrawMode(modeLoadingFrom);
		break;
	}
	case DOWNLOAD_WORKSHOP_MAP_LOOP:
	{
		preScreenTexture->setView(v);
		loadingBackpack->Draw(preScreenTexture);
		break;
	}
	case QUICKPLAY_TEST:
	{
		preScreenTexture->setView(v);
		loadingBackpack->Draw(preScreenTexture);
		break;
	}
	case QUICKPLAY_PLAY:
	{
		preScreenTexture->setView(v);
		loadingBackpack->Draw(preScreenTexture);
		break;
	}
	case MATCH_RESULTS:
	{
		preScreenTexture->setView(v);
		matchResultsScreen->Draw(preScreenTexture);
		break;
	}
	case POST_MATCH_OPTIONS:
	{
		preScreenTexture->setView(v);
		matchResultsScreen->Draw(preScreenTexture);

		customMatchManager->postMatchPopup->Draw(preScreenTexture);

		break;
	}
	default:
		assert(0);
		break;
	}

	fader->Draw( EffectLayer::IN_FRONT_OF_UI, preScreenTexture);
	//swiper->Draw(preScreenTexture);
	DrawEffects(preScreenTexture);


	if (menuMode == KINBOOSTLOADINGMAP)
	{
		preScreenTexture->setView(v);
		worldMap->kinBoostScreen->DrawLateKin(preScreenTexture);
	}
}

void MainMenu::RunFreePlayMap(const std::string &path)
{
	freeplayMapName = path;
	LoadMode(MainMenu::RUN_FREEPLAY_MAP);
	//SetMode(MainMenu::RUNWORKSHOPMAP);
}

void MainMenu::DownloadAndRunWorkshopMap()
{
	//currWorkshopMap = path;
	mapBrowserScreen->browserHandler->SubscribeToItem();
	modeLoadingFrom = menuMode;	
	modeToLoad = MainMenu::DOWNLOAD_WORKSHOP_MAP_LOOP;
	SetMode(MainMenu::DOWNLOAD_WORKSHOP_MAP_START);

	fader->Fade(false, 60, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
	//modeLoadingFrom = menuMode;

	//SetMode(LOADINGMENUSTART);

	//modeToLoad = m;
	//LoadMode(MainMenu::DOWNLOAD_WORKSHOP_MAP_LOOP);
}

void MainMenu::SetToMatchResults(GameSession *p_game)
{

}