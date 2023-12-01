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
#include "SaveMenuScreen.h"

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
#include "AdventureManager.h"

#include "CustomMatchManager.h"
#include "OnlineMenuScreen.h"
#include "EditorMenuScreen.h"
#include "MatchResultsScreen.h"
#include "WorkshopMapPopup.h"
#include "WorkshopBrowser.h"
#include "FreeplayScreen.h"
#include "WorkshopManager.h"

#include "PostMatchOptionsPopup.h"
#include "UIMouse.h"
#include "UIController.h"
#include "LobbyBrowser.h"
#include "SinglePlayerControllerJoinScreen.h"
#include "GameSettingsScreen.h"
#include "globals.h"
#include "ClosedBetaScreen.h"
#include "CreditsMenuScreen.h"

#include "RemoteStorageManager.h"
#include "Leaderboard.h"
#include "FeedbackForm.h"
#include "FeedbackManager.h"

#include "Minimap.h"
#include "HitboxManager.h"
#include "RandomPicker.h"

using namespace std;
using namespace sf;
using namespace boost::filesystem;

#define TUTORIAL_PATH "Resources/Maps/EarlyAccess/Tutorial/tut1" + string(MAP_EXT)
#define TIMESTEP (1.0 / 60.0)

sf::RenderTexture *MainMenu::preScreenTexture = NULL;
sf::RenderTexture *MainMenu::postProcessTexture2 = NULL;
sf::RenderTexture *MainMenu::minimapTexture = NULL;
sf::RenderTexture *MainMenu::mapTexture = NULL;
sf::RenderTexture *MainMenu::pauseTexture = NULL;
sf::RenderTexture *MainMenu::saveTexture = NULL;
sf::RenderTexture *MainMenu::mapPreviewTexture = NULL;
sf::RenderTexture *MainMenu::mapPreviewThumbnailTexture = NULL;
sf::RenderTexture *MainMenu::brushPreviewTexture = NULL;
sf::RenderTexture *MainMenu::extraScreenTexture = NULL;
sf::RenderTexture *MainMenu::auraCheckTexture = NULL;

MainMenu * MainMenu::currInstance = NULL;

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
	ss << centiSecond;

	return ss.str();
}

void MainMenu::SetupTerrainShaders()
{
	if (terrainShaders != NULL)
	{
		return;
	}

	//global tileset
	ts_terrain = GetSizedTileset("Env/terrain_128x128.png");

	terrainShaders = new Shader[TerrainPolygon::TOTAL_TERRAIN_TYPES];

	for (int i = 0; i < TerrainPolygon::TOTAL_TERRAIN_TYPES; ++i)
	{
		SetupTerrainShader(terrainShaders[i], i);
	}
}

void MainMenu::SetupTerrainShader(sf::Shader &sh, int terrainIndex)
{
	if (!sh.loadFromFile("Resources/Shader/terrain.frag", sf::Shader::Fragment))
	{
		cout << "terrain shader not loading correctly" << endl;
		return;
	}

	sh.setUniform("u_texture", *ts_terrain->texture);
	sh.setUniform("Resolution", Vector2f(1920, 1080));
	//sh.setUniform("AmbientColor", ColorGL(Color::White)); //just not used currently
	sh.setUniform("skyColor", ColorGL(Color::White));

	float tilePattern[TerrainPolygon::TILE_PATTERN_TOTAL_INDEXES];
	Glsl::Vec4 tileQuads[TerrainPolygon::TOTAL_TILES_IN_USE];

	RandomPicker p;
	p.AddActiveOption(0, 2);
	p.AddActiveOption(1, 2);
	p.AddActiveOption(2, 2);
	p.AddActiveOption(3, 2);

	p.ShuffleActiveOptions();

	for (int i = 0; i < TerrainPolygon::TILE_PATTERN_TOTAL_INDEXES; ++i)
	{
		tilePattern[i] = p.AlwaysGetNextOption();//rand() % TOTAL_TILES_IN_USE;
	}

	int tile = terrainIndex * 4;//(8 * terrainWorldType + terrainVariation) * 4;
	IntRect ir;//rand() % 8);//tile + rand() % 2);

	float width = ts_terrain->texture->getSize().x;
	float height = ts_terrain->texture->getSize().y;


	sh.setUniformArray("u_patternGrid", tilePattern, TerrainPolygon::TILE_PATTERN_TOTAL_INDEXES);

	for (int i = 0; i < TerrainPolygon::TOTAL_TILES_IN_USE; ++i)
	{
		ir = ts_terrain->GetSubRect(tile + i);
		tileQuads[i] = Glsl::Vec4(ir.left / width, ir.top / height, (ir.left + ir.width) / width, (ir.top + ir.height) / height);
	}

	sh.setUniformArray("u_quadArray", tileQuads, TerrainPolygon::TOTAL_TILES_IN_USE);
}

void MainMenu::SetupWaterShaders()
{
	if (waterShaders != NULL)
	{
		return;
	}

	ts_water = GetSizedTileset("Env/water_128x128.png");
	waterShaders = new Shader[TerrainPolygon::WATER_Count];
	minimapWaterShaders = new Shader[TerrainPolygon::WATER_Count];

	for (int i = 0; i < TerrainPolygon::WATER_Count; ++i)
	{
		SetupWaterShader(waterShaders[i], i);
		SetupWaterShader(minimapWaterShaders[i], i);
		minimapWaterShaders[i].setUniform("zoom", Minimap::MINIMAP_ZOOM);
		minimapWaterShaders[i].setUniform("topLeft", Vector2f(0, 0));
	}
}

void MainMenu::SetupWaterShader(sf::Shader &waterShader, int waterIndex)
{
	if (!waterShader.loadFromFile("Resources/Shader/water.frag", sf::Shader::Fragment))
	{
		cout << "water SHADER NOT LOADING CORRECTLY" << endl;
		return;
	}

	waterShader.setUniform("u_slide", 0.f);
	waterShader.setUniform("u_texture", *ts_water->texture);
	waterShader.setUniform("Resolution", Vector2f(1920, 1080));
	//waterShader.setUniform("AmbientColor", Glsl::Vec4(1, 1, 1, 1));
	//waterShader.setUniform("skyColor", ColorGL(Color::White));

	Color wColor = TerrainPolygon::GetWaterColor(waterIndex);
	wColor.a = 200;
	waterShader.setUniform("u_waterBaseColor", ColorGL(wColor));

	IntRect ir1 = ts_water->GetSubRect(waterIndex * 2);
	IntRect ir2 = ts_water->GetSubRect(waterIndex * 2 + 1);

	float width = ts_water->texture->getSize().x;
	float height = ts_water->texture->getSize().y;

	waterShader.setUniform("u_quad1",
		Glsl::Vec4(ir1.left / width, ir1.top / height,
		(ir1.left + ir1.width) / width, (ir1.top + ir1.height) / height));

	waterShader.setUniform("u_quad2",
		Glsl::Vec4(ir2.left / width, ir2.top / height,
		(ir2.left + ir2.width) / width, (ir2.top + ir2.height) / height));
}

void MainMenu::LevelLoad(GameSession *gs)
{
	/*if (worldMap != NULL && saveMenu != NULL)
	{
		delete worldMap;
		worldMap = NULL;
		delete saveMenu;
		saveMenu = NULL;
	}*/
	
	adventureManager->DestroySaveMenu();
	adventureManager->DestroyWorldMap();


	GameSession::sLoad(gs);
}

void MainMenu::sLevelLoad(MainMenu *mm, GameSession *gs)
{
	mm->LevelLoad(gs);
}


void MainMenu::TransitionMode(Mode fromMode, Mode toMode)
{
	ControllerDualStateQueue *storedControllerStates = NULL;
	ControlProfile *storedControlProfile = NULL;
	int storedSkin = -1;


	switch (fromMode)
	{
	case SAVEMENU:
		/*assert(worldMap != NULL);
		delete worldMap;
		worldMap = NULL;

		assert(saveMenu != NULL);
		currSaveFile = NULL;
		delete saveMenu;
		saveMenu = NULL;*/
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
		adventureManager->worldMap->CurrSelector()->DestroyBGs();
		adventureManager->worldMap->CurrSelector()->FocusedSector()->DestroyMapPreview();
		break;
	}
	case WORLDMAP:
	{
		break;
	}
	case RUN_ADVENTURE_MAP:
	{
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
	case RUN_WORKSHOP_MAP:
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
	case SINGLE_PLAYER_CONTROLLER_JOIN_TUTORIAL:
	case SINGLE_PLAYER_CONTROLLER_JOIN_ADVENTURE:
	case SINGLE_PLAYER_CONTROLLER_JOIN_ONLINE:
	{
		if (singlePlayerControllerJoinScreen != NULL)
		{
			if (toMode == SAVEMENU)
			{
				storedControllerStates = singlePlayerControllerJoinScreen->playerBoxGroup->GetControllerStates(0);
				storedControlProfile = singlePlayerControllerJoinScreen->playerBoxGroup->GetControlProfile(0);
				storedSkin = singlePlayerControllerJoinScreen->playerBoxGroup->GetSkinIndex(0);
			}

			delete singlePlayerControllerJoinScreen;
			singlePlayerControllerJoinScreen = NULL;
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
			adventureManager->CreateWorldMap();
			adventureManager->CreateSaveMenu();

			adventureManager->worldMap->InitSelectors();
			adventureManager->worldMap->SetShipToColony(0);
			adventureManager->worldMap->state = WorldMap::PLANET;
			//assert(adventureManager->worldMap == NULL);
			//assert(adventureManager->saveMenu == NULL);
			//adventureManager->worldMap = new WorldMap(this);
			//adventureManager->saveMenu = new SaveMenuScreen(this);
			//adventureManager->saveMenu->Reset();
			//adventureManager->worldMap->state = WorldMap::PLANET;//WorldMap::PLANET_AND_SPACE;
			//adventureManager->worldMap->frame = 0;
			//adventureManager->worldMap->InitSelectors();
			//adventureManager->worldMap->SetDefaultSelections();
			//adventureManager->worldMap->UpdateWorldStats();
		}
		else if (fromMode == RUN_ADVENTURE_MAP)
		{
			adventureManager->CreateWorldMap();
			adventureManager->CreateSaveMenu();
			//assert(worldMap == NULL);
			//assert(saveMenu == NULL);
			//worldMap = new WorldMap(this);
			//saveMenu = new SaveMenuScreen(this);
			//saveMenu->Reset();
			//worldMap->state = WorldMap::PLANET;//WorldMap::PLANET_AND_SPACE;
			//worldMap->frame = 0;
			//worldMap->InitSelectors();
			//worldMap->SetDefaultSelections();
			//worldMap->UpdateWorldStats();
		}
		break;
	}
	case WORLDMAP_COLONY:
	{
		if (fromMode == WORLDMAP)
		{
			//going to worldmap_colony
			SaveFile *saveFile = adventureManager->currSaveFile;
			saveFile->mostRecentWorldSelected = adventureManager->worldMap->selectedColony;
			saveFile->Save();
			adventureManager->worldMap->CurrSelector()->CreateBGs();

			adventureManager->UpdateWorldDependentTileset(adventureManager->worldMap->CurrSelector()->world->index);

			if (adventureManager->worldMap->CurrSelector()->numSectors == 1)
			{
				adventureManager->worldMap->CurrSelector()->FocusedSector()->UpdateMapPreview();
			}
		}
		else if (fromMode == RUN_ADVENTURE_MAP)
		{


			adventureManager->CreateWorldMapOnCurrLevel();
			//adventureManager->CreateWorldMap();
			adventureManager->CreateSaveMenu();

			delete currLevel;
			currLevel = NULL;
			adventureManager->currLevel = NULL;

			musicPlayer->PlayMusic(menuMusic);

			//adventureManager->worldMap->SetToLevel( adventureManager->currLevel->)
			//adventureManager->worldMap->SetToColonyMode(0);
			/*worldMap->selectors[worldMap->selectedColony]->ReturnFromMap();
			SetMode(WORLDMAP_COLONY);
			worldMap->CurrSelector()->FocusedSector()->UpdateLevelStats();
			worldMap->CurrSelector()->FocusedSector()->UpdateStats();
			worldMap->CurrSelector()->FocusedSector()->UpdateMapPreview();

			worldMap->Update();

			musicPlayer->TransitionMusic(menuMusic, 60);*/
		}

	}
	case SAVEMENU:
	{
		if (fromMode == SINGLE_PLAYER_CONTROLLER_JOIN_ADVENTURE)
		{
			assert(adventureManager == NULL);
			adventureManager = new AdventureManager;
			adventureManager->controllerInput = storedControllerStates;
			adventureManager->currProfile = storedControlProfile;
			adventureManager->CreateWorldMap();
			adventureManager->CreateSaveMenu();

		}
		break;
	}
	case TITLEMENU:
	{
		if (fromMode == SAVEMENU)
		{
			assert(adventureManager != NULL);

			delete adventureManager;

			adventureManager = NULL;
		}
		else if (fromMode == RUN_ADVENTURE_MAP)
		{
			delete currLevel;
			currLevel = NULL;
			adventureManager->currLevel = NULL;

			delete adventureManager;
			adventureManager = NULL;
		}
		else if (fromMode == ADVENTURETUTORIAL)
		{
			assert(adventureManager != NULL);

			delete adventureManager;

			adventureManager = NULL;
		}
		else if (fromMode == POST_MATCH_OPTIONS)
		{
			if (adventureManager != NULL)
			{
				delete adventureManager;
				adventureManager = NULL;
			}
		}
		else if (fromMode == RUN_WORKSHOP_MAP)
		{
			delete workshopBrowser;
			workshopBrowser = NULL;
		}

		assert(titleScreen == NULL);
		titleScreen = new TitleScreen(this);
		titleScreen->Reset();
		break;
	}

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
	{
		assert(currTutorialSession == NULL);
		gameRunType = GameRunType::GRT_TUTORIAL;
		currTutorialSession = new GameSession(menuMatchParams);
		GameSession::sLoad(currTutorialSession);
		break;
	}
	case ADVENTURETUTORIAL:
	{
		assert(currTutorialSession == NULL);
		gameRunType = GameRunType::GRT_TUTORIAL;
		currTutorialSession = new GameSession(menuMatchParams);
		GameSession::sLoad(currTutorialSession);

		adventureManager->DestroyWorldMap();
		adventureManager->DestroySaveMenu();
		break;
	}
	case RUN_WORKSHOP_MAP:
	{
		assert(currFreePlaySession == NULL);
		//assert(freeplayScreen != NULL);

		/*if (fromMode == BROWSE_WORKSHOP)
		{
			delete workshopBrowser;
			workshopBrowser = NULL;
		}*/

		//MatchParams mp = freeplayScreen->GetMatchParams();
		gameRunType = GameRunType::GRT_FREEPLAY;
		currFreePlaySession = new GameSession(menuMatchParams);
		GameSession::sLoad(currFreePlaySession);
		break;
	}
	case RUN_FREEPLAY_MAP:
	{
		assert(currFreePlaySession == NULL);
		//assert(freeplayScreen != NULL);

		//MatchParams mp = freeplayScreen->GetMatchParams();
		gameRunType = GameRunType::GRT_FREEPLAY;
		currFreePlaySession = new GameSession(menuMatchParams);
		GameSession::sLoad(currFreePlaySession);
		break;
	}
	case FREEPLAY:
	{
		assert(freeplayScreen == NULL);
		freeplayScreen = new FreeplayScreen;
		freeplayScreen->Start();

		if (fromMode == BROWSE_WORKSHOP)
		{
			workshopBrowser->ClearAllPreviewsButSelected();
		}
		else if (fromMode == RUN_FREEPLAY_MAP)
		{
			freeplayScreen->SetFromMatchParams(*menuMatchParams);
		}

		break;
	}
	case SINGLE_PLAYER_CONTROLLER_JOIN_ADVENTURE:
	case SINGLE_PLAYER_CONTROLLER_JOIN_TUTORIAL:
	case SINGLE_PLAYER_CONTROLLER_JOIN_ONLINE:
	{
		assert(singlePlayerControllerJoinScreen == NULL);
		singlePlayerControllerJoinScreen = new SinglePlayerControllerJoinScreen(this);
		singlePlayerControllerJoinScreen->Start();

		if (toMode == SINGLE_PLAYER_CONTROLLER_JOIN_ADVENTURE || toMode == SINGLE_PLAYER_CONTROLLER_JOIN_ONLINE)
		{
			singlePlayerControllerJoinScreen->SetMode(PlayerBox::MODE_CONTROLLER_ONLY);
		}
		break;
	}
	case RUN_ADVENTURE_MAP:
	{
		//gets loaded with an adventure loading screen, so doesn't ever use transitionmode

		//adventureManager->DestroySaveMenu();
		//adventureManager->DestroyWorldMap();
		/*assert(worldMap != NULL && saveMenu != NULL);

		delete worldMap;
		worldMap = NULL;

		delete saveMenu;
		saveMenu = NULL;*/
		break;
	}
	}
}




void MainMenu::sTransitionMode(MainMenu *mm, Mode fromMode, Mode toMode )
{
	mm->TransitionMode(fromMode, toMode);
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

MainMenu::MainMenu( bool p_steamOn)
	:windowWidth(1920), windowHeight(1080)
{
	assert(currInstance == NULL);
	currInstance = this;

	steamOn = p_steamOn;

	//steamOn = false;

	adventureManager = NULL;

	customCursor = NULL;
	window = NULL;

	ControllerSettings::InitStrings();

	selectorAnimFrame = 0;
	selectorAnimDuration = 21;
	selectorAnimFactor = 3;
	
	menuMatchParams = new MatchParams;

	if (steamOn)
	{
		remoteStorageManager = new RemoteStorageManager;
	}
	else
	{
		remoteStorageManager = NULL;
	}

	kinHitboxManager = new HitboxManager("Kin/Hitboxes");

	CreatePlayerTilesets();

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
	singlePlayerControllerJoinScreen = NULL;

	arial.loadFromFile("Resources/Fonts/Kinetic_Font_01.ttf");
	consolas.loadFromFile("Resources/Fonts/Courier New.ttf");

	//player shaders
	RegisterShader("colorswap");
	RegisterShader("playerdesperation");
	RegisterShader("playersuper");
	RegisterShader("motionghost");
	RegisterShader("shield");
	RegisterShader("wire");

	//enemy shaders
	RegisterShader("enemyhurt");
	RegisterShader("enemykey");

	


	transLength = 60;
	transFrame = 0;

	config = new Config();
	//Config::CreateLoadThread(config);
	//config->WaitForLoad();
	config->Load();
	//config->Load();
	//config->WaitForLoad();
	windowWidth = config->GetData().resolutionX;
	windowHeight = config->GetData().resolutionY;
	style = config->GetData().windowStyle;

	CONTROLLERS.CheckForControllers();
	

	//CONTROLLERS.Update();

	cpm = new ControlProfileManager;
	cpm->LoadProfiles();

	//MusicManager mm(this);
	musicManager = new MusicManager(this);
	musicManager->LoadMusicNames();

	messagePopup = new MessagePopup;

	deadThread = NULL;
	loadThread = NULL;
	
	ts_mainOption = GetSizedTileset("Menu/Title/mainmenu_text_512x64.png");
	ts_menuSelector = GetSizedTileset("Menu/menu_selector_64x64.png");

	ts_buttonIcons = GetSizedTileset("Menu/button_icon_128x128.png");
	ts_keyboardIcons = GetSizedTileset("Menu/keyboard_icons_64x64.png");
	ts_thanksForPlaying = NULL;

	selectorSprite.setTexture(*ts_menuSelector->texture);

	activatedMainMenuOptions[0] = true;		//adventure
	activatedMainMenuOptions[1] = true;		//freeplay
	activatedMainMenuOptions[2] = steamOn;//false;	//local multiplayer
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


	

	musicPlayer = new MusicPlayer(this);

	//clean this up for when someone doesn't have the music maybe? shouldn't crash at least.
	menuMusic = musicManager->GetMusicInfo("w0_2_Breakneck_Menu_01");
	if (menuMusic != NULL)
	{
		menuMusic->Load();
	}
	
	
	//cout << "DDDDDD " << endl;


	//this is turned on when starting at the titlescreen
	titleScreen = NULL;
	//titleScreen = new TitleScreen(this);
	closedBetaScreen = new ClosedBetaScreen;




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

	soundNodeList = new SoundNodeList( 20 );
	soundNodeList->SetSoundVolume(config->GetData().soundVolume);
	//soundNodeList->SetGlobalVolume(100);//config->GetData().soundVolume );


	CreateRenderTextures();

	transWorldMapFrame = 0;
	
	if (titleScreen != NULL)
	{
		titleScreen->Draw(preScreenTexture);
	}
	
	waterShaders = NULL;
	minimapWaterShaders = NULL;

	terrainShaders = NULL;

	SetupWaterShaders();
	SetupTerrainShaders();

	adventureManager = NULL;
	//worldMap = new WorldMap( this );

	

	levelSelector = new LevelSelector( this );
	
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

	gameSettingsScreen = new GameSettingsScreen(this);

	mapBrowserScreen = new MapBrowserScreen(this);

	workshopBrowser = NULL;

	onlineMenuScreen = new OnlineMenuScreen(this);

	editorMenuScreen = new EditorMenuScreen(this);

	matchResultsScreen = NULL;

	loadingBackpack = new LoadingBackpack( this );

	CONTROLLERS.Update();
}

void MainMenu::SetupWindow()
{
	if (window != NULL)
	{
		window->close();
		delete window;
		window = NULL;
	}
	
	windowWidth = config->GetData().resolutionX;
	windowHeight = config->GetData().resolutionY;
	style = config->GetData().windowStyle;

	/*windowWidth = 960;
	windowHeight = 540;*/
	window = new RenderWindow(sf::VideoMode(windowWidth, windowHeight), "Kinetic: Break All Limits",
		config->GetData().windowStyle, sf::ContextSettings(0, 0, 0, 0, 0));
	window->setKeyRepeatEnabled(false);

	MOUSE.SetRenderWindow(window);
	UICONTROLLER.SetRenderWindow(window);

	CONTROLLERS.SetRenderWindow(window);

	if (customCursor == NULL)
	{
		customCursor = new CustomCursor;
	}
	
	customCursor->Init(window);

	MOUSE.SetCustomCursor(customCursor);

	//window->setMouseCursorVisible(false);

	//customCursor->Grab();

	assert(window != NULL);
	window->setVerticalSyncEnabled(true);

	auto vMode = VideoMode::getDesktopMode();

	Vector2i border((vMode.width - windowWidth) / 2, (vMode.height - windowHeight) / 2 );

	window->setPosition(border);

	Session *sess = Session::GetSession();
	if (sess != NULL)
	{
		sess->window = window;

		View vTest;
		vTest.setCenter(0, 0);
		vTest.setSize(1920 / 2, 1080 / 2);
		window->setView(vTest);

		//simulates what is done within gamesession. fix this later and make it cleaner
	}
	else
	{
		window->setView(v);
	}
	//window->setFramerateLimit(120);
	//std::cout << "opened window" << endl;
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
		pauseTexture->create(1920, 1080);//1820, 980);
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
		mapPreviewTexture->create(912, 492);
		mapPreviewTexture->clear();
	}

	if (mapPreviewThumbnailTexture == NULL)
	{
		mapPreviewThumbnailTexture = new RenderTexture;
		mapPreviewThumbnailTexture->create(228, 123); // 1/4 the size of the original for now
		mapPreviewThumbnailTexture->clear();
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

MainMenu::~MainMenu()
{
	assert(currInstance == this);
	currInstance = NULL;

	if (kinHitboxManager != NULL)
	{
		delete kinHitboxManager;
		kinHitboxManager = NULL;
	}

	if (waterShaders != NULL)
	{
		delete[] waterShaders;
		waterShaders = NULL;
	}

	if (terrainShaders != NULL)
	{
		delete[] terrainShaders;
		terrainShaders = NULL;
	}

	if (minimapWaterShaders != NULL)
	{
		delete[] minimapWaterShaders;
		minimapWaterShaders = NULL;
	}


	window->close();

	if (remoteStorageManager != NULL)
	{
		delete remoteStorageManager;
	}

	delete closedBetaScreen;

	delete menuMatchParams;

	delete customCursor;

	delete loadingBackpack;
	delete cpm;
	delete musicManager;

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

	if (adventureManager != NULL)
	{
		delete adventureManager;
	}

	delete messagePopup;

	delete config;

	delete musicPlayer;

	if (titleScreen != NULL)
	{
		delete titleScreen;
	}
	
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
	delete mapPreviewThumbnailTexture;
	delete auraCheckTexture;
	delete brushPreviewTexture;

	
	delete levelSelector;
	delete window;
	
	delete gameSettingsScreen;
	delete mapBrowserScreen;
	delete onlineMenuScreen;
	delete editorMenuScreen;

	if (workshopBrowser != NULL)
	{
		delete workshopBrowser;
	}
	
	delete fader;
	delete swiper;
	delete indEffectPool;

	delete globalFile;
}

//singleton
MainMenu *MainMenu::GetInstance()
{
	return currInstance;
}

void MainMenu::Init()
{	
	ts_buttonIcons = GetSizedTileset("Menu/button_icon_128x128.png");

	ts_splashScreen = GetTileset( "Menu/splashscreen_1920x1080.png", 1920, 1080 );
	splashSprite.setTexture( *ts_splashScreen->texture );

	fader = new Fader;
	swiper = new Swiper();

	Swiper::LoadSwipeType( this, Swiper::W1);

	//FeedbackManager::SubmitFeedback("memory test", "body test");
	
	indEffectPool = new EffectPool(EffectType::FX_IND, 4);
	indEffectPool->Reset();


	soundInfos[S_DOWN] = soundManager.GetSound( "menu_down" );
	soundInfos[S_UP] = soundManager.GetSound( "menu_up" );
	soundInfos[S_SELECT] = soundManager.GetSound( "menu_select" );

	ts_loadBG = NULL;

	cout << "init finished" << endl;
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
	if (menuMode == TITLEMENU || menuMode == TRANS_MAIN_TO_SAVE )//|| menuMode == WORLDMAP)
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
		adventureManager->kinBoostScreen->Reset();
	}

	if (menuMode == TITLEMENU)
	{
		selectorAnimFrame = 0;
		MOUSE.SetControllersOn(true);
		MOUSE.Show();
		customCursor->SetMode(CustomCursor::M_REGULAR);
		customCursor->Show();
	}

	if (menuMode == ONLINE_MENU)
	{
		MOUSE.Show();
		onlineMenuScreen->Start();
	}
	else if (menuMode == EDITOR_MENU)
	{
		editorMenuScreen->Start();
	}
	else if (menuMode == NETPLAY_MATCH_RESULTS)
	{
		matchResultsScreen->Reset();
	}
	else if (menuMode == POST_MATCH_OPTIONS)
	{
		customMatchManager->OpenPostMatchPopup();
	}
	else if (menuMode == SAVEMENU)
	{
		if (oldMode != WORLDMAP)
		{
			musicPlayer->PlayMusic(menuMusic);
		}

		if (adventureManager->currSaveFile == NULL)
		{
			adventureManager->saveMenu->SetSkin(0);
		}
		else
		{
			adventureManager->saveMenu->SetSkin(adventureManager->currSaveFile->defaultSkinIndex);
		}
	}
	else if (menuMode == BROWSE_WORKSHOP)
	{
		MOUSE.Show();
		//SetMouseGrabbed(true);
		//SetMouseVisible(true);
	}

	if (menuMode == WORLDMAP_COLONY)
	{
		adventureManager->worldMap->state = WorldMap::COLONY;
		adventureManager->worldMap->frame = 0;
	}
	if (menuMode == WORLDMAP && adventureManager->worldMap->state == WorldMap::COLONY)
	{
		soundNodeList->ActivateSound(soundManager.GetSound("world_zoom_out"));
		adventureManager->worldMap->state = WorldMap::COLONY_TO_PLANET;
		adventureManager->worldMap->frame = 0;
		adventureManager->worldMap->UpdateWorldStats();
	}
	if (menuMode == WORLDMAP && oldMode == SAVEMENU)
	{
		adventureManager->worldMap->state = WorldMap::PLANET_VISUAL_ONLY;
		adventureManager->worldMap->frame = 0;
	}
	if (menuMode == QUICKPLAY_TEST)
	{
		MOUSE.Hide();
	}
	else if (menuMode == POST_MATCH_OPTIONS)
	{
		MOUSE.Show();
	}
	else if (menuMode == CREDITS)
	{
		titleScreen->creditsMenuScreen->Start();
	}
	else if (menuMode == RUN_WORKSHOP_MAP)
	{
		MOUSE.Hide();
	}

	if (menuMode == THANKS_FOR_PLAYING)
	{
		ts_thanksForPlaying = GetTileset("Story/Ship_04.png", 1920, 1080);
		ts_thanksForPlaying->SetQuadSubRect(thanksQuad, 0);
		SetRectTopLeft(thanksQuad, 1920, 1080, Vector2f(0, 0));
	}
	else if (oldMode == THANKS_FOR_PLAYING)
	{
		DestroyTileset(ts_thanksForPlaying);
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
	namePopup.AddBasicTextBox( "name", Vector2i( 10, 10 ), 300, 40, "test" );

	

	Panel downloadPopup("remove popup", 500, 200, &customMapHandler);
	downloadPopup.pos = Vector2i(960 - 250, 540);
	downloadPopup.AddButton("downloadok", Vector2i(300, 0), Vector2f(100, 50), "OK");
	downloadPopup.AddBasicTextBox("index", Vector2i(10, 10), 300, 40, "0");

	Panel loginPopup("login popup", 800, 200, &customMapHandler);
	loginPopup.pos = Vector2i(960 - 400, 540);
	loginPopup.AddButton("loginok", Vector2i(630, 0), Vector2f(100, 50), "OK");
	loginPopup.AddBasicTextBox("pass", Vector2i(10, 10), 300, 40, "");
	loginPopup.AddBasicTextBox("user", Vector2i(320, 10), 300, 40, "");

	Panel removePopup("remove popup", 500, 200, &customMapHandler);
	removePopup.pos = Vector2i(960 - 250, 540);
	removePopup.AddButton("removeok", Vector2i(300, 0), Vector2f(100, 50), "OK");
	removePopup.AddBasicTextBox("index", Vector2i(10, 10), 300, 40, "0");
	//Panel loginPopup( "login popup")
	//bool showNamePopup = false;

	ls.UpdateMapList();

	ls.LoadAndRewriteAllMaps();
	//ls.LoadAndRewriteAllMaps();

	//empty map. fix this later
	string empty = "5\n0 0\nmat\n5\n-209 78\n286 78\n286 132\n60 132\n-201 132\n0\n0\n0\n1\n-- 1\ngoal -air 0 0 76";

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
		MOUSE.Update(mousePos);//GetPixelPos());
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

	window->setView(oldView);
}

void MainMenu::GGPOOption()
{
	EditSession *edit = new EditSession(this, "Resources\\Maps\\W2\\gateblank91" + string(MAP_EXT));
	edit->Run();
	delete edit;
}

sf::IntRect MainMenu::GetKeyboardKeyIconTile( Keyboard::Key key )
{
	int buttonIndex = key;//0;//controllerInput->con->keySettings.buttonMap[button];
	if (buttonIndex == Keyboard::Unknown)
	{
		buttonIndex = 75; //blank tile for now
	}

	Tileset *ts = GetButtonIconTileset(ControllerType::CTYPE_KEYBOARD);

	return ts->GetSubRect(buttonIndex);
}

sf::IntRect MainMenu::GetButtonIconTile(ControllerSettings::ButtonType button, ControlProfile *profile)
{
	/*CTYPE_XBOX,
		CTYPE_GAMECUBE,
		CTYPE_PS4,
		CTYPE_PS5,
		CTYPE_KEYBOARD,
		CTYPE_NONE,*/

	if (profile == NULL)
	{
		return IntRect();
	}

	int buttonIndex = 0;

	int cType = profile->GetControllerType();

	switch (cType)
	{
	case CTYPE_XBOX:
	{
		if (button >= ControllerSettings::BUTTONTYPE_LLEFT && button <= ControllerSettings::BUTTONTYPE_RDOWN )
		{
			int diff = button - ControllerSettings::BUTTONTYPE_LLEFT;

			buttonIndex = XBOX_LLEFT + diff;
		}
		else
		{
			buttonIndex = profile->Filter(button);//0;//(controllerInput->con->filter[button]);// -1);

			if (buttonIndex == XBOX_BLANK)
			{
				return IntRect(0, 0, 0, 0);
			}
		}
		break;
	}
	case CTYPE_GAMECUBE:
	{
		if (button >= ControllerSettings::BUTTONTYPE_LLEFT && button <= ControllerSettings::BUTTONTYPE_RDOWN)
		{
			int diff = button - ControllerSettings::BUTTONTYPE_LLEFT;

			buttonIndex = (XBOX_LLEFT + diff) + 16 * 2;
		}
		else
		{
			buttonIndex = profile->Filter(button) + 16 * 2;//0;//(controllerInput->con->filter[button]) + 16 * 2;// - 1) + 16 * 2;

			if (buttonIndex == XBOX_BLANK)
			{
				return IntRect(0, 0, 0, 0);
			}
		}
		
		break;
	}
	case CTYPE_PS4:
	case CTYPE_PS5:
	{
		//buttonIndex + 16
		break;
	}
	case CTYPE_KEYBOARD:
	{
		buttonIndex = profile->Filter(button);//0;//controllerInput->con->keySettings.buttonMap[button];
		if (buttonIndex == Keyboard::Unknown)
		{
			buttonIndex = 75; //blank tile for now
		}

		//if (baseButtonIndex < 12 * 6)
		//{
		//	//clean this up more later when alex updates the image
		//	return mainMenu->ts_keyboardIcons->GetSubRect(baseButtonIndex);
		//}
		//else
		//{
		//	return mainMenu->ts_keyboardIcons->GetSubRect(0);
		//}
	}
	}

	Tileset *ts = GetButtonIconTileset(cType);

	return ts->GetSubRect(buttonIndex);
}

sf::IntRect MainMenu::GetButtonIconTileForMenu(int controllerType, XBoxButton button)
{
	/*CTYPE_XBOX,
	CTYPE_GAMECUBE,
	CTYPE_PS4,
	CTYPE_PS5,
	CTYPE_KEYBOARD,
	CTYPE_NONE,*/



	int buttonIndex = 0;

	switch (controllerType)
	{
	case CTYPE_XBOX:
	{
		buttonIndex = button;//(controllerInput->[button]);// -1);
		break;
	}
	case CTYPE_GAMECUBE:
	{
		buttonIndex = button + 16 * 2;//(controllerInput->con->filter[button]) + 16 * 2;// - 1) + 16 * 2;
		break;
	}
	case CTYPE_PS4:
	case CTYPE_PS5:
	{
		//buttonIndex + 16
		break;
	}
	case CTYPE_KEYBOARD:
	{
		buttonIndex = CONTROLLERS.GetMenuKeyFromControllerButton(button);//controllerInput->con->keySettings.buttonMap[button];


		//if (baseButtonIndex < 12 * 6)
		//{
		//	//clean this up more later when alex updates the image
		//	return mainMenu->ts_keyboardIcons->GetSubRect(baseButtonIndex);
		//}
		//else
		//{
		//	return mainMenu->ts_keyboardIcons->GetSubRect(0);
		//}
	}
	}

	Tileset *ts = GetButtonIconTileset(controllerType);

	return ts->GetSubRect(buttonIndex);
}

Tileset * MainMenu::GetButtonIconTileset(int controllerType )
{
	//ControllerType controllerType = CONTROLLERS.GetWindowsController(controllerIndex)->GetCType();//GetController(controllerIndex)->GetCType();

	if (controllerType == CTYPE_KEYBOARD)
	{
		return ts_keyboardIcons;
	}
	else
	{
		return ts_buttonIcons;
	}
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
				path from("Resources/Maps/empty" + string(MAP_EXT));

				std::stringstream ssPath;
				ssPath << ls.localPaths[ls.selectedIndex] << ls.newLevelName << MAP_EXT;
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
				ssPath << "Resources/" << toNode->GetLocalPath() << ls.newLevelName << MAP_EXT;
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


#include <sfeMovie/Movie.hpp>

void MainMenu::CreatePlayerTilesets()
{
	//Tileset *ts_borders = 
	//ts_water = GetSizedTileset("Env/water_128x128.png");
	//ts_terrain = GetSizedTileset("Env/terrain_128x128.png");
	Tileset *ts_waterSurface = GetSizedTileset("Env/water_surface_64x2.png");
	Tileset *ts_grass = GetSizedTileset("Env/grass_128x128.png");

	Actor a;

	//create FX Tilesets
	string folderFX = "Kin/FX/";
	playerTilesetMap[PTS_FX_FAIR_SWORD_LIGHTNING_0] = GetSizedTileset(folderFX, "fair_sword_lightning_256x256.png");
	playerTilesetMap[PTS_FX_FAIR_SWORD_LIGHTNING_1] = GetSizedTileset(folderFX, "fair_sword_lightning_256x256.png");
	playerTilesetMap[PTS_FX_FAIR_SWORD_LIGHTNING_2] = GetSizedTileset(folderFX, "fair_sword_lightning_256x256.png");
	playerTilesetMap[PTS_FX_DAIR_SWORD_LIGHTNING_0] = GetSizedTileset(folderFX, "dair_sword_lightning_256x256.png");
	playerTilesetMap[PTS_FX_DAIR_SWORD_LIGHTNING_1] = GetSizedTileset(folderFX, "dair_sword_lightning_256x256.png");
	playerTilesetMap[PTS_FX_DAIR_SWORD_LIGHTNING_2] = GetSizedTileset(folderFX, "dair_sword_lightning_256x256.png");
	playerTilesetMap[PTS_FX_UAIR_SWORD_LIGHTNING_0] = GetSizedTileset(folderFX, "uair_sword_lightning_256x256.png");
	playerTilesetMap[PTS_FX_UAIR_SWORD_LIGHTNING_1] = GetSizedTileset(folderFX, "uair_sword_lightning_256x256.png");
	playerTilesetMap[PTS_FX_UAIR_SWORD_LIGHTNING_2] = GetSizedTileset(folderFX, "uair_sword_lightning_256x256.png");
	playerTilesetMap[PTS_FX_BOUNCE_BOOST] = GetSizedTileset(folderFX, "bounceboost_256x192.png");
	playerTilesetMap[PTS_FX_HURT_SPACK] = GetSizedTileset(folderFX, "fx_hurt_spack_128x160.png");
	playerTilesetMap[PTS_FX_DASH_START] = GetSizedTileset(folderFX, "fx_dash_start_160x160.png");
	playerTilesetMap[PTS_FX_DASH_REPEAT] = GetSizedTileset(folderFX, "fx_dash_repeat_192x128.png");
	playerTilesetMap[PTS_FX_LAND_0] = GetSizedTileset(folderFX, "fx_land_a_128x128.png");
	playerTilesetMap[PTS_FX_LAND_1] = GetSizedTileset(folderFX, "fx_land_b_192x208.png");
	playerTilesetMap[PTS_FX_LAND_2] = GetSizedTileset(folderFX, "fx_land_c_224x224.png");
	playerTilesetMap[PTS_FX_RUN_START] = GetSizedTileset(folderFX, "fx_runstart_128x128.png");
	playerTilesetMap[PTS_FX_SPRINT_0] = GetSizedTileset(folderFX, "fx_sprint_a_192x192.png");
	playerTilesetMap[PTS_FX_SPRINT_1] = GetSizedTileset(folderFX, "fx_sprint_b_320x320.png");
	playerTilesetMap[PTS_FX_SPRINT_2] = GetSizedTileset(folderFX, "fx_sprint_c_320x320.png");
	playerTilesetMap[PTS_FX_RUN] = GetSizedTileset(folderFX, "fx_run_144x128.png");
	playerTilesetMap[PTS_FX_JUMP_0] = GetSizedTileset(folderFX, "fx_jump_a_128x80.png");
	playerTilesetMap[PTS_FX_JUMP_1] = GetSizedTileset(folderFX, "fx_jump_b_160x192.png");
	playerTilesetMap[PTS_FX_JUMP_2] = GetSizedTileset(folderFX, "fx_jump_c_160x192.png");
	playerTilesetMap[PTS_FX_WALLJUMP_0] = GetSizedTileset(folderFX, "fx_walljump_a_160x160.png");
	playerTilesetMap[PTS_FX_WALLJUMP_1] = GetSizedTileset(folderFX, "fx_walljump_b_224x224.png");
	playerTilesetMap[PTS_FX_WALLJUMP_2] = GetSizedTileset(folderFX, "fx_walljump_c_224x224.png");
	playerTilesetMap[PTS_FX_DOUBLE] = GetSizedTileset(folderFX, "fx_double_256x128.png");
	playerTilesetMap[PTS_FX_GRAV_REVERSE] = GetSizedTileset(folderFX, "fx_grav_reverse_128x128.png");
	playerTilesetMap[PTS_FX_SPEED_LEVEL_CHARGE] = GetSizedTileset(folderFX, "fx_elec_128x128.png");
	playerTilesetMap[PTS_FX_RIGHT_WIRE_BOOST] = GetSizedTileset(folderFX, "wire_boost_r_64x64.png");
	playerTilesetMap[PTS_FX_LEFT_WIRE_BOOST] = GetSizedTileset(folderFX, "wire_boost_b_64x64.png");
	playerTilesetMap[PTS_FX_DOUBLE_WIRE_BOOST] = GetSizedTileset(folderFX, "wire_boost_m_64x64.png");
	playerTilesetMap[PTS_FX_AIRDASH_DIAGONAL] = GetSizedTileset(folderFX, "fx_airdash_diag_128x160.png");
	playerTilesetMap[PTS_FX_AIRDASH_UP] = GetSizedTileset(folderFX, "fx_airdash_128x128.png");
	playerTilesetMap[PTS_FX_AIRDASH_HOVER] = GetSizedTileset(folderFX, "fx_airdash_hold_96x80.png");
	playerTilesetMap[PTS_FX_GATE_ENTER] = GetSizedTileset(folderFX, "fx_gate_enter_160x128.png");
	playerTilesetMap[PTS_FX_SMALL_LIGHTNING] = GetSizedTileset(folderFX, "fx_elec_128x96.png");
	playerTilesetMap[PTS_FX_GATE_BLACK] = GetSizedTileset(folderFX, "keydrain_160x160.png");
	playerTilesetMap[PTS_FX_DASH_BOOST] = GetSizedTileset(folderFX, "fx_dash_boost_128x256.png");
	playerTilesetMap[PTS_FX_SPRINT_STAR] = GetSizedTileset(folderFX, "fx_sprint_star_01_64x64.png");
	playerTilesetMap[PTS_FX_LAUNCH_PARTICLE_0] = GetSizedTileset(folderFX, "launch_fx_192x128.png");
	playerTilesetMap[PTS_FX_LAUNCH_PARTICLE_1] = GetSizedTileset(folderFX, "launch_fx_192x128.png");
	playerTilesetMap[PTS_FX_ENTER] = GetSizedTileset(folderFX, "fx_enter_256x256.png");
	playerTilesetMap[PTS_FX_EXITENERGY_0] = GetSizedTileset(folderFX, "exitenergy_0_512x512.png");
	playerTilesetMap[PTS_FX_EXITENERGY_1] = GetSizedTileset(folderFX, "exitenergy_1_512x512.png");
	playerTilesetMap[PTS_FX_EXITENERGY_2] = GetSizedTileset(folderFX, "exitenergy_2_512x512.png");
}

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
	/*RemoteStorageManager rsm;
	rsm.LoadAll();
	rsm.Test();*/


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
	//SetMode(SPLASH);
	//menuMode = TITLEMENU;
	menuMode = CLOSED_BETA;

	
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
				CONTROLLERS.Update();

				mousePixelPos = GetPixelPos();//sf::Mouse::getPosition(*window);

				MOUSE.Update(mousePixelPos);

				UICONTROLLER.Update();

				if (CONTROLLERS.AltF4())
				{
					SetMode(EXITING);
					quit = true;
				}

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

	window->create(sf::VideoMode(windowWidth, windowHeight), "Breakneck",
		config->GetData().windowStyle, sf::ContextSettings());
	window->setPosition(Vector2i(0, 0));
	style = p_style;
	return;

	//if (style != p_style)
	//{
	//	//sf::ContextSettings contextSettings( 0, 0, 0, 0)

	//	window->create( sf::VideoMode(windowWidth, windowHeight), "Breakneck",
	//		config->GetData().windowStyle, sf::ContextSettings());
	//	style = p_style;
	//}
	//else
	//{
	//	window->setSize(Vector2u(windowWidth, windowHeight));
	//	window->setPosition(Vector2i(0, 0));
	//}
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
		DestroyTileset(ts_loadBG);
		ts_loadBG = NULL;
	}

	ts_loadBG = GetTileset(ss.str(), 1920, 1080);

	loadingBackpack->SetScale(1.f);
	loadingBackpack->SetPosition(Vector2f(1920 - 260, 1080 - 200));
	loadingBGSpr.setTexture(*ts_loadBG->texture);
}

void MainMenu::SetModeKinBoostLoadingMap(int variation)
{
	//fader->Clear();
	//fader->
	//swiper->Swipe(Swiper::W1, 15, true);
	gameRunType = GRT_ADVENTURE;
	SetMode(KINBOOSTLOADINGMAP);
	//preScreenTexture->setView(v);
	//wIndex = min(wIndex, 1); //because there are only screens for 2 worlds
	//loadingBGSpr.setTexture(*ts_loadBG[wIndex]->texture);


}

void MainMenu::AdventureLoadLevel(LevelLoadParams &loadParams)
{
	string levelPath = loadParams.adventureMap->GetMapPath();//lev->GetFullName();// name;

	doneLoading = false;

	int wIndex = loadParams.world;
	gameRunType = GameRunType::GRT_ADVENTURE;
	SetModeAdventureLoadingMap(wIndex);

	if (adventureManager->leaderboard->IsHidden())
	{
		adventureManager->leaderboard->Reset();
	}

	MatchParams mp;
	mp.saveFile = adventureManager->currSaveFile;
	mp.mapPath = levelPath;
	mp.controllerStateVec[0] = adventureManager->controllerInput;
	mp.controlProfiles[0] = adventureManager->currProfile;
	mp.playerSkins[0] = adventureManager->currSaveFile->defaultSkinIndex;
	
	//do this when using practice mode!
	if (adventureManager->parallelPracticeMode)
	{
		mp.gameModeType = MatchParams::GAME_MODE_PARALLEL_PRACTICE;
		mp.netplayManager = netplayManager;
		mp.numPlayers = 2; //me plus 1 other person for now
		netplayManager->FindPracticeMatch(levelPath, loadParams.level->index );
	}

	currLevel = new GameSession(&mp);
	currLevel->SetBestGhostOn(loadParams.bestTimeGhostOn);
	currLevel->SetBestReplayOn(loadParams.bestReplayOn);
	currLevel->level = loadParams.level;

	/*if (adventureManager->leaderboard->IsTryingToStartReplay())
	{
		currLevel->TryStartLeaderboardReplay(adventureManager->leaderboard->replayChosen);
		adventureManager->leaderboard->Hide();
	}*/

	

	adventureManager->currLevel = currLevel;

	loadThread = new boost::thread(MainMenu::sLevelLoad, this, currLevel);

	accumulator = 0;//TIMESTEP + .1;
	currentTime = 0;
	gameClock.restart();
}

void MainMenu::PlayIntroMovie()
{
	//you'll get crashes when you add this because it doesn't use the controller stuff correctly etc
	//just double check/redo stuff when you add the movie
	return;

	adventureManager->worldMap->SetToLevel(0, 0, 0 );
	//worldMap->testSelector->UpdateAllInfo();

	SetMode(INTROMOVIE);
	introMovie->Play();

	//musicPlayer->FadeOutCurrentMusic(30);
	MusicInfo *info = musicManager->songMap["w0_0_Film"];
	musicPlayer->TransitionMusic(info, 60, sf::seconds( 60 ));
	
	Level *lev = &(adventureManager->adventurePlanet->worlds[0].sectors[0].levels[0]);
	AdventureMap &am = adventureManager->adventureFile.GetMap(lev->index);
	string levelPath = am.GetMapPath();//lev->GetFullName();
	//window->setActive(false);
	doneLoading = false;

	//int wIndex = lev->sec->world->index;
	gameRunType = GameRunType::GRT_ADVENTURE;
	//SetModeLoadingMap(wIndex);


	MatchParams mp;
	mp.saveFile = adventureManager->currSaveFile;//adventureManager->files[adventureManager->curr];
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
	GameSession *old = m->currLevel;
	delete old;

	m->GoToNextLevel(am, lev);
}

void MainMenu::GoToNextLevel(AdventureMap *am,
	Level *lev)
{
	string levelPath = am->GetMapPath();

	adventureManager->leaderboard->Reset();

	MatchParams mp;
	mp.saveFile = adventureManager->currSaveFile;
	mp.mapPath = levelPath;
	mp.controllerStateVec[0] = adventureManager->controllerInput;
	mp.controlProfiles[0] = adventureManager->currProfile;
	mp.playerSkins[0] = adventureManager->currSaveFile->defaultSkinIndex;

	//do this when using practice mode!
	if (adventureManager->parallelPracticeMode)
	{
		mp.gameModeType = MatchParams::GAME_MODE_PARALLEL_PRACTICE;
		mp.netplayManager = netplayManager;
		mp.numPlayers = 2; //me plus 1 other person for now
		netplayManager->FindPracticeMatch(levelPath, lev->index);
	}

	currLevel = new GameSession(&mp);
	currLevel->level = lev;
	currLevel->boostEntrance = true;

	adventureManager->currLevel = currLevel;

	GameSession::sLoad(currLevel);
}

void MainMenu::UpdateMenuMode()
{
	(this->*updateModeFuncs[menuMode])();
}

void MainMenu::ReturnToWorldAfterLevel()
{
	//SingleAxisSelector *sa = worldMap->selectors[worldMap->selectedColony]->FocusedSector()->mapSASelector;
	//int numLevels = worldMap->GetCurrSectorNumLevels();//worldMap->selectors[worldMap->selectedColony]->sectors[secIndex]->numLevels;

	musicPlayer->StopCurrentMusic();
	LoadMode(WORLDMAP_COLONY);
	//int numWorlds = worldMap->adventurePlanet->numWorlds;
	//if (worldMap->selectedColony == numWorlds - 1)
	//{
	//	if (sa->currIndex == numLevels - 1)
	//	{
	//		if (currSaveFile->GetNumCompleteWorlds(worldMap->adventurePlanet) == numWorlds)
	//		{
	//			SetMode(THANKS_FOR_PLAYING);
	//			//return;
	//		}
	//	}
	//}

	/*if (menuMode != THANKS_FOR_PLAYING)
	{
		worldMap->selectors[worldMap->selectedColony]->ReturnFromMap();
		SetMode(WORLDMAP_COLONY);
		worldMap->CurrSelector()->FocusedSector()->UpdateLevelStats();
		worldMap->CurrSelector()->FocusedSector()->UpdateStats();
		worldMap->CurrSelector()->FocusedSector()->UpdateMapPreview();

		worldMap->Update();

		musicPlayer->TransitionMusic(menuMusic, 60);
	}*/
	
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
		
		if (CONTROLLERS.ButtonPressed_Any() )
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
		messagePopup->Update();
		if (messagePopup->action == MessagePopup::A_INACTIVE )
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
			adventureManager->worldMap->HandleEvent(ev);

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

		adventureManager->worldMap->Update();

		break;
	}
	case LOAD_ADVENTURE_MAP:
	{
		//havent tested but should be necessary here
		while (window->pollEvent(ev))
		{
		}

		loadingBackpack->Update();

		if (netplayManager != NULL && netplayManager->IsPracticeMode())
		{
			netplayManager->Update();
		}
		

		if (loadThread != NULL)
		{
			if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
			{
				//window->setVerticalSyncEnabled(true);
				delete loadThread;
				loadThread = NULL;

				if (netplayManager != NULL && netplayManager->IsPracticeMode() )
				{
					if (netplayManager->TrySetupPractice(currLevel))
					{
						SetMode(RUN_ADVENTURE_MAP);
					}
					else
					{
						SetMode(SETUP_PRACTICE_ADVENTURE_MAP);
					}
				}
				else
				{
					SetMode(RUN_ADVENTURE_MAP);
				}
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
	case SETUP_PRACTICE_ADVENTURE_MAP:
	{
		//havent tested but should be necessary here
		while (window->pollEvent(ev))
		{
		}

		loadingBackpack->Update();

		netplayManager->Update();

		if (netplayManager->TrySetupPractice(currLevel))
		{
			SetMode(RUN_ADVENTURE_MAP);
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
		}

		if (netplayManager != NULL && netplayManager->IsPracticeMode())
		{
			netplayManager->Update();
		}

		if (adventureManager->kinBoostScreen->IsEnded())//swiper->IsPostWipe())
		{
			fader->Fade(true, 30, Color::Black, true, EffectLayer::IN_FRONT_OF_UI);
			SetMode(RUN_ADVENTURE_MAP);
		}
		else if (adventureManager->kinBoostScreen->level == NULL && loadThread == NULL && deadThread == NULL && adventureManager->kinBoostScreen->IsBoosting())
		{
			if (netplayManager != NULL && netplayManager->IsPracticeMode())
			{
				if (netplayManager->TrySetupPractice(currLevel))
				{
					adventureManager->kinBoostScreen->End();
				}
			}
			else
			{
				adventureManager->kinBoostScreen->End();
			}
		}
		else
		{
			if (adventureManager->kinBoostScreen->frame == 60 && adventureManager->kinBoostScreen->IsBoosting())
			{
				//window->setVerticalSyncEnabled(false);
				//window->setFramerateLimit(60);
				//string levelPath = kinBoostScreen->level->GetFullName();//kinBoostScreen->levName;
				Level *lev = adventureManager->kinBoostScreen->level;
				deadThread = new boost::thread(MainMenu::sGoToNextLevel, this, &adventureManager->adventureFile.GetMap(lev->index), lev);
				adventureManager->kinBoostScreen->level = NULL;
			}

		}
		adventureManager->kinBoostScreen->Update();
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

		adventureManager->leaderboard->Reset();

		if (result == GameSession::GR_EXIT_PRACTICE_TO_RACE)
		{
			window->setView(oldView);

			const MatchParams &oldParams = currLevel->matchParams;
			netplayManager->matchParams.Clear();

			netplayManager->matchParams.mapPath = oldParams.mapPath;
			//netplayManager->matchParams.saveFile = oldParams.saveFile; //savefile should be NULL
			netplayManager->matchParams.netplayManager = oldParams.netplayManager;
			netplayManager->matchParams.gameModeType = MatchParams::GAME_MODE_PARALLEL_RACE;
			netplayManager->matchParams.randSeed = oldParams.randSeed;

			netplayManager->matchParams.playerSkins[0] = netplayManager->netplayPlayers[0].skinIndex; //= oldParams.playerSkins;
			netplayManager->matchParams.playerSkins[1] = netplayManager->netplayPlayers[1].skinIndex; //= oldParams.playerSkins;

			netplayManager->matchParams.controllerStateVec = oldParams.controllerStateVec;
			netplayManager->matchParams.controlProfiles = oldParams.controlProfiles;

			netplayManager->myControllerInput = netplayManager->matchParams.controllerStateVec[0];
			netplayManager->myCurrProfile = netplayManager->matchParams.controlProfiles[0];

			netplayManager->numPlayers = 2;//numOtherPlayers + 1; //+1 is me

			netplayManager->matchParams.numPlayers = netplayManager->numPlayers; //always keep this synced up with numPlayers

			gameRunType = MainMenu::GRT_FREEPLAY;

			currLevel->UpdateMatchParams(netplayManager->matchParams);

			netplayManager->game = currLevel;

			currLevel = NULL;

			//needs to wait until after game is assigned to work
			netplayManager->StartTestRace();

			SetMode(QUICKPLAY_PRE_MATCH);
			customMatchManager->StartQuickplayPreMatchScreen();
			break;
		}

		if (netplayManager != NULL && netplayManager->IsPracticeMode())
		{
			netplayManager->Abort();
		}

		window->setView(oldView);

		//JUST FOR TESTING
		//worldMap = new WorldMap(this);
		//saveMenu = new SaveMenuScreen(this);
		//saveMenu->Reset();
		//----ENDING JUST FOR TESTING

		//SingleAxisSelector *sa = worldMap->selectors[worldMap->selectedColony]->FocusedSector()->mapSASelector;
		//int numLevels = worldMap->GetCurrSectorNumLevels();//worldMap->selectors[worldMap->selectedColony]->sectors[secIndex]->numLevels;
		
		if (result == GameSession::GR_WIN)
		{
			//currFile->Save();

			//delete currLevel;
			//currLevel = NULL;
			fader->Clear();

			ReturnToWorldAfterLevel();
		}
		else if (result == GameSession::GR_WINCONTINUE)
		{
			if (!adventureManager->TryToGoToNextLevel())
			{
				

				ReturnToWorldAfterLevel();
			}
			//int w = 0;
			//int s = 0;
			//int m = 0;
			//adventureManager->adventureFile.GetMapIndexes(currLevel->level->index, w, s, m);

			//adventureManager->adventurePlanet->worlds[w].sectors[s].numLevels;

			//auto &sector = adventureManager->adventureFile.GetSector(w, s);

			//if (m < sector.GetNumExistingMaps() - 1)
			//{
			//	++m;

			//	AdventureNextLevel(sector.maps[m].//&(//worldMap->GetCurrSector().levels[sa->currIndex]));
			//}
			//else
			//{
			//	currFile->Save();

			//	delete currLevel;
			//	currLevel = NULL;

			//	fader->Clear();

			//	ReturnToWorldAfterLevel();
			//}

		}
		else if (result == GameSession::GR_EXITTITLE)
		{
			//Sector &sec = worldMap->GetCurrSector();
			//for (int i = 0; i < sec.numLevels; ++i)
			//{
			//	currFile->SetLevelNotJustBeaten(&sec.levels[i]);
			//}
			////fix this later for other options

			//currFile->Save();

			

			LoadMode(TITLEMENU);
		}
		else if (result == GameSession::GR_EXITGAME)
		{
			//Sector &sec = worldMap->GetCurrSector();
			//for (int i = 0; i < sec.numLevels; ++i)
			//{
			//	currFile->SetLevelNotJustBeaten(&sec.levels[i]);
			//}
			////fix this later for other options

			//currFile->Save();

			delete currLevel;
			currLevel = NULL;

			SetMode(EXITING);
			quit = true;
		}
		else
		{
			//Sector &sec = worldMap->GetCurrSector();
			//for (int i = 0; i < sec.numLevels; ++i)
			//{
			//	currFile->SetLevelNotJustBeaten(&sec.levels[i]);
			//}
			////fix this later for other options

			//currFile->Save();

			//might leak
			//delete currLevel;
			//currLevel = NULL;

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

		MOUSE.SetControllersOn(true);

		window->setView(oldView);

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
			delete currTutorialSession;
			currTutorialSession = NULL;

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
			delete currTutorialSession;
			currTutorialSession = NULL;

			//delete adventureManager;
			//adventureManager = NULL;

			SetMode(EXITING);
			quit = true;
		}
		else
		{
			musicPlayer->TransitionMusic(menuMusic, 60);
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
			adventureManager->saveMenu->HandleEvent(ev);
		}

		adventureManager->worldMap->Update();
		if (!adventureManager->saveMenu->Update())
		{
			musicPlayer->FadeOutCurrentMusic(30);
			LoadMode(TITLEMENU);
		}
		else
		{
			if (adventureManager->saveMenu->action == SaveMenuScreen::TRANSITIONTUTORIAL)
			{
				MatchParams mp;
				mp.mapPath = TUTORIAL_PATH;//"Resources/Maps/EarlyAccess/tut1" + string(MAP_EXT);
				//mp.controllerStateVec[0] = singlePlayerControllerJoinScreen->playerBox->controllerStates;
				//fix this soon!
				mp.randSeed = time(0);
				mp.numPlayers = 1;
				mp.gameModeType = MatchParams::GAME_MODE_BASIC;

				mp.controllerStateVec[0] = adventureManager->controllerInput;
				mp.controlProfiles[0] = adventureManager->currProfile;
				mp.playerSkins[0] = adventureManager->currSaveFile->defaultSkinIndex;

				*menuMatchParams = mp;

				musicPlayer->FadeOutCurrentMusic(30);
				LoadMode(ADVENTURETUTORIAL);
			}
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
			adventureManager->saveMenu->Reset();
		}

		if (transFrame < transLength / 2)
		{
			titleScreen->Update();
		}
		else
		{
			adventureManager->saveMenu->Update();
			adventureManager->worldMap->Update();
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
			adventureManager->saveMenu->Reset();
		}

		if (transFrame < transLength / 2)
		{
			adventureManager->saveMenu->Update();
			adventureManager->worldMap->Update();
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
	case TRANS_MAIN_TO_GAME_SETTINGS:
	{
		while (window->pollEvent(ev))
		{

		}
		SetMode(GAME_SETTINGS);
		config->WaitForLoad();

		gameSettingsScreen->UpdateFromConfig();
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
	case GAME_SETTINGS:
	{
		while (window->pollEvent(ev))
		{
			gameSettingsScreen->HandleEvent(ev);
		}

		gameSettingsScreen->Update();

		if (gameSettingsScreen->action == GameSettingsScreen::A_CANCEL)
		{
			SetMode(TITLEMENU);
		}
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
		
		titleScreen->creditsMenuScreen->Update();

		if (titleScreen->creditsMenuScreen->action == CreditsMenuScreen::A_BACK)
		{
			SetMode(TITLEMENU);
		}
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
			adventureManager->worldMap->RunSelectedMap();
		}
		else if (modeFrame < 30)
		{
			adventureManager->worldMap->Update();
		}
		break;
	}
	case INTROMOVIE:
	{
		while (window->pollEvent(ev))
		{

		}
		//ControllerState &introInput = GetCurrInputUnfiltered(0);
		introMovie->skipHolder->Update(CONTROLLERS.ButtonHeld_A());
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

		if (CONTROLLERS.ButtonPressed_A())
		{
			//this will break because worldmap doesnt exist at this point i think
			adventureManager->worldMap->CurrSelector()->ReturnFromMap();
			SetMode(WORLDMAP_COLONY);
			adventureManager->worldMap->CurrSelector()->FocusedSector()->UpdateLevelStats();
			adventureManager->worldMap->CurrSelector()->FocusedSector()->UpdateStats();
			adventureManager->worldMap->CurrSelector()->FocusedSector()->UpdateMapPreview();

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
			delete currFreePlaySession;
			currFreePlaySession = NULL;

			SetMode(EXITING);
			quit = true;
		}
		else if (result == GameSession::GR_EXITTITLE)
		{
			LoadMode(TITLEMENU);
		}
		else
		{
			LoadMode(FREEPLAY);
			//LoadMode(BROWSE_WORKSHOP);
		}

		break;
	}
	case RUN_WORKSHOP_MAP:
	{
		while (window->pollEvent(ev))
		{

		}

		View oldView = window->getView();

		int result = currFreePlaySession->Run();

		window->setView(oldView);

		if (result == GameSession::GR_EXITGAME)
		{
			delete currFreePlaySession;
			currFreePlaySession = NULL;

			SetMode(EXITING);
			quit = true;
		}
		else if (result == GameSession::GR_EXITTITLE)
		{
			LoadMode(TITLEMENU);
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
		else if (workshopBrowser->action == WorkshopBrowser::A_PLAY)
		{
			DownloadAndRunWorkshopMap();
			workshopBrowser->action = WorkshopBrowser::A_POPUP;
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
			MOUSE.Hide();
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
			LoadMode(TITLEMENU);
			//SetMode(TITLEMENU);
			break;
		}
		
		break;
	}
	case EDITOR_MENU:
	{
		while (window->pollEvent(ev))
		{
			editorMenuScreen->HandleEvent(ev);
		}
		editorMenuScreen->Update();

		switch (editorMenuScreen->action)
		{
		case EditorMenuScreen::A_NEW_MAP:

			//mapBrowserScreen = MainMenu::GetInstance()->mapBrowserScreen;

			//mapBrowserScreen->StartWorkshopBrowsing( MapBrowser::WORKSHOP );

			/*assert(workshopBrowser == NULL);
			workshopBrowser = new WorkshopBrowser;

			workshopBrowser->Start();

			SetMode(BROWSE_WORKSHOP);*/

			RunEditor(TITLEMENU, "");

			break;
		case EditorMenuScreen::A_OPEN_MAP:
			//netplayManager->FindQuickplayMatch();
			//SetMode(QUICKPLAY_TEST);
			RunEditor(TITLEMENU, editorMenuScreen->mapBrowserScreen->browserHandler->confirmedMapFilePath);
			break;
		case EditorMenuScreen::A_CANCELLED:
		{
			LoadMode(TITLEMENU);
			break;
		}
		//case OnlineMenuScreen::A_CREATE_LOBBY:
		//	customMatchManager->CreateCustomLobby();
		//	SetMode(CUSTOM_MATCH_SETUP);
		//	break;
		//case OnlineMenuScreen::A_JOIN_LOBBY:
		//	customMatchManager->BrowseCustomLobbies();
		//	SetMode(CUSTOM_MATCH_SETUP);
		//	break;
		//case OnlineMenuScreen::A_CANCELLED:
		//	LoadMode(TITLEMENU);
		//	//SetMode(TITLEMENU);
		//	break;
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
	case CUSTOM_MATCH_SETUP_FROM_PRACTICE:
	{
		while (window->pollEvent(ev))
		{
			customMatchManager->HandleEvent(ev);
		}

		if (!customMatchManager->Update())
		{
			//update this soon
			SetMode(ONLINE_MENU);
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

			modeToLoad = RUN_WORKSHOP_MAP;//RUN_FREEPLAY_MAP;//FREEPLAY;//RUN_FREEPLAY_MAP;



			//MapNode *selectedNode = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;

			//if (selectedNode == NULL)
			//	assert(0);

			menuMatchParams->Clear();
			menuMatchParams->mapPath = workshopBrowser->workshopMapPopup->currMapNode->filePath;//mapBrowserScreen->browserHandler->confirmedMapFilePath;//selectedNode->filePath.string();

			//should always need to do this since you're always coming from the workshop browser to get to this state.
			menuMatchParams->controllerStateVec[0] = netplayManager->myControllerInput;
			menuMatchParams->controlProfiles[0] = netplayManager->myCurrProfile;

			loadThread = new boost::thread(MainMenu::sTransitionMode, this, modeLoadingFrom, modeToLoad);
			SetMode(LOADINGMENULOOP);
		}
		break;
	case FREEPLAY:
	{
		while (window->pollEvent(ev))
		{
			freeplayScreen->HandleEvent(ev);
		}

		freeplayScreen->Update();

		if (freeplayScreen->action == FreeplayScreen::A_START)
		{
			//LoadMode(RUN_FREEPLAY_MAP);
			//RunFreePlayMap(freeplayScreen->GetMatchParams().mapPath.string());
			*menuMatchParams = freeplayScreen->GetMatchParams();
			LoadMode(RUN_FREEPLAY_MAP);
		}
		else if (freeplayScreen->action == FreeplayScreen::A_BACK)
		{
			LoadMode(TITLEMENU);
		}
		break;
	}
	case SINGLE_PLAYER_CONTROLLER_JOIN_TUTORIAL:
	{
		while (window->pollEvent(ev))
		{
			singlePlayerControllerJoinScreen->HandleEvent(ev);
		}

		singlePlayerControllerJoinScreen->Update();

		if (singlePlayerControllerJoinScreen->action == SinglePlayerControllerJoinScreen::A_START)
		{
			MatchParams mp;
			mp.mapPath = TUTORIAL_PATH;
			mp.controllerStateVec[0] = singlePlayerControllerJoinScreen->playerBoxGroup->GetControllerStates(0);
			mp.playerSkins[0] = singlePlayerControllerJoinScreen->playerBoxGroup->GetSkinIndex(0);
			mp.controlProfiles[0] = singlePlayerControllerJoinScreen->playerBoxGroup->GetControlProfile(0);
			mp.playerSkins[0] = singlePlayerControllerJoinScreen->playerBoxGroup->GetSkinIndex(0);
			mp.randSeed = time(0);
			mp.numPlayers = 1;
			mp.gameModeType = MatchParams::GAME_MODE_BASIC;
			*menuMatchParams = mp;

			LoadMode(TUTORIAL);
		}
		else if (singlePlayerControllerJoinScreen->action == SinglePlayerControllerJoinScreen::A_BACK)
		{
			LoadMode(TITLEMENU);
		}
		break;
	}
	case SINGLE_PLAYER_CONTROLLER_JOIN_ADVENTURE:
	{
		while (window->pollEvent(ev))
		{
			singlePlayerControllerJoinScreen->HandleEvent(ev);
		}

		singlePlayerControllerJoinScreen->Update();

		if (singlePlayerControllerJoinScreen->action == SinglePlayerControllerJoinScreen::A_START)
		{
			LoadMode(SAVEMENU);
		}
		else if (singlePlayerControllerJoinScreen->action == SinglePlayerControllerJoinScreen::A_BACK)
		{
			LoadMode(TITLEMENU);
		}
		break;
	}
	case SINGLE_PLAYER_CONTROLLER_JOIN_ONLINE:
	{
		while (window->pollEvent(ev))
		{
			singlePlayerControllerJoinScreen->HandleEvent(ev);
		}

		singlePlayerControllerJoinScreen->Update();

		if (singlePlayerControllerJoinScreen->action == SinglePlayerControllerJoinScreen::A_START)
		{
			netplayManager->myControllerInput = singlePlayerControllerJoinScreen->playerBoxGroup->GetControllerStates(0);
			netplayManager->myCurrProfile = singlePlayerControllerJoinScreen->playerBoxGroup->GetControlProfile(0);

			//skin deliberately not set here. set later in the menus

			
			LoadMode(ONLINE_MENU);
		}
		else if (singlePlayerControllerJoinScreen->action == SinglePlayerControllerJoinScreen::A_BACK)
		{
			LoadMode(TITLEMENU);
		}
		break;
	}
	case QUICKPLAY_TEST:
	{
		while (window->pollEvent(ev))
		{
			switch (ev.type)
			{
			case sf::Event::KeyPressed:
			{
				//needs cleanup
				if (ev.key.code == Keyboard::Escape)
				{
					MOUSE.Show();
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

			SetMode(QUICKPLAY_PRE_MATCH);
			customMatchManager->StartQuickplayPreMatchScreen();

			//SetMode(QUICKPLAY_PLAY);

			//turned this off recently
			//fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
		}
		break;
	}
	case QUICKPLAY_PRE_MATCH:
	{
		while (window->pollEvent(ev))
		{
			switch (ev.type)
			{
			}
		}

		customMatchManager->Update();

		if (customMatchManager->action == CustomMatchManager::A_QUICKPLAY_PRE_MATCH_DONE)
		{
			SetMode(QUICKPLAY_PLAY);
		}
		break;
	}
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
			mp.filePath = "Resources/Maps/W2/afighting1.kinmap";
			netplayManager->RunMatch(&mp);*/
		}
		else// if (!fader->IsFading())
		{
			if (netplayManager->action == NetplayManager::A_READY_TO_RUN)
			{
				cout << "RUNNING MATCHHHHHHHH" << endl;
				netplayManager->RunMatch();

				//results screen instead...

				if (netplayManager->action == NetplayManager::A_DISCONNECT)
				{
					//fix this so when the opponent disconnects after the game has ended that everything is still fine.
					cout << "EXITED ON DISCONNECT" << endl;
					messagePopup->Pop("Opponent disconnected");//, 60);
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
					SetMode(NETPLAY_MATCH_RESULTS);
				}
			}
		}
		break;
	}
	case NETPLAY_MATCH_RESULTS:
	{
		while (window->pollEvent(ev))
		{
			//matchResultsScreen->HandleEvent(ev);
		}

		netplayManager->Update();

		if (netplayManager->CheckResultsScreen())
		{
			SetMode(POST_MATCH_OPTIONS);
			break;
		}

		if (!matchResultsScreen->Update())
		{
			
			SetMode(POST_MATCH_OPTIONS);
			//SetMode(TITLEMENU);
		}


		if (netplayManager->action == NetplayManager::A_WAIT_FOR_GGPO_SYNC)
		{
			cout << "got signal to rematch too early" << endl;
			SetMode(POST_MATCH_OPTIONS);
		}
		break;
	}
	case POST_MATCH_OPTIONS:
	{
		while (window->pollEvent(ev))
		{
			customMatchManager->HandleEvent(ev);
			//matchResultsScreen->HandleEvent(ev);
		}

		if ( !netplayManager->IsHost() && netplayManager->postMatchOptionReceived != -1 )//NetplayManager::A_WAIT_FOR_GGPO_SYNC)
		{
			switch (netplayManager->postMatchOptionReceived)
			{
			case NetplayManager::POST_MATCH_A_REMATCH:
			{
				netplayManager->ClearPostPracticeMatchInfo(); //for practice race rematches only

				delete matchResultsScreen;
				matchResultsScreen = NULL;

				netplayManager->action = NetplayManager::A_WAIT_FOR_GGPO_SYNC;

				cout << "client starting map rematch!!" << endl;
				netplayManager->game->InitGGPO();
				SetMode(QUICKPLAY_PLAY);
				fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
				break;
			}
			case NetplayManager::POST_MATCH_A_CHOOSE_MAP:
			{
				netplayManager->CleanupMatch();
				
				delete matchResultsScreen;
				matchResultsScreen = NULL;

				SetMode(CUSTOM_MATCH_SETUP);
				customMatchManager->StartClientWaitingRoomForNextMap();
				break;
			}
			case NetplayManager::POST_MATCH_A_LEAVE:
			{
				break;
			}
			/*case NetplayManager::POST_MATCH_A_QUICKPLAY_KEEP_PLAYING:
			{
				

				break;
			}*/
			case NetplayManager::POST_MATCH_A_QUICKPLAY_LEAVE:
			{
				break;
			}
			}
			
			netplayManager->postMatchOptionReceived = -1;
			break;
		}

		customMatchManager->Update();

		

		

		bool isHost = netplayManager->IsHost();

		//bool needResultsScreen = false;

		//bool hostNeedsResults = isHost && customMatchManager->action == CustomMatchManager::A_POST_MATCH_HOST;
		//bool clientNeedsResults = 
		//

		//needResultsScreen = ;
		// 
		//if ((isHost && customMatchManager->action != CustomMatchManager::A_POST_MATCH_HOST)
		//	|| (!isHost && customMatchManager->action != CustomMatchManager::A_POST_MATCH_CLIENT)
		//	|| ( customMatchManager->action !)
		//{
		//	//delete matchResultsScreen;
		//	//matchResultsScreen = NULL;
		//	//cout << "deleting match results screen early" << endl;
		//}

		switch (customMatchManager->action)
		{
		case CustomMatchManager::A_POST_MATCH_CLIENT:
		{
			if (!netplayManager->IsConnectedToHost())
			{
				netplayManager->CleanupMatch();
				netplayManager->Abort();

				delete matchResultsScreen;
				matchResultsScreen = NULL;
				//SetMode(TITLEMENU);
				LoadMode(TITLEMENU);
			}
			
			//fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
			break;
		}
		case CustomMatchManager::A_POST_MATCH_CLIENT_LEAVE:
		{
			netplayManager->CleanupMatch();
			netplayManager->Abort();

			delete matchResultsScreen;
			matchResultsScreen = NULL;

			LoadMode(TITLEMENU);
			//SetMode(TITLEMENU);
			 
			//some signal to tell the host that you have left, or just disconnect? probably just disconnect
			break;
		}
		case CustomMatchManager::A_POST_MATCH_HOST_LEAVE:
		{
			netplayManager->CleanupMatch();
			netplayManager->Abort();

			delete matchResultsScreen;
			matchResultsScreen = NULL;

			LoadMode(TITLEMENU);
			//SetMode(TITLEMENU);

			//instead of just disconnecting, need to send a signal to the clients on which option was chosen.
			break;
		}
		case CustomMatchManager::A_POST_MATCH_HOST_REMATCH:
		{
			netplayManager->game->InitGGPO();
			netplayManager->HostInitiateRematch();
			
			fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);

			delete matchResultsScreen;
			matchResultsScreen = NULL;

			SetMode(QUICKPLAY_PLAY);
			//QUICKPLAY_PLAY
			//SetMode(QUICKPLAY_TEST);
			break;
		}
		case CustomMatchManager::A_POST_MATCH_HOST_CHOOSE_MAP:
		{
			//netplayManager->CleanupMatch();
			//netplayManager->Abort();
			//SetMode(TITLEMENU);

			netplayManager->CleanupMatch();
			netplayManager->SendPostMatchChooseMapSignalToClients();

			delete matchResultsScreen;
			matchResultsScreen = NULL;

			customMatchManager->BrowseForNextMap();
			SetMode(CUSTOM_MATCH_SETUP);

			//now go to map choosing stage
			break;
		}
		case CustomMatchManager::A_POST_MATCH_QUICKPLAY:
		{
			if (isHost)
			{
				if (netplayManager->receivedLeaveNetplaySignal)
				{
					cout << "received signal to exit and kick everyone out" << endl;
					netplayManager->CleanupMatch();
					netplayManager->Abort();

					delete matchResultsScreen;
					matchResultsScreen = NULL;

					LoadMode(TITLEMENU);
				}
				//wait for leaving messages
			}
			else
			{
				if (!netplayManager->IsConnectedToHost())
				{
					netplayManager->CleanupMatch();
					netplayManager->Abort();

					delete matchResultsScreen;
					matchResultsScreen = NULL;
					//SetMode(TITLEMENU);
					LoadMode(TITLEMENU);
				}
			}
			break;
		}
		case CustomMatchManager::A_POST_MATCH_QUICKPLAY_VOTE_KEEP_PLAYING:
		{
			netplayManager->CleanupMatch();

			if (netplayManager->IsHost())
			{
				netplayManager->HostQuickplayVoteToKeepPlaying();
			}
			else
			{
				netplayManager->PrepareClientForNextQuickplayMap();
				netplayManager->SendPostMatchQuickplayVoteToKeepPlayingToHost();
			}

			customMatchManager->action = CustomMatchManager::A_POST_MATCH_QUICKPLAY_VOTE_KEEP_PLAYING_WAIT_FOR_OTHERS;
			break;
		}
		case CustomMatchManager::A_POST_MATCH_QUICKPLAY_VOTE_KEEP_PLAYING_WAIT_FOR_OTHERS:
		{
			//host
			if (netplayManager->IsHost())
			{
				if (netplayManager->action == NetplayManager::A_ALL_VOTED_TO_KEEP_PLAYING)
				{
					cout << "we made it!" << endl;

					netplayManager->CleanupMatch();
					//netplayManager->SendPostMatchQuickplayKeepPlayingSignalToClients();

					delete matchResultsScreen;
					matchResultsScreen = NULL;

					netplayManager->HostLoadNextQuickplayMap();

					SetMode(QUICKPLAY_TEST);

					//customMatchManager->BrowseForNextMap();
					//SetMode(CUSTOM_MATCH_SETUP);
					//netplayManager->game->InitGGPO();
					//netplayManager->HostInitiateRematch();

					//fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);

					//SetMode(QUICKPLAY_PLAY);
				}
			}
			else
			{
				//cout << "my lobbymanager index: " << netplayManager->lobbyManager->currentLobby.data.mapIndex << ", my netplay: " << netplayManager->currMapIndex << endl;
				if (netplayManager->lobbyManager->currentLobby.data.mapIndex > netplayManager->currMapIndex)
				{
					//cout << "made it loading" << endl;

					netplayManager->currMapIndex = netplayManager->lobbyManager->currentLobby.data.mapIndex;
					//maybe tell the lobby manager to refresh or check for updates to the lobby if this doens't match

					//netplayManager->receivedNextMapData = false;

					netplayManager->CheckForMapAndSetMatchParams();

					SetMode(QUICKPLAY_TEST);

					netplayManager->LoadMap();
				}
			}
			
			break;
		}
		case CustomMatchManager::A_POST_MATCH_QUICKPLAY_LEAVE:
		{
			if (!isHost)
			{
				netplayManager->SendPostMatchQuickplayLeaveSignalToHost();

				netplayManager->CleanupMatch();
				netplayManager->Abort();

				delete matchResultsScreen;
				matchResultsScreen = NULL;

				LoadMode(TITLEMENU);
			}
			else
			{
				netplayManager->CleanupMatch();
				netplayManager->Abort();

				delete matchResultsScreen;
				matchResultsScreen = NULL;

				LoadMode(TITLEMENU);
			}
			
			break;
		}
		case CustomMatchManager::A_POST_MATCH_PRACTICE:
		{
			if (isHost)
			{
				if (netplayManager->receivedLeaveNetplaySignal)
				{
					cout << "received signal from client to exit" << endl;
					netplayManager->CleanupMatch();
					netplayManager->Abort();

					delete matchResultsScreen;
					matchResultsScreen = NULL;

					LoadMode(TITLEMENU);
				}
				else if( netplayManager->PeerWantsToKeepPlayingPractice() && netplayManager->wantsToKeepPlayingPractice )
				{
					netplayManager->ClearPostPracticeMatchInfo(); //for practice race rematches only

					netplayManager->game->InitGGPO();
					netplayManager->HostInitiateRematch();

					fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);

					delete matchResultsScreen;
					matchResultsScreen = NULL;

					SetMode(QUICKPLAY_PLAY);
				}
				
				//wait for leaving messages
			}
			else
			{
				if (!netplayManager->IsConnectedToHost())
				{
					cout << "leaving practice to title screen because the host left" << "\n";

					netplayManager->CleanupMatch();
					netplayManager->Abort();

					delete matchResultsScreen;
					matchResultsScreen = NULL;
					//SetMode(TITLEMENU);
					LoadMode(TITLEMENU);
				}
			}
			//fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
			break;
		}
		case CustomMatchManager::A_POST_MATCH_PRACTICE_LEAVE:
		{
			if (!isHost)
			{
				netplayManager->SendPostMatchPracticeLeaveSignalToHost();

				netplayManager->CleanupMatch();
				netplayManager->Abort();

				delete matchResultsScreen;
				matchResultsScreen = NULL;

				LoadMode(TITLEMENU);
			}
			else
			{
				netplayManager->CleanupMatch();
				netplayManager->Abort();

				delete matchResultsScreen;
				matchResultsScreen = NULL;

				LoadMode(TITLEMENU);
			}
			break;
		}
		}

		break;
	}
	case CLOSED_BETA:
	{
		while (window->pollEvent(ev))
		{

		}

		closedBetaScreen->Update();

		if (closedBetaScreen->action == ClosedBetaScreen::A_DONE)
		{
			LoadMode(TITLEMENU);
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

Tileset *MainMenu::GetPlayerTileset(PlayerTilesetOptions option)
{
	return playerTilesetMap[option];
}

void MainMenu::LoadMode(Mode m)
{
	if (m == TITLEMENU || m == ONLINE_MENU )
	{
		MOUSE.Show();
	}

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

void MainMenu::UnlockSkin(int skinIndex)
{
	globalFile->UnlockSkin(skinIndex);
}

bool MainMenu::IsSkinUnlocked(int skinIndex)
{
	return globalFile->IsSkinUnlocked(skinIndex);
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
		//titleScreen->testForm->HandleEvent(ev);
		//continue;
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
				//GameEditLoop2( "Maps/aba.kinmap" );
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
	

	if (isMouseClickedOnCurrentOption )//|| CONTROLLERS.ButtonPressed_A() )
	{
		soundNodeList->ActivateSound(soundManager.GetSound("main_menu_select"));
		switch (saSelector->currIndex)
		{
		case M_ADVENTURE:
		{
			//SetMode(MATCH_RESULTS);
			//matchResultsScreen = netplayManager->CreateResultsScreen();
			MOUSE.Hide();
			musicPlayer->FadeOutCurrentMusic(30);
			LoadMode(SINGLE_PLAYER_CONTROLLER_JOIN_ADVENTURE);
			//customCursor->SetMode(CustomCursor::M_SHIP);
			
			break;
		}
		case M_FREE_PLAY:
		{
			MOUSE.Hide();
			LoadMode(FREEPLAY);
			break;
		}
		case M_ONLINE:
		{
			MOUSE.Hide();
			LoadMode(SINGLE_PLAYER_CONTROLLER_JOIN_ONLINE);
			//onlineMenuScreen->Start();
			//LoadMode(ONLINE_MENU);
			//SetMode(ONLINE_MENU);

			//netplayManager->isSyncTest = true;
			//netplayManager->FindQuickplayMatch();
			//SetMode(QUICKPLAY_TEST);
			
			break;
		}
		case M_LEVEL_EDITOR:
		{
			musicPlayer->FadeOutCurrentMusic(30);
			LoadMode(EDITOR_MENU);
			//RunEditor(TITLEMENU, "");
			break;
		}
		case M_OPTIONS:
		{
			//config->Load();
			config->Load();
			//Config::CreateLoadThread(config);
			//config->WaitForLoad();
			
			gameSettingsScreen->Start();
			SetMode(GAME_SETTINGS);
			//SetMode(TRANS_MAIN_TO_GAME_SETTINGS);
			break;
		}
		case M_TUTORIAL:
		{
			musicPlayer->FadeOutCurrentMusic(30);
			//LoadMode(TUTORIAL);
			//singlePlayerControllerJoinScreen->
			customCursor->Hide();
			LoadMode(SINGLE_PLAYER_CONTROLLER_JOIN_TUTORIAL);
			break;
		}
		case M_CREDITS:
		{
			SetMode(CREDITS);
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
		if (CONTROLLERS.KeyboardButtonHeld(Keyboard::Tilde))
		{
			quit = true;
			return;
		}

		int oldIndex = saSelector->currIndex;
		//int res = saSelector->UpdateIndex(menuCurrInput.LUp(), menuCurrInput.LDown());

		//if mouse is enabled, lets you mouse over to select
		for (int i = 0; i < saSelector->totalItems; ++i)
		{
			if (activatedMainMenuOptions[i] && saSelector->currIndex != i )
			{
				if (QuadContainsPoint(mainMenuOptionQuads + i * 4,
					MOUSE.GetFloatPos()))
				{
					saSelector->SetIndex(i);
					soundNodeList->ActivateSound(soundManager.GetSound("main_menu_change"));
					break;
				}
			}
		}
		

		/*if (res != 0)
		{
			soundNodeList->ActivateSound(soundManager.GetSound("main_menu_change"));
		}*/

		//while (!activatedMainMenuOptions[saSelector->currIndex])
		//{
		//	if (saSelector->currIndex != oldIndex)
		//	{
		//		if (saSelector->currIndex > oldIndex || (saSelector->currIndex == 0 && oldIndex == saSelector->totalItems - 1))
		//		{
		//			//down
		//			++saSelector->currIndex;
		//			if (saSelector->currIndex == saSelector->totalItems)
		//			{
		//				saSelector->currIndex = 0;
		//			}
		//		}
		//		else
		//		{
		//			--saSelector->currIndex;
		//			if (saSelector->currIndex < 0)
		//			{
		//				saSelector->currIndex = saSelector->totalItems - 1;
		//			}
		//		}
		//	}
		//}

	}

	UpdateMenuOptionText();
}

void MainMenu::RegisterShader(const std::string &shaderType)
{
	assert(shaderStringsStreams.count(shaderType) == 0);

	ifstream is( "Resources/Shader/" + shaderType + ".frag");

	shaderStringsStreams[shaderType] << is.rdbuf();

	cout << "registering shader: " << shaderType << "\n";
}

bool MainMenu::TryAssignShader(sf::Shader &sh, const std::string &shaderType)
{
	if (shaderStringsStreams.count(shaderType) == 0)
	{
		return false;
	}
	else
	{
		if (!sh.loadFromMemory(shaderStringsStreams[shaderType].str(), sf::Shader::Fragment))
		{
			cout << "reigstered shader not loading correctly: " << shaderType << "\n";
			return false;
		}
		else
		{
			return true;
		}
	}
}

bool MainMenu::LoadShader(sf::Shader &sh, const std::string &shaderType)
{
	if (shaderStringsStreams.count(shaderType) == 0)
	{
		RegisterShader(shaderType);

		if (sh.loadFromMemory(shaderStringsStreams[shaderType].str(), sf::Shader::Fragment))
		{
			return true;
		}
		else
		{
			cout << "unregistered shader not loading correctly: " << shaderType << "\n";
		}
	}
	else
	{
		if (sh.loadFromMemory(shaderStringsStreams[shaderType].str(), sf::Shader::Fragment))
		{
			return true;
		}
		else
		{
			cout << "registered shader not loading correctly: " << shaderType << "\n";
		}
	}
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
		messagePopup->Draw(preScreenTexture);
		break;
	}
	case WORLDMAP:
	case WORLDMAP_COLONY:
	{
		preScreenTexture->setView(v);
		adventureManager->DrawWorldMap(preScreenTexture);
	}
	break;
	case TRANS_SAVE_TO_WORLDMAP:
	case SAVEMENU:
	{
		adventureManager->DrawWorldMap(preScreenTexture);
		adventureManager->saveMenu->Draw(preScreenTexture);
		break;
	}
	case SETUP_PRACTICE_ADVENTURE_MAP:
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
		adventureManager->kinBoostScreen->Draw(preScreenTexture);
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
			adventureManager->DrawWorldMap(preScreenTexture);
			adventureManager->saveMenu->Draw(preScreenTexture);
		}
		break;
	}
	case TRANS_SAVE_TO_MAIN:
	{
		int tFrame = transFrame - 1;
		if (tFrame < transLength / 2)
		{
			adventureManager->DrawWorldMap(preScreenTexture);
			adventureManager->saveMenu->Draw(preScreenTexture);
		}
		else
		{
			titleScreen->Draw(preScreenTexture);
		}
		break;
	}
	case TRANS_MAIN_TO_GAME_SETTINGS:
	{
		preScreenTexture->setView(v);
		gameSettingsScreen->Draw(preScreenTexture);
		break;
	}
	case GAME_SETTINGS:
	{
		preScreenTexture->setView(v);
		gameSettingsScreen->Draw(preScreenTexture);
		break;
	}
	case TRANS_CREDITS_TO_MAIN:
	{
		//preScreenTexture->setView(v);
		//titleScreen->creditsMenuScreen->Draw(preScreenTexture);
		break;
	}
	case TRANS_MAIN_TO_CREDITS:
	{
		//preScreenTexture->setView(v);
		//creditsMenu->Draw(preScreenTexture);
		break;
	}
	case CREDITS:
	{
		preScreenTexture->setView(v);
		
		titleScreen->creditsMenuScreen->Draw(preScreenTexture);
		break;
	}
	case TRANS_WORLDMAP_TO_LOADING:
	{
		if (modeFrame < 30)
		{
			adventureManager->DrawWorldMap(preScreenTexture);
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
	case EDITOR_MENU:
	{
		preScreenTexture->setView(v);
		editorMenuScreen->Draw(preScreenTexture);
		break;
	}
	case CUSTOM_MATCH_SETUP_FROM_PRACTICE:
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
	case RUN_WORKSHOP_MAP:
	{
		break;
	}
	case FREEPLAY:
	{
		freeplayScreen->Draw(preScreenTexture);
		break;
	}
	case SINGLE_PLAYER_CONTROLLER_JOIN_TUTORIAL:
	{
		singlePlayerControllerJoinScreen->Draw(preScreenTexture);
		break;
	}
	case SINGLE_PLAYER_CONTROLLER_JOIN_ADVENTURE:
	{
		singlePlayerControllerJoinScreen->Draw(preScreenTexture);
		break;
	}
	case SINGLE_PLAYER_CONTROLLER_JOIN_ONLINE:
	{
		singlePlayerControllerJoinScreen->Draw(preScreenTexture);
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
	case QUICKPLAY_PRE_MATCH:
	{
		preScreenTexture->setView(v);
		customMatchManager->Draw(preScreenTexture);
		break;
	}
	case QUICKPLAY_PLAY:
	{
		preScreenTexture->setView(v);
		loadingBackpack->Draw(preScreenTexture);
		break;
	}
	case NETPLAY_MATCH_RESULTS:
	{
		preScreenTexture->setView(v);
		matchResultsScreen->Draw(preScreenTexture);
		break;
	}
	case POST_MATCH_OPTIONS:
	{
		preScreenTexture->setView(v);

		//this needs to get fixed for smoothness later
		if (matchResultsScreen != NULL)
		{
			matchResultsScreen->Draw(preScreenTexture);
		}
		

		customMatchManager->Draw(preScreenTexture);		

		break;
	}
	case CLOSED_BETA:
	{
		preScreenTexture->setView(v);
		closedBetaScreen->Draw(preScreenTexture);
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
		adventureManager->kinBoostScreen->DrawLateKin(preScreenTexture);
	}
}

//void MainMenu::RunFreePlayMap(const MatchParams &params)
//{
//	//freeplayMapName = path;
//	LoadMode(MainMenu::RUN_FREEPLAY_MAP);
//	//SetMode(MainMenu::RUNWORKSHOPMAP);
//}

void MainMenu::DownloadAndRunWorkshopMap()
{
	//currWorkshopMap = path;

	/*if (workshopBrowser != NULL)
	{
		delete workshopBrowser;
		workshopBrowser = NULL;
	}*/

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

void MainMenu::OnGameLobbyJoinRequestedCallback(GameLobbyJoinRequested_t *pCallback)
{
	if (menuMode == CUSTOM_MATCH_SETUP && customMatchManager->action == CustomMatchManager::A_LOBBY_BROWSER)
	{
		cout << "attempting to join invited lobby" << endl;
		customMatchManager->lobbyBrowser->TryJoinLobbyFromInvite(pCallback->m_steamIDLobby);

		//netplayManager->lobbyManager->TryJoiningLobby(pCallback->m_steamIDLobby);
		//lobbyBrowser->TryJoinLobby(netplayManager->lobbyManager->joinRequestLobbyId);
	}
	else if (menuMode == TITLEMENU)
	{
		cout << "attempting to join invited lobby from title screen" << endl;

		TransitionMode(menuMode, CUSTOM_MATCH_SETUP);
		SetMode(CUSTOM_MATCH_SETUP);
		onlineMenuScreen->Start();
		customMatchManager->TryEnterLobbyFromInvite(pCallback->m_steamIDLobby);
	}
	//SetJoinRequest(pCallback->m_steamIDLobby, pCallback->m_steamIDFriend);
}

namespace fs = boost::filesystem;
void MainMenu::copyDirectoryRecursively(const fs::path& sourceDir, const fs::path& destinationDir)
{
	//cout << "copy directory recursively" << "\n";
	if (!fs::exists(sourceDir) || !fs::is_directory(sourceDir))
	{
		throw std::runtime_error("Source directory " + sourceDir.string() + " does not exist or is not a directory");
	}
	if (fs::exists(destinationDir))
	{
		throw std::runtime_error("Destination directory " + destinationDir.string() + " already exists");
	}
	if (!fs::create_directory(destinationDir))
	{
		throw std::runtime_error("Cannot create destination directory " + destinationDir.string());
	}

	//if this fails its usually because I used / in the paths instead of backslash
	for (const auto& dirEnt : fs::recursive_directory_iterator{ sourceDir })
	{
		const auto& path = dirEnt.path();
		auto relativePathStr = path.string();
		boost::replace_first(relativePathStr, sourceDir.string(), "");
		fs::copy(path, destinationDir / relativePathStr);
	}
}