#include "MainMenu.h"
#include <boost/filesystem.hpp>
#include "EditSession.h"
#include "GameSession.h"
#include "SaveFile.h"
#include <iostream>
#include <sstream>
#include "Parallax.h"
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


using namespace std;
using namespace sf;
using namespace boost::filesystem;

#define TIMESTEP 1.0 / 60.0


const int LoadingMapProgressDisplay::NUM_LOAD_THREADS = 5;

sf::RenderTexture *MainMenu::preScreenTexture = NULL;
sf::RenderTexture *MainMenu::lastFrameTexture = NULL;
sf::RenderTexture *MainMenu::postProcessTexture = NULL;
sf::RenderTexture *MainMenu::postProcessTexture1 = NULL;
sf::RenderTexture *MainMenu::postProcessTexture2 = NULL;
sf::RenderTexture *MainMenu::minimapTexture = NULL;
sf::RenderTexture *MainMenu::mapTexture = NULL;
sf::RenderTexture *MainMenu::pauseTexture = NULL;
sf::RenderTexture *MainMenu::saveTexture = NULL;
sf::RenderTexture *MainMenu::mapPreviewTexture = NULL;
sf::RenderTexture *MainMenu::extraScreenTexture = NULL;
sf::RenderTexture *MainMenu::auraCheckTexture = NULL;

const int MapSelectionMenu::BOX_WIDTH = 580;
const int MapSelectionMenu::BOX_HEIGHT = 40;
const int MapSelectionMenu::BOX_SPACING = 0;

sf::Font MainMenu::arial;
int MainMenu::masterVolume = 100;

MultiSelectionSection::MultiSelectionSection(MainMenu *p_mainMenu, MapSelectionMenu *p_parent,
	int p_playerIndex, Vector2f &p_topMid )
	:mainMenu( p_mainMenu ), parent( p_parent ), playerIndex( p_playerIndex ), team( T_NOT_CHOSEN ),
	skinIndex( S_STANDARD ), active( false ), topMid( p_topMid )
{
	profileSelect = new ControlProfileMenu( this,playerIndex,
		mainMenu->cpm->profiles, p_topMid );
	bHoldThresh = 30;


	if (parent == NULL)
	{
		active = true;
	}
	holdingB = false;
	FillRingSection *blah[] = { new FillRingSection(mainMenu->tilesetManager, Color::Red, sf::Color::Black,
		sf::Color::Blue,
		3, 40, 0) };

	backLoaderOffset = Vector2f(0, 50);
	backLoader = new FillRing(topMid + backLoaderOffset, 1, blah);
	backLoader->ResetEmpty();

	Tileset *ts_bg = mainMenu->tilesetManager.GetTileset( "Menu/multiprofile.png", 480, 400);
	bgSprite.setTexture(*ts_bg->texture);
	bgSprite.setTextureRect(ts_bg->GetSubRect( p_playerIndex ));
	bgSprite.setOrigin(bgSprite.getLocalBounds().width / 2, 0);
	bgSprite.setPosition(topMid);

	offRect.setSize(Vector2f( ts_bg->tileWidth, ts_bg->tileHeight ));
	offRect.setFillColor(Color(0, 0, 0, 180));
	offRect.setOrigin(offRect.getLocalBounds().width / 2, 0);
	offRect.setPosition(topMid);
	//profileSelect->UpdateNames();	
}

bool MultiSelectionSection::IsReady()
{
	if (profileSelect->state == ControlProfileMenu::S_SELECTED)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void MultiSelectionSection::SetTopMid( sf::Vector2f &tm )
{
	topMid = tm;
	backLoader->SetPosition(topMid + backLoaderOffset);
	profileSelect->SetTopMid(topMid);
	bgSprite.setPosition(topMid);
	offRect.setPosition(topMid);
}

bool MultiSelectionSection::ButtonEvent( UIEvent eType,
		ButtonEventParams *param )
{
	return true;
}

void MultiSelectionSection::Update()
{
	MainMenu *mm = mainMenu;
	ControllerState &currInput = mm->GetCurrInput( playerIndex );
	ControllerState &prevInput = mm->GetPrevInput( playerIndex );

	if( !active )
	{
		bool a = currInput.A && !prevInput.A;
		if (a)
		{
			active = true;
			bHoldFrames = 0;
		}
		return;
	}
	else
	{
		if (profileSelect->state == ControlProfileMenu::S_SELECTED)
		{
			if (currInput.B)
			{
				holdingB = true;
			}
			else
			{
				holdingB = false;
				bHoldFrames = 0;
			}

			if (holdingB)
			{
				if (backLoader->Fill(1) > 0)
				{

				}
				backLoader->Update();
			}
			else
			{
				backLoader->Drain(1);
				backLoader->Update();
			}
		}
		else
		{
			holdingB = false;
		}
	}

	//if (!IsReady())
	{
		bool rT = (currInput.RightTriggerPressed() && !prevInput.RightTriggerPressed());
		bool lT = (currInput.LeftTriggerPressed() && !prevInput.LeftTriggerPressed());

		bool rS = (currInput.rightShoulder && !prevInput.rightShoulder);
		bool lS = (currInput.leftShoulder && !prevInput.leftShoulder);
		if (rT)
		{
			switch (team)
			{
			case T_NOT_CHOSEN:
			{
				team = T_RED;
				break;
			}
			case T_RED:
			{
				team = T_BLUE;
				break;
			}
			case T_BLUE:
			{
				team = T_RED;
				break;
			}
			}
		}
		else if (lT)
		{
			switch (team)
			{
			case T_NOT_CHOSEN:
			{
				team = T_BLUE;
				break;
			}
			case T_RED:
			{
				team = T_BLUE;
				break;
			}
			case T_BLUE:
			{
				team = T_RED;
				break;
			}
			}
		}

		if (rS)
		{
			if (skinIndex < S_Count)
			{
				++skinIndex;
			}
			else
			{
				skinIndex = 0;
			}
		}
		else if (lS)
		{
			if (skinIndex > 0)
			{
				--skinIndex;
			}
			else
			{
				skinIndex = S_Count - 1;
			}
		}

		ControlProfile *oldProf = profileSelect->currProfile;

		profileSelect->Update(currInput, prevInput);

		/*if (profileSelect->currProfile != oldProf)
		{
			selectedProfileText.setString(profileSelect->currProfile->name);
			selectedProfileText.setOrigin(selectedProfileText.getLocalBounds().width / 2, 0);
		}*/
	}

	int numPlayersActive = 0;
	if (parent != NULL)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (parent->multiPlayerSection[i]->active)
			{
				++numPlayersActive;
			}
		}
		//numPlayersActive  = parent->GetNumActivePlayers();
	}
	else
	{
		numPlayersActive = 1;
	}
}

void MultiSelectionSection::Draw( sf::RenderTarget *target )
{
	
	//if (parent != NULL)
	/*{
		Color c;
		switch (playerIndex)
		{
		case 0:
			c = Color::Red;
			break;
		case 1:
			c = Color::Yellow;
			break;
		case 2:
			c = Color::White;
			break;
		case 3:
			c = Color::Magenta;
			break;
		}

		sf::RectangleShape rs;
		rs.setFillColor(c);
		rs.setSize(Vector2f(1920 / 4, 400));
		rs.setOrigin(rs.getLocalBounds().width / 2, 0);
		rs.setPosition(profileSelect->topMid);
		target->draw(rs);
	}*/

	
	target->draw(bgSprite);
	target->draw( playerSprite );
	profileSelect->Draw( target );

	if (holdingB)
	{
		backLoader->Draw(target);
	}

	if( !active )
		target->draw(offRect);
}

bool MultiSelectionSection::ShouldGoBack()
{
	if (backLoader->IsFull())
	{
		backLoader->ResetEmpty();
		return true;
	}
	
	return false;
}

MultiLoadingScreen::MultiLoadingScreen( MainMenu *p_mainMenu )
	:mainMenu( p_mainMenu ), loadThread( NULL )
{
	
	int quarter = 1920 / 4;
	menuOffset = Vector2f(0, 0);
	for( int i = 0; i < 4; ++i )
	{
		Vector2f topMid = Vector2f(quarter * i + quarter / 2, 1080 - 400) + menuOffset;
		//playerSection[i] = new MultiSelectionSection( mainMenu, this, i, topMid);
	}

	progressDisplay = new LoadingMapProgressDisplay(mainMenu, menuOffset + Vector2f(480, 540));
	//progressDisplay->SetProgressString("hello here i am");
	//progressDisplay->SetProgressString("hello here i am", 2);

	gs = NULL;
	loadThread = NULL;	
}

void MultiLoadingScreen::Reset( boost::filesystem::path p_path )
{
	filePath = p_path;

	previewSprite.setTexture(*mainMenu->mapSelectionMenu->previewSprite.getTexture());
	previewSprite.setPosition(Vector2f(400, 0) + menuOffset);
	//SetPreview();
	
	if( gs != NULL )
	{
		delete gs;
	}
	else if( loadThread != NULL )
	{
		delete loadThread;
	}

	gs = new GameSession( NULL, mainMenu, p_path.string() );

	gs->progressDisplay = progressDisplay;

	loadThread = new boost::thread( GameSession::sLoad, gs );

	for (int i = 0; i < 4; ++i)
	{
		playerSection[i]->active = false;
	}
}

//void MultiLoadingScreen::SetPreview()
//{
//	string previewFile = filePath.parent_path().relative_path().string() + string( "/" ) + filePath.stem().string() + string( "_preview_960x540.png" );
//	previewTex.loadFromFile( previewFile );
//	previewSprite.setTexture( previewTex );
//	previewSprite.setPosition( Vector2f( 480, 0 ) + menuOffset );
//}

void MultiLoadingScreen::Draw( sf::RenderTarget *target )
{
	target->draw( previewSprite );
	for( int i = 0; i < 4; ++i )
	{
		playerSection[i]->Draw( target );
	}
	progressDisplay->Draw(target);
}

int MultiLoadingScreen::GetNumActivePlayers()
{
	int activeCount = 0;
	for( int i = 0; i < 4; ++i )
	{
		if( playerSection[i]->active )
		{
			activeCount++;
		}
	}

	return activeCount;
}

bool MultiLoadingScreen::AllPlayersReady()
{
	int readyCount = 0;
	for( int i = 0; i < 4; ++i )
	{
		if( playerSection[i]->active )
		{
			if( playerSection[i]->IsReady() )
			{
				readyCount++;
			}
			else
			{
				return false;
			}
		}
	}

	return readyCount > 0;
}

void MultiLoadingScreen::Update()
{
	progressDisplay->UpdateText();

	for (int i = 0; i < 4; ++i)
	{
		if (playerSection[i]->ShouldGoBack())
		{
			mainMenu->SetMode(MainMenu::Mode::TRANS_MULTIPREVIEW_TO_MAPSELECT);

			gs->SetContinueLoading(false);
			loadThread->join();
			delete loadThread;
			loadThread = NULL;
			delete gs;
			gs = NULL;
			return;
		}
		if (!playerSection[i]->active)
		{
			if (mainMenu->GetCurrInput(i).B && !mainMenu->GetPrevInput(i).B)
			{
				
			}
		}
	}

	for( int i = 0; i < 4; ++i )
	{
		playerSection[i]->Update();
	}

	if( AllPlayersReady() )
	{
		//loadThread->join();
		//boost::chrono::steady_clock::now()
		if( loadThread->try_join_for( boost::chrono::milliseconds( 0 ) ) )
		{
			for( int i = 0; i < 4; ++i )
			{
				mainMenu->GetController( i ).SetFilter( playerSection[i]->profileSelect->currProfile->filter );
			}

			int res = gs->Run();

			XBoxButton filter[ControllerSettings::Count];
			SetFilterDefault( filter );

			for( int i = 0; i < 4; ++i )
			{
				mainMenu->GetController( i ).SetFilter( filter );
			}

			delete loadThread;
			loadThread = NULL;
			delete gs;
			gs = NULL;

			View vv;
			vv.setCenter(960, 540);
			vv.setSize(1920, 1080);
			mainMenu->window->setView(vv);

			mainMenu->v.setCenter(mainMenu->leftCenter);
			mainMenu->v.setSize(Vector2f(1920, 1080));
			mainMenu->preScreenTexture->setView(mainMenu->v);

			mainMenu->SetMode(MainMenu::Mode::MAPSELECT);
			mainMenu->mapSelectionMenu->state = MapSelectionMenu::State::S_MAP_SELECTOR;
			mainMenu->v.setCenter(mainMenu->leftCenter);
			mainMenu->preScreenTexture->setView(mainMenu->v);
		}	
	}
}

GameController &MainMenu::GetController( int index )
{
	return *controllers[index];
}

void MainMenu::UpdateMenuOptionText()
{
	int breatheFrames = 60;
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

	SetRectColor(mainMenuOptionHighlight + saSelector->currIndex * 4, Color( 255, 255, 255, alpha * 255 ));

	//for (int i = 0; i < MainMenuOptions::M_Count; ++i)
	//{
	//	if (saSelector->currIndex == i)
	//	{
	//		SetRectSubRect( mainMenuOptionQuads + i * 4, ts_mainOption->GetSubRect(i + 7) );
	//		//menuOptions[i].setFillColor(Color::Red);
	//	}
	//	else
	//	{
	//		SetRectSubRect(mainMenuOptionQuads + i * 4, ts_mainOption->GetSubRect(i));
	//	}
	//}
}

MainMenu::MainMenu()
	:windowWidth(1920), windowHeight(1080)
{
	arial.loadFromFile("Breakneck_Font_01.ttf");

	transLength = 60;
	transFrame = 0;

	cpm = new ControlProfileManager;
	cpm->LoadProfiles();

	Shard::SetupShardMaps();
	//MusicManager mm(this);
	musicManager = new MusicManager(this);
	musicManager->LoadMusicNames();

	pauseMenu = new PauseMenu(this);

	titleScreen = new TitleScreen(this);
	
	ts_mainOption = tilesetManager.GetTileset("Menu/mainmenu_text_512x64.png", 512, 64);

	for (int i = 0; i < MainMenuOptions::M_Count; ++i)
	{
		//if (saSelector->currIndex == i)
		//{
		//	SetRectSubRect(mainMenuOptionQuads + i * 4, ts_mainOption->GetSubRect(i + 7));
		//	//menuOptions[i].setFillColor(Color::Red);
		//}
		//else
		{
			SetRectSubRect(mainMenuOptionQuads + i * 4, ts_mainOption->GetSubRect(i));
			SetRectSubRect(mainMenuOptionHighlight + i * 4, ts_mainOption->GetSubRect(i+7));
		}
	}

	introMovie = new IntroMovie;
	
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


	for (int i = 0; i < 4; ++i)
	{
		controllers[i] = new GameController(i);
	}

	fadeRect.setFillColor(Color::Black);
	fadeRect.setSize(Vector2f(1920, 1080));
	fadeRect.setPosition(0, 0);

	config = new Config();
	config->WaitForLoad();
	windowWidth = config->GetData().resolutionX;
	windowHeight = config->GetData().resolutionY;

	cout << "start mm constfr" << endl;
	//load a preferences file at some point for window resolution and stuff

	//could be bad cuz its static
	

	uiView = View(sf::Vector2f(960, 540), sf::Vector2f(1920, 1080));
	v = View(Vector2f(1920 / 2, 1080 / 2), Vector2f(1920, 1080));


	splashFadeFrame = 0;
	splashFadeOutLength = 40;

	sf::Text t;
	t.setFont(arial);
	t.setCharacterSize(40);
	t.setFillColor(Color::White);
	

	int waitFrames[] = { 10, 5, 2 };
	int waitModeThresh[] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 7, 0);

	Vector2f textBase(100, 300);
	int textOptionSpacing = 6;
	int charHeight = 40;

	

	for (int i = 0; i < MainMenuOptions::M_Count; ++i)
	{
		menuOptions[i].setFont(arial);
		menuOptions[i].setCharacterSize(charHeight);
		menuOptions[i].setFillColor(Color::White);
	}


	string optionStrings[] = { "Adventure", "Free Play", "Local Multiplayer", "Level Editor",
		"Options", "Credits", "Exit" };

	

	for (int i = 0; i < MainMenuOptions::M_Count; ++i)
	{
		menuOptions[i].setString(optionStrings[i]);
		menuOptions[i].setOrigin(menuOptions[i].getLocalBounds().width / 2, 0);
		menuOptions[i].setPosition(textBase.x, textBase.y + (textOptionSpacing + charHeight) * i);

		SetRectCenter(mainMenuOptionQuads + i * 4, 512, 64, textBase + Vector2f(256, 32 + i * (64 + textOptionSpacing) ) );
		SetRectCenter(mainMenuOptionHighlight + i * 4, 512, 64, textBase + Vector2f(256, 32 + i * (64 + textOptionSpacing)));
		
		//SetRectSubRect(mainMenuOptionQuads + i * 4, ts_mainOption->GetSubRect(0));
	}

	menuOptionsBG.setFillColor(Color( 0, 0, 0, 70 ));
	menuOptionsBG.setPosition(textBase);
	menuOptionsBG.setSize(Vector2f(512, (64 + textOptionSpacing) * 7));

	soundNodeList = new SoundNodeList( 10 );
	soundNodeList->SetGlobalVolume( config->GetData().volume );

	//menuMode = SPLASH;
	//menuMode = MAPSELECT;

	if( preScreenTexture == NULL )
	{
		preScreenTexture = new RenderTexture;
		preScreenTexture->create( 1920, 1080 );
		preScreenTexture->clear();
	}

	if (lastFrameTexture == NULL)
	{
		lastFrameTexture = new RenderTexture;
		lastFrameTexture->create(1920, 1080);
		lastFrameTexture->clear();
	}

	if ( extraScreenTexture == NULL)
	{
		extraScreenTexture = new RenderTexture;
		extraScreenTexture->create(1920, 1080);
		extraScreenTexture->clear();
	}

	if( postProcessTexture == NULL )
	{
		postProcessTexture = new RenderTexture;
		postProcessTexture->create( 1920/2, 1080/2 );
		postProcessTexture->clear();
	}
	
	if( postProcessTexture1 == NULL )
	{
		postProcessTexture1 = new RenderTexture;
		postProcessTexture1->create( 1920/2, 1080/2 );
		postProcessTexture1->clear();
	}
	
	if( postProcessTexture2 == NULL )
	{
		postProcessTexture2 = new RenderTexture;
		postProcessTexture2->create( 1920, 1080 );
		postProcessTexture2->clear();
	}

	if( minimapTexture == NULL )
	{
		minimapTexture = new RenderTexture;
		//minimapTexture->setSmooth( true );
		minimapTexture->create( 500, 500 );
		minimapTexture->clear();
	}

	if( mapTexture == NULL )
	{
		mapTexture = new RenderTexture;
		mapTexture->create( 1720, 880 );
		mapTexture->clear();
	}

	if( pauseTexture == NULL )
	{
		pauseTexture = new RenderTexture;
		pauseTexture->create( 1820, 980 );
		pauseTexture->clear();
	}

	if( saveTexture == NULL )
	{
		saveTexture= new RenderTexture;
		saveTexture->create( 1920, 1080 );
		saveTexture->clear();
	}

	if( mapPreviewTexture == NULL )
	{
		mapPreviewTexture= new RenderTexture;
		mapPreviewTexture->create( 960-48, 540-48 );
		mapPreviewTexture->clear();
	}

	if (auraCheckTexture == NULL)
	{
		auraCheckTexture = new RenderTexture;
		auraCheckTexture->create(512, 512);
		auraCheckTexture->clear();
	}

	transWorldMapFrame = 0;
	

	titleScreen->Draw(preScreenTexture);

	worldMap = new WorldMap( this );

	

	levelSelector = new LevelSelector( this );

	

	kinTitleSprite.setPosition( 512, 1080 );
	
	kinTitleSpriteFrame = 0;
	kinTotalFrames = 76 * 2 + 50;

	currentMenuSelect = 0;

	bool fullWindow = true;

	
	
	
	/*if( sf::Keyboard::isKeyPressed( Keyboard::W ) )
	{
		fullWindow = false;
		windowWidth /= 2;
		windowHeight /= 2;
	}*/
	window = NULL;

	window = new RenderWindow( sf::VideoMode( windowWidth, windowHeight ), "Breakneck",
		config->GetData().windowStyle, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	cout << "start mm constr 222" << endl;
	//if( fullWindow )
	//{
	//	window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
	//		sf::VideoMode( windowWidth, windowHeight ), "Breakneck", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	//	window->setPosition( Vector2i(800, 0 ));

	//}
	//else
	//{
	//	cout << "blah0" << endl;
	//	std::vector<sf::VideoMode> i = sf::VideoMode::getFullscreenModes();
	//	VideoMode vm( 1600, 900 );
 //       //sf::RenderWindow window(i.front(), "SFML WORKS!", sf::Style::Fullscreen);
	//	//window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
	//	//	sf::VideoMode( 1920 / 1, 1079 / 1), "Breakneck", sf::Style::Fullscreen, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	//	//window = new sf::RenderWindow( VideoMode( 1920, 1080 ), "Breakneck", sf::Style::None);
	//	style = sf::Style::None;
	//	assert( i.size() != 0 );
	//	VideoMode vidMode = i.front();

	//	cout << "creating window: " << (int)vidMode.isValid() << endl;
	//	window = new sf::RenderWindow( vidMode, "Breakneck", style);
	//	cout << "blah1" << endl;
	//	//window = new sf::RenderWindow( vm, "Breakneck", sf::Style::None);

	//		//sf::VideoMode( 1920 / 1, 1080 / 1), "Breakneck", sf::Style::Fullscreen, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	//}

	assert( window != NULL );
	window->setVerticalSyncEnabled( true );
	std::cout << "opened window" << endl;
	
	window->setView( v );

	betaText.setString( " Press any button to start \n For help and info check \n - beta_info.txt\n Breakneck Beta\n Updated 9/2/2016");
	betaText.setCharacterSize( 20 );
	betaText.setFillColor( Color::Red );
	betaText.setPosition( 50, 200 );
	betaText.setFont( arial );

	slideCurrFrame = 0;
	numSlideFrames = 60;

	Init();

	

	multiLoadingScreen = new MultiLoadingScreen( this );

	mapSelectionMenu = new MapSelectionMenu(this, Vector2f(0, 100));

	optionsMenu = new OptionsMenuScreen(this);

	creditsMenu = new CreditsMenuScreen(this);

	saveMenu = new SaveMenuScreen(this);

	worldMap->testSelector->UpdateAllInfo();

	worldMap->Draw(preScreenTexture);
	saveMenu->Draw(preScreenTexture);

	FillRingSection *blah[] = { new FillRingSection(tilesetManager, Color::Red, sf::Color::Black,
		sf::Color::Blue,
		3, 300, 0) };

	Tileset *ts_loadIcon = tilesetManager.GetTileset("Menu/loadicon_320x320.png", 320, 320);

	for (int i = 0; i < 3; ++i)
	{
		loadingIconBackpack[i].setTexture(*ts_loadIcon->texture);
		loadingIconBackpack[i].setTextureRect(ts_loadIcon->GetSubRect(i));
		loadingIconBackpack[i].setOrigin(loadingIconBackpack[i].getLocalBounds().width / 2, loadingIconBackpack[i].getLocalBounds().height / 2);
		loadingIconBackpack[i].setPosition(1920 - 260, 1080 - 200);
	}

	Tileset *ts_loadBG = tilesetManager.GetTileset("Menu/load_w1_1.png", 1920, 1080);
	loadingBGSpr.setTexture(*ts_loadBG->texture);
	//testRing = new FillRing( Vector2f( 200, 200 ), 1, blah);
}

SaveFile *MainMenu::GetCurrentProgress()
{
	return saveMenu->files[saveMenu->selectedSaveIndex];
}

MainMenu::~MainMenu()
{
	window->close();

	delete config;

	delete window;

	delete pauseMenu;
	
	delete preScreenTexture;
	delete lastFrameTexture;
	delete postProcessTexture;
	delete postProcessTexture1;
	delete postProcessTexture2;
	delete minimapTexture;
	delete mapTexture;

	delete soundNodeList;
}

void MainMenu::Init()
{	
	ts_splashScreen = tilesetManager.GetTileset( "Menu/splashscreen_1920x1080.png", 1920, 1080 );
	splashSprite.setTexture( *ts_splashScreen->texture );

	/*ts_kinTitle[0] = tilesetManager.GetTileset( "Title/kin_title_1_1216x1080.png", 1216, 1080 );
	ts_kinTitle[1] = tilesetManager.GetTileset( "Title/kin_title_2_1216x1080.png", 1216, 1080 );
	ts_kinTitle[2] = tilesetManager.GetTileset( "Title/kin_title_3_1216x1080.png", 1216, 1080 );
	ts_kinTitle[3] = tilesetManager.GetTileset( "Title/kin_title_4_1216x1080.png", 1216, 1080 );
	ts_kinTitle[4] = tilesetManager.GetTileset( "Title/kin_title_5_1216x1080.png", 1216, 1080 );
	ts_kinTitle[5] = tilesetManager.GetTileset( "Title/kin_title_6_1216x1080.png", 1216, 1080 );
	ts_kinTitle[6] = tilesetManager.GetTileset( "Title/kin_title_7_1216x1080.png", 1216, 1080 );*/
	
	

	soundBuffers[S_DOWN] = soundManager.GetSound( "menu_down.ogg" );
	soundBuffers[S_UP] = soundManager.GetSound( "menu_up.ogg" );
	soundBuffers[S_SELECT] = soundManager.GetSound( "menu_select.ogg" );

	

	cout << "init finished" << endl;

	//parBack = new Parallax();
	//parFront = new Parallax();
	//parBack->AddRepeatingSprite( ts_asteroid1, 0, Vector2f( -1920, 0 ), 1920 * 3, 50 );
	//parBack->AddRepeatingSprite( ts_asteroid1, 0, Vector2f( 1920, 0 ), 1920 * 2, 50 );
	//parFront->AddRepeatingSprite( ts_asteroid0, 0, Vector2f( 0, 0 ), 1920, 50 );
	//parFront->AddRepeatingSprite( ts_asteroid2, 0, Vector2f( 0, 0 ), 1920, 50 );

	//asteroidFrame = 0;
	
}

void MainMenu::GameEditLoop( const std::string &p_path )
{
	int result = 0;

	Vector2f lastViewSize( 0, 0 );
	Vector2f lastViewCenter( 0, 0 );
	while( result == 0 )
	{
		EditSession *es = new EditSession(this);
		//boost::filesystem::path( p_path );
		result = es->Run( p_path, lastViewCenter, lastViewSize );
		delete es;
		if( result > 0 )
			break;

		//v.setSize( 1920, 1080 );
		window->setView( v );
		GameSession *gs = new GameSession( NULL, this, p_path );
		GameSession::sLoad(gs);
		
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
		window->setView( v );
		GameSession *gs = new GameSession( NULL, this, p_path );
		GameSession::sLoad(gs);
		result = gs->Run();
		lastViewCenter = gs->lastViewCenter;
		lastViewSize = gs->lastViewSize;
		delete gs;
		if( result > 0 )
			break;

		EditSession *es = new EditSession(this );
		result = es->Run( p_path, lastViewCenter, lastViewSize );
		delete es;
	}
}

void MainMenu::SetMode(Mode m)
{
	switch (m)
	{
	case TRANS_OPTIONS_TO_MAIN:
	{
		slideCurrFrame = 0;
		slideStart = topCenter;
		slideEnd = trueCenter;
		break;
	}
	case TRANS_MAIN_TO_OPTIONS:
	{
		slideCurrFrame = 0;
		slideStart = trueCenter;
		slideEnd = topCenter;
		break;
	}
	case TRANS_MAIN_TO_SAVE:
	{
		transFrame = 0;
		transLength = 60;
		saveMenu->Reset();
		saveMenu->Update();
		slideCurrFrame = 0;
		slideStart = trueCenter;
		slideEnd = rightCenter;
		break;
	}
	case TRANS_SAVE_TO_MAIN:
	{
		transLength = 60;
		transFrame = 0;
		slideCurrFrame = 0;
		slideStart = rightCenter;
		slideEnd = trueCenter;
		break;
	}
	case TRANS_MAIN_TO_CREDITS:
	{
		slideCurrFrame = 0;
		slideStart = trueCenter;
		slideEnd = bottomCenter;
		break;
	}
	case TRANS_CREDITS_TO_MAIN:
	{
		slideCurrFrame = 0;
		slideStart = bottomCenter;
		slideEnd = trueCenter;
		break;
	}
	case TRANS_MAIN_TO_MAPSELECT:
	{
		slideCurrFrame = 0;
		slideStart = trueCenter;
		slideEnd = leftCenter;

		break;
	}
	case TRANS_MAPSELECT_TO_MAIN:
	{
		slideCurrFrame = 0;
		slideStart = leftCenter;
		slideEnd = trueCenter;
		break;
	}
	case TRANS_MAPSELECT_TO_MULTIPREVIEW:
	{
		slideCurrFrame = 0;
		slideStart = leftCenter;
		slideEnd = doubleLeftCenter;
		break;
	}
	case TRANS_MULTIPREVIEW_TO_MAPSELECT:
	{
		slideCurrFrame = 0;
		slideStart = doubleLeftCenter;
		slideEnd = leftCenter;
		break;
	}
	}

	menuMode = m;
}

void MainMenu::DrawMenuOptionText(sf::RenderTarget *target)
{
	target->draw(menuOptionsBG);
	for (int i = 0; i < MainMenuOptions::M_Count; ++i)
	{
		target->draw(mainMenuOptionQuads, 7 * 4, sf::Quads, ts_mainOption->texture);
		//preScreenTexsetture->draw(menuOptions[i]);
	}

	target->draw(mainMenuOptionHighlight + saSelector->currIndex * 4, 4, sf::Quads,
		ts_mainOption->texture);
}

void MainMenu::CustomMapsOption()
{
	LevelSelector &ls = *levelSelector;
	sf::Event ev;
	window->setView( uiView );
	bool quit = false;

	CustomMapsHandler customMapHandler( this );

	Panel p( "custom maps", 1920 - ls.width, 1080, &customMapHandler );
	p.pos.x += ls.width;
	p.AddButton( "Play", Vector2i( 100, 0 ), Vector2f( 100, 50 ), "PLAY" );
	p.AddButton( "Edit", Vector2i( 100, 100 ), Vector2f( 100, 50 ), "EDIT" );
	p.AddButton( "Create New", Vector2i( 100, 200 ), Vector2f( 175, 50 ), "CREATE NEW" );
	p.AddButton( "Delete", Vector2i( 900, 300 ), Vector2f( 150, 50 ), "DELETE" );

	Panel namePopup( "name popup", 300, 200, &customMapHandler );
	namePopup.pos = Vector2i( 960, 540 );
	namePopup.AddButton( "ok", Vector2i( 100, 0 ), Vector2f( 100, 50 ), "OK" );	
	namePopup.AddTextBox( "name", Vector2i( 10, 10 ), 100, 40, "test" );
	//bool showNamePopup = false;

	ls.UpdateMapList();

	string empty = "5\n0 0\nmat\n5\n-209 78\n286 78\n286 132\n60 132\n-201 132\n0\n0\n0\n1\n-- 1\ngoal -air 0 0 76";

	while( !quit )
	{
		window->clear();
		
		
		Vector2i mousePos = sf::Mouse::getPosition( *window );
		//mousePos /= 2;
		

		//window.setView( uiView );
		Vector2f uiMouse = window->mapPixelToCoords( mousePos );

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
						return;
					}
					else
					{
						if( customMapHandler.showNamePopup )
						{
							//ls.newLevelName = "";
							ls.newLevelName = "";
							namePopup.SendKey( ev.key.code, ev.key.shift );
							if( ls.newLevelName != "" )
							{
								
								if( ls.text[ls.selectedIndex].getFillColor() == Color::Red )
								{
									path from( "Maps/empty.brknk" );

									std::stringstream ssPath;
									ssPath << ls.localPaths[ls.selectedIndex] << ls.newLevelName << ".brknk";
									string toString = ssPath.str();
									path to( toString );

									try 
									{
										boost::filesystem::copy_file( from, to, copy_option::fail_if_exists );

										//GameEditLoop( ls.GetSelectedPath() );

										ls.UpdateMapList();

										window->setView( uiView );
									}
									catch (const boost::system::system_error &err) 
									{
										cout << "file already exists!" << endl;
									}
								}
								else
								{
									path from( ls.GetSelectedPath() );
									TreeNode * toNode = ls.dirNode[ls.selectedIndex];

									std::stringstream ssPath;
									ssPath << toNode->GetLocalPath() << ls.newLevelName << ".brknk";
									string toString = ssPath.str();

									path to( toString );
									
									try 
									{
										boost::filesystem::copy_file( from, to, copy_option::fail_if_exists );

										//cout << "copying to: " << to.string() << endl;
										//GameEditLoop( toNode->GetLocalPath() );

										ls.UpdateMapList();

										window->setView( uiView );
									}
									catch (const boost::system::system_error &err) 
									{
										cout << "file already exists!" << endl;
									}
								}
								
								
							}
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
							namePopup.Update( true, uiMouse.x, uiMouse.y);
						}
						else
						{
							p.Update( true, uiMouse.x, uiMouse.y);
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
						if( customMapHandler.showNamePopup )
						{
							ls.newLevelName = "";
							namePopup.Update( false, uiMouse.x, uiMouse.y );
							if( ls.newLevelName != "" )
							{
								
								if( ls.text[ls.selectedIndex].getFillColor() == Color::Blue )
								{
									path from( "Maps/empty.brknk" );
									string toString = ls.localPaths[ls.selectedIndex] + ls.newLevelName + ".brknk";
									path to( toString );

									try 
									{
										boost::filesystem::copy_file( from, to, copy_option::fail_if_exists );

										GameEditLoop( ls.GetSelectedPath() );

										ls.UpdateMapList();

										window->setView( uiView );
									}
									catch (const boost::system::system_error &err) 
									{
										cout << "file already exists!" << endl;
									}
								}
								else
								{
									path from( ls.localPaths[ls.selectedIndex] );
									TreeNode * toNode = ls.dirNode[ls.selectedIndex];
									path to( toNode->GetLocalPath() + ls.newLevelName + ".brknk" );

									try 
									{
										boost::filesystem::copy_file( from, to, copy_option::fail_if_exists );

										GameEditLoop( to.string() );

										ls.UpdateMapList();

										window->setView( uiView );
									}
									catch (const boost::system::system_error &err) 
									{
										cout << "file already exists!" << endl;
									}
								}
								
								
							}
						}
						else
						{
							p.Update( false, uiMouse.x, uiMouse.y );
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
			}
		}

		p.Draw( window );
		ls.Draw( window );

		if( customMapHandler.showNamePopup )
		{
			namePopup.Draw( window );
		}

		window->display();
		
		
	}
	window->setView( v );
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

	bool quit = false;

	bool worldMapUpdate = false;
	double currentTime = 0;
	double accumulator = TIMESTEP + .1;

	

#if defined( USE_MOVIE_TEST )
	sfe::Movie m;
	assert( m.openFromFile("Movie/testvid.ogv") );
	m.fit(sf::FloatRect(0, 0, 1920, 1080));

	
	m.play();
#endif
	/*int moveDownFrames = 0;
	int moveUpFrames = 0;
	int moveLeftFrames = 0;
	int moveRightFrames = 0;*/
	

	gameClock.restart();

	//saveTexture->setView( v );
	//menuMode = MAPSELECT;//menuMode = SPLASH;//DEBUG_RACEFIGHT_RESULTS;
	//menuMode = OPTIONS;

	//SetMode(TRANS_MAIN_TO_OPTIONS);
	//SetMode(TRANS_MAIN_TO_SAVE);
	//SetMode(TRANS_MAIN_TO_CREDITS);
	//SetMode(TRANS_MAIN_TO_MAPSELECT);
	//SetMode(SPLASH);
	SetMode(MAINMENU);
	//SetMode(OPTIONS);
	//menuMode = MainMenu::Mode::TRANS_MAIN_TO_OPTIONS;//MainMenu::Mode::MULTIPREVIEW;
#if defined( USE_MOVIE_TEST )
	sf::Shader sh;
	assert( sh.loadFromFile("Shader/test.frag", sf::Shader::Fragment ) );
	
	sf::Vertex ff[4] = {
		sf::Vertex( Vector2f( 0, 0 ) ),
		sf::Vertex(Vector2f(500, 0)),
		sf::Vertex(Vector2f(500, 500)),
		sf::Vertex(Vector2f(0, 500))
	};
#endif
	while( !quit )
	{
		double newTime = gameClock.getElapsedTime().asSeconds();
		double frameTime = newTime - currentTime;

		if ( frameTime > 0.25 )
		{
			frameTime = 0.25;	
		}
        currentTime = newTime;

		accumulator += frameTime;

		preScreenTexture->clear();
		window->clear();
		
		
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Srcreen And Depth Buffer

		while ( accumulator >= TIMESTEP  )
        {
#if defined( USE_MOVIE_TEST )
			if (m.getStatus() == sfe::Status::Stopped)
			{
				m.setPlayingOffset(sf::Time::Zero);
				m.play();
			}

			m.update();
			const sf::Texture &currImage = m.getCurrentImage();

			sh.setUniform("texture", currImage);
#endif
			worldMapUpdate = false;

			menuPrevInput = menuCurrInput;
			menuCurrInput.Set( ControllerState() );

			//int upCount = 0;
			//int downCount = 0;

			for( int i = 0; i < 4; ++i )
			{
				ControllerState &prevInput = GetPrevInputUnfiltered( i );
				ControllerState &currInput = GetCurrInputUnfiltered( i );
				GameController &c = GetController( i );

				prevInput = currInput;
				bool active = c.UpdateState();

				if( active )
				{
					currInput = c.GetState();

					menuCurrInput.A |= ( currInput.A && !prevInput.A );
					menuCurrInput.B |= ( currInput.B && !prevInput.B );
					menuCurrInput.X |= ( currInput.X && !prevInput.X );
					menuCurrInput.Y |= ( currInput.Y && !prevInput.Y );
					menuCurrInput.rightShoulder |= ( currInput.rightShoulder && !prevInput.rightShoulder );
					menuCurrInput.leftShoulder |= ( currInput.leftShoulder && !prevInput.leftShoulder );
					menuCurrInput.start |= ( currInput.start && !prevInput.start );
					menuCurrInput.leftTrigger = max(menuCurrInput.leftTrigger, currInput.leftTrigger);
					menuCurrInput.rightTrigger= max(menuCurrInput.rightTrigger, currInput.rightTrigger);
					menuCurrInput.back |= ( currInput.back && !prevInput.back );
					menuCurrInput.leftStickPad |= currInput.leftStickPad;
				}
				else
				{
					currInput.Set( ControllerState() );
				}

			}
			

			switch( menuMode )
		{
			case DEBUG_RACEFIGHT_RESULTS:
				{
				GameSession *gs = new GameSession( NULL, this, "Maps/W1/arena04.brknk" );
				GameSession::sLoad(gs);
				gs->Run();
				return;
				break;
				}
			case SPLASH:
				{
					bool A = menuCurrInput.A && !menuPrevInput.A;
					bool B = menuCurrInput.B && !menuPrevInput.B;
					bool X = menuCurrInput.X && !menuPrevInput.X;
					bool Y = menuCurrInput.Y && !menuPrevInput.Y;
					bool r = menuCurrInput.rightShoulder && !menuPrevInput.rightShoulder;
					bool l = menuCurrInput.leftShoulder && !menuPrevInput.leftShoulder;

					if( A || B || X || Y || r || l )
					{
						menuMode = SPLASH_TRANS;
						splashFadeFrame = 0;

						sf::Color fadeColor = fadeRect.getFillColor();
						fadeRect.setFillColor( Color( fadeColor.r, fadeColor.g, 
							fadeColor.b, 
							(((double)splashFadeFrame) / splashFadeOutLength) * 255 ) );
					}
					break;
				}
			case SPLASH_TRANS:
				{
					if( splashFadeFrame > splashFadeOutLength )
					{
						menuMode = MAINMENU;
						splashFadeFrame = 0;

						sf::Color fadeColor = fadeRect.getFillColor();
						fadeRect.setFillColor( Color( fadeColor.r, fadeColor.g, 
							fadeColor.b, 255 - (((double)splashFadeFrame) / splashFadeOutLength) * 255 ) );
					}
					else
					{
						sf::Color fadeColor = fadeRect.getFillColor();
						fadeRect.setFillColor( Color( fadeColor.r, fadeColor.g, 
							fadeColor.b, 
							(((double)splashFadeFrame) / splashFadeOutLength) * 255 ) );
						++splashFadeFrame;
					}
					break;
				}
			case MAINMENU:
				{
				titleScreen->Update();
					if( splashFadeFrame <= splashFadeOutLength )
					{
						sf::Color fadeColor = fadeRect.getFillColor();
						fadeRect.setFillColor( Color( fadeColor.r, fadeColor.g, 
							fadeColor.b, 255 - (((double)splashFadeFrame) / splashFadeOutLength) * 255 ) );
						++splashFadeFrame;
					}


					while( window->pollEvent( ev ) )
					{
						switch( ev.type )
				{
				case sf::Event::KeyPressed:
					{
						/*if( ev.key.code == Keyboard::Num1 )
						{
							cout << "starting level 1" << endl;
							titleMusic.stop();
							GameEditLoop2( "test1" );
							window->setView( v );
							titleMusic.play();
						}
						else if( ev.key.code == Keyboard::Num2 )
						{
							cout << "starting level 2" << endl;
							titleMusic.stop();
							GameEditLoop2( "test2" );
							window->setView( v );
							titleMusic.play();
						}
						else if( ev.key.code == Keyboard::Num3 )
						{
							cout << "starting level 3" << endl;
							titleMusic.stop();

							GameEditLoop2( "test3" );
							window->setView( v );
							titleMusic.play();
						}*/
						if( ev.key.code == Keyboard::Escape )
						{
							//quit = true;
						}
						else if( ev.key.code == Keyboard::M )
						{
							CustomMapsOption();
							//WorldSelectMenu();
						}
						else if( ev.key.code == Keyboard::Return || ev.key.code == Keyboard::Space )
						{
							//menuMode = WORLDMAP;
							//worldMap->state = WorldMap::PLANET_AND_SPACE;//WorldMap::PLANET_AND_SPACE;
							//worldMap->frame = 0;
							//worldMap->UpdateMapList();

						}
						else if( ev.key.code == Keyboard::Up )
						{
							/*currentMenuSelect--;
							if( currentMenuSelect < 0 )
								currentMenuSelect = 4;*/
						}
						else if( ev.key.code == Keyboard::Down )
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
						if( ev.mouseButton.button == Mouse::Button::Left )
						{
					//		ls.LeftClick( true, sf::Mouse::getPosition( *window ) );
						}
						break;
					}
				case sf::Event::MouseButtonReleased:
					{
						if( ev.mouseButton.button == Mouse::Button::Left )
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

					if( menuCurrInput.B && !menuPrevInput.B )
					{
						quit = true;
						break;
					}

					if( menuCurrInput.A || menuCurrInput.back || menuCurrInput.Y || menuCurrInput.X || 
						menuCurrInput.rightShoulder || menuCurrInput.leftShoulder )
					{
						switch (saSelector->currIndex)
						{
						case M_ADVENTURE:
						{
							SetMode(TRANS_MAIN_TO_SAVE);
							break;
						}
						case M_FREE_PLAY:
						{
							SetMode(TRANS_MAIN_TO_MAPSELECT);
							break;
						}
						case M_LOCAL_MULTIPLAYER:
						{
							SetMode(TRANS_MAIN_TO_MAPSELECT);
							break;
						}
						case M_LEVEL_EDITOR:
						{
							SetMode(TRANS_MAIN_TO_MAPSELECT);
							break;
						}
						case M_OPTIONS:
						{
							//config->Load();
							Config::CreateLoadThread(config);
							SetMode(TRANS_MAIN_TO_OPTIONS);
							break;
						}
						case M_CREDITS:
						{
							SetMode(TRANS_MAIN_TO_CREDITS);
							break;
						}
						case M_EXIT:
						{
							quit = true;
							break;
						}
						}

						/*case M_ADVENTURE:
							{
								menuMode = MULTIPREVIEW;
								multiLoadingScreen->Reset( "Maps/W1/arena04.brknk" );


								break;


								menuMode = SAVEMENU;
								saveMenu->Reset();
								
								break;
							}
						case M_CONTINUE:
							{
								menuMode = SAVEMENU;
								saveMenu->Reset();
								break;
							}
						case M_CUSTOM_MAPS:
							break;
						case M_LEVEL_EDITOR:
							break;
						case M_OPTIONS:
							break;
						case M_CREDITS:
							break;
						case M_EXIT:
							break;
						}*/
					}
					else
					{
						saSelector->UpdateIndex(menuCurrInput.LUp(), menuCurrInput.LDown());
					}

					UpdateMenuOptionText();

					//bool canMoveSame = (moveDelayCounter == 0);

				
					//if( (menuCurrInput.LDown() || menuCurrInput.PDown()) && ( !moveDown || canMoveSame ) )
					//{
					//	currentMenuSelect++;
					//	if( currentMenuSelect == M_Count )
					//		currentMenuSelect = 0;
					//	//moveDown = true;
					//	moveDelayCounter = moveDelayFrames;
					//}
					//else if( ( menuCurrInput.LUp() || menuCurrInput.PUp() ) && ( !moveUp || canMoveSame ) )
					//{
					//	currentMenuSelect--;
					//	if( currentMenuSelect < 0 )
					//		currentMenuSelect = M_Count - 1;
					//	//moveUp = true;
					//	moveDelayCounter = moveDelayFrames;
					//}
					//else
					//{
					//}
				

					/*if( moveDelayCounter > 0 )
					{
						moveDelayCounter--;
					}*/
				
					/*if( !(menuCurrInput.LDown() || menuCurrInput.PDown()) )
						{
							moveDelayCounter = 0;
							moveDown = false;
						}
						else if( ! ( menuCurrInput.LUp() || menuCurrInput.PUp() ) )
						{
							moveDelayCounter = 0;
							moveUp = false;
						}*/
				

					/*if( kinTitleSpriteFrame == kinTotalFrames )
					{
						kinTitleSpriteFrame = 0;
					}

					int trueKinFrame = 0;
					if( kinTitleSpriteFrame < 8 * 2 )
					{
						trueKinFrame = 0;
					}
					else if( kinTitleSpriteFrame < 16 * 2 )
					{
						trueKinFrame = 1;
					}
					else if( kinTitleSpriteFrame < 24 * 2 )
					{
						trueKinFrame = 2;
					}
					else if( kinTitleSpriteFrame < 48 * 2 )
					{
						trueKinFrame = 3;
					}
					else if( kinTitleSpriteFrame < 56 * 2 )
					{
						trueKinFrame = 4;
					}
					else if( kinTitleSpriteFrame < 60 * 2 )
					{
						trueKinFrame = 5;
					}
					else
					{
						trueKinFrame = 6;
					}*/

					//cout << "kinsprite: " << trueKinFrame << endl;
					//kinTitleSprite.setTexture( *(ts_kinTitle[ trueKinFrame ]->texture) );
					//kinTitleSprite.setOrigin( 0, kinTitleSprite.getLocalBounds().height );

					//kinTitleSpriteFrame++;	
					break;
				}
			case WORLDMAP:
				{
					while( window->pollEvent( ev ) )
					{
						switch( ev.type )
						{
							case sf::Event::KeyPressed:
							{
								if( ev.key.code == Keyboard::Escape )
								{
									//quit = true;
								}
							}
						}
					}

					worldMap->Update(menuPrevInput, menuCurrInput);

					break;
				}
			case LOADINGMAP:
			{
				if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
				{
					delete loadThread;
					loadThread = NULL;
					menuMode = RUNNINGMAP;
				}
				else
				{
					loadingIconBackpack[1].rotate(-1);
					loadingIconBackpack[2].rotate(2);
				}
				break;
			}
			case RUNNINGMAP:
			{
				View oldView = window->getView();
				
				GameSession::GameResultType result = 
					(GameSession::GameResultType)currLevel->Run();
				SaveFile *currFile = GetCurrentProgress();

				switch (result)
				{
				case GameSession::GR_WIN:
				{
					World & world = currFile->worlds[currLevel->mh->envType];
					bool doneCheck = false;
					for (int i = 0; i < world.numSectors && !doneCheck; ++i)
					{
						Sector &sec = world.sectors[i];
						for (int j = 0; j < sec.numLevels && !doneCheck; ++j)
						{
							Level &lev = sec.levels[j];
							if (lev.GetFullName() == currLevel->fileName)
							{
								if (!lev.GetComplete())
								{
									lev.justBeaten = true;
								}

								lev.SetComplete(true);
								
								doneCheck = true;
							}
						}
					}
					break;
				}
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

				currFile->Save();

				delete currLevel;
				currLevel = NULL;

				window->setView(oldView);

				menuMode = MainMenu::WORLDMAP;
				break;
				
			}
			case SAVEMENU:
				{
				worldMap->Update(menuPrevInput, menuCurrInput);
				saveMenu->Update();
					//parBack->Update( offset0 );
					//parFront->Update( offset1 );

					//backPar->Update( 

					break;
				}
			case TRANS_MAIN_TO_SAVE:
			{
				if (transFrame == transLength)
				{
					menuMode = SAVEMENU;
					break;
				}
				
				float fadeFactor; 

				if (transFrame >= transLength / 2)
				{
					fadeFactor = (float)( transFrame - (transLength / 2) ) / (transLength / 2);
					fadeFactor = 1.f - fadeFactor;
				}
				else
				{
					fadeFactor = (float)transFrame / (transLength / 2);
				}
				sf::Color fadeColor = fadeRect.getFillColor();
				fadeRect.setFillColor(Color(fadeColor.r, fadeColor.g,
					fadeColor.b, (fadeFactor * 255) ) );
				
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
				//worldMap->Reset( );
				//worldMap->Update( currInput, prevInput );
				/*if (slideCurrFrame > numSlideFrames)
				{
					
				}
				else
				{
					Slide();
				}*/
				break;
			}
			case TRANS_SAVE_TO_MAIN:
			{
				UpdateMenuOptionText();


				if (transFrame == transLength)
				{
					menuMode = MAINMENU;
					break;
				}

				

				float fadeFactor;

				if (transFrame >= transLength / 2)
				{
					fadeFactor = (float)(transFrame - (transLength / 2)) / (transLength / 2);
					fadeFactor = 1.f - fadeFactor;
				}
				else
				{
					fadeFactor = (float)transFrame / (transLength / 2);
				}
				sf::Color fadeColor = fadeRect.getFillColor();
				fadeRect.setFillColor(Color(fadeColor.r, fadeColor.g,
					fadeColor.b, (fadeFactor * 255)));

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
				
				/*if (slideCurrFrame > numSlideFrames)
				{
					menuMode = MAINMENU;
				}
				else
				{
					Slide();
				}*/
				break;
			}
			case TRANS_SAVE_TO_WORLDMAP:
				{
					//menuMode = INTROMOVIE;
					//introMovie->Play();
					menuMode = WORLDMAP;
					worldMap->testSelector->UpdateAllInfo();
					break;
					//saveTexture->clear();
					//if( kinFaceFrame == saveKinFaceTurnLength * 3 + 40 )
					//{
					//	menuMode = WORLDMAP;
					//	break;
					//	//kinFaceFrame = 0;
					//}

					//if( kinFaceFrame < saveKinFaceTurnLength * 3 )
					//{
					//	saveKinFace.setTextureRect( ts_saveMenuKinFace->GetSubRect( kinFaceFrame / 3 ) );
					//}
					//else
					//{
					//	saveKinFace.setTextureRect( ts_saveMenuKinFace->GetSubRect( saveKinFaceTurnLength - 1 ) );
					//}
				
					//if( kinFaceFrame < saveJumpLength * saveJumpFactor )
					//{
					//	if( kinFaceFrame == 0 )
					//	{
					//		saveKinJump.setTexture( *ts_saveKinJump1->texture );
					//	}
					//	else if( kinFaceFrame == 3 * saveJumpFactor )
					//	{
					//		saveKinJump.setTexture( *ts_saveKinJump2->texture );
					//	}

					//	int f = kinFaceFrame / saveJumpFactor;
					//	if( kinFaceFrame < 3 * saveJumpFactor )
					//	{
					//		saveKinJump.setTextureRect( ts_saveKinJump1->GetSubRect( f ) );
					//	}
					//	else
					//	{
					//		saveKinJump.setTextureRect( ts_saveKinJump2->GetSubRect( f - 3 ) );
					//	}

					//	saveKinJump.setOrigin( saveKinJump.getLocalBounds().width, 0);
					//}
				
					//transAlpha = (1.f - transWorldMapFrame / 40.f) * 255;

					//kinFaceFrame++;

					//if( kinFaceFrame >= saveKinFaceTurnLength * 3 )
					//{
					//	transWorldMapFrame++;
					//}
				

					//UpdateClouds();
					break;
				}
			case MULTIPREVIEW:
				{
					multiLoadingScreen->Update();
					break;
				}
			case TRANS_MAPSELECT_TO_MULTIPREVIEW:
				{
					menuMode = MULTIPREVIEW;
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
				menuMode = MAPSELECT;
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
				menuMode = MAPSELECT;
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
				mapSelectionMenu->Update(menuCurrInput, menuPrevInput);
					break;
				}
			case TRANS_MAPSELECT_TO_MAIN:
			{
				menuMode = MAINMENU;
				/*if (slideCurrFrame > numSlideFrames)
				{
					menuMode = MAINMENU;
				}
				else
				{
					Slide();
				}*/
				break;
			}
			case TRANS_MAIN_TO_OPTIONS:
			{
				menuMode = OPTIONS;
				config->WaitForLoad();

				optionsMenu->Load();
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
				optionsMenu->Update();
				break;
			}
			case TRANS_OPTIONS_TO_MAIN:
			{
				menuMode = MAINMENU;
				/*if (slideCurrFrame > numSlideFrames)
				{
					menuMode = MAINMENU;
				}
				else
				{
					Slide();
				}*/
				break;
			}
			case TRANS_MAIN_TO_CREDITS:
			{
				menuMode = CREDITS;
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
				creditsMenu->Update();
				break;
			}
			case TRANS_CREDITS_TO_MAIN:
			{
				menuMode = MAINMENU;
				/*if (slideCurrFrame > numSlideFrames)
				{
					menuMode = MAINMENU;
				}
				else
				{
					Slide();
				}*/
				break;
			}
			case INTROMOVIE:
			{
				if (!introMovie->Update() || (menuCurrInput.A && !menuPrevInput.A))
				{
					introMovie->Stop();

					if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
					{
						delete loadThread;
						loadThread = NULL;
						menuMode = RUNNINGMAP;
					}
					else
					{
						menuMode = LOADINGMAP;
					}
					//AdventureLoadLevel(&GetCurrentProgress()->worlds[0].sectors[0].levels[0]);
				}
				
				break;
			}
				
		}
			

			soundNodeList->Update();

			accumulator -= TIMESTEP;
		}
		
		if( quit )
		{
			break;
		}

		switch( menuMode )
		{
		case SPLASH:
			{
				preScreenTexture->draw( splashSprite );
				break;
			}
		case SPLASH_TRANS:
			{
				preScreenTexture->draw( splashSprite );
				preScreenTexture->draw( fadeRect );
				break;
			}
		case MAINMENU:
			{
				//preScreenTexture->setView( v );
				titleScreen->Draw(preScreenTexture);
				
				DrawMenuOptionText(preScreenTexture);

				//preScreenTexture->setView( uiView );

				if( splashFadeFrame <= splashFadeOutLength )
				{
					preScreenTexture->draw( fadeRect );
				}
				break;
			}
		case WORLDMAP:
			{
				preScreenTexture->setView(v);
				worldMap->Draw(preScreenTexture);
			}
			break;
		case TRANS_SAVE_TO_WORLDMAP:
		case SAVEMENU:
			{
			///	preScreenTexture->draw( worldMap->
			worldMap->Draw(preScreenTexture);
			saveMenu->Draw(preScreenTexture);

				
				
				break;
			}
		case MULTIPREVIEW:
			{
				multiLoadingScreen->Draw( preScreenTexture );
				break;
			}
		case LOADINGMAP:
		{
			preScreenTexture->draw(loadingBGSpr);

			for( int i = 0; i < 3; ++i )
				preScreenTexture->draw(loadingIconBackpack[i]);

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
			mapSelectionMenu->Draw( preScreenTexture );
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
			int tFrame = transFrame - 1;
			if (tFrame < transLength / 2)
			{
				titleScreen->Draw( preScreenTexture );
			}
			else
			{
				worldMap->Draw(preScreenTexture);
				saveMenu->Draw(preScreenTexture);
			}
			//preScreenTexture->setView(v);
			

			preScreenTexture->draw(fadeRect);

			
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
				DrawMenuOptionText(preScreenTexture);
			}
			//preScreenTexture->setView(v);


			preScreenTexture->draw(fadeRect);
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
			//Vector2f f = v.getCenter();
			//v.setCenter(-960, 540);
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
		case INTROMOVIE:
			introMovie->Draw(preScreenTexture);
			break;

		}
		//window->pushGLStates();
		
		//worldMap->Draw(preScreenTexture);

		
		//window->popGLStates();
		
		//window->setView( window->getDefaultView() );
		
		
		//window->setView( //window->getDefaultView() );

		
		
		//prim.DrawTetrahedron( window );

		
		//prim.Draw2( window );

		//window->pushGLStates();
		
		

		//window->popGLStates();

		//if( menuMode == SAVEMENU || menuMode == TRANS_SAVE_TO_WORLDMAP )
		//{
		//	saveTexture->display();
		//	sf::Sprite saveSpr;
		//	saveSpr.setTexture( saveTexture->getTexture() );
		//	//saveSpr.getOrigin( )

		//	if( menuMode == TRANS_SAVE_TO_WORLDMAP )
		//	{
		//		saveSpr.setColor( Color( 255, 255, 255, transAlpha ) );
		//	}
		//	preScreenTexture->draw( saveSpr );
		//}

		//preScreenTexture->draw( ff, 4, sf::Quads,  &sh );
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



void MainMenu::ResizeWindow( int p_windowWidth, 
		int p_windowHeight, int p_style )
{
	if( windowWidth == p_windowWidth && windowHeight == p_windowHeight && style == p_style )
	{
		return;
	}

	style = p_style;
	windowWidth = p_windowWidth;
	windowHeight = p_windowHeight;

	//if( style == p_style )

	//make sure to set to the center
	window->setSize(Vector2u(windowWidth, windowHeight));
	/*else
	{
		if (window->isOpen())
		{
			window->
		}

		window->create(VideoMode(windowWidth, windowHeight), "Breakneck", style);
	}*/
	 
	//View blahV;
	//blahV.setCenter( 0, 0 );
	//blahV.setSize( 1920/ 2, 1080 / 2 );
	////v.setCenter( 960, 540 );
	//window->setView( blahV );
}

void MainMenu::AdventureLoadLevel(Level *lev, bool loadingScreen)
{
	string levelPath = lev->GetFullName();// name;
	//View oldView = window->getView();

	currLevel = new GameSession(saveMenu->files[saveMenu->selectedSaveIndex], this, levelPath);

	if( loadingScreen )
		menuMode = LOADINGMAP;

	loadThread = new boost::thread(GameSession::sLoad, currLevel);
}

void MainMenu::PlayIntroMovie()
{
	worldMap->state = WorldMap::COLONY;
	worldMap->selectedColony = 0;
	worldMap->selectedLevel = 0;
	worldMap->testSelector->UpdateAllInfo();

	menuMode = MainMenu::Mode::INTROMOVIE;
	introMovie->Play();

	AdventureLoadLevel(&(GetCurrentProgress()->worlds[0].sectors[0].levels[0]), false);
	//soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_SELECT]);
}

CustomMapsHandler::CustomMapsHandler( MainMenu *p_menu )
		:menu( p_menu ), optionChosen( false ), showNamePopup( false )
{
}

void CustomMapsHandler::ButtonCallback( Button *b, const std::string & e )
{
	LevelSelector &ls = *menu->levelSelector;
	if( ls.text[ls.selectedIndex].getFillColor() == Color::Blue )
	{
		if( b->name == "Play" )
		{
			optionChosen = true;
			GameSession *gs = new GameSession( NULL, menu, ls.GetSelectedPath() );
			GameSession::sLoad( gs );
			gs->Run();
			menu->window->setView( menu->uiView );
			delete gs;
		}
		else if( b->name == "Edit" )
		{
			optionChosen = true;
			menu->GameEditLoop( ls.GetSelectedPath() );//ls.paths[ls.selectedIndex].().string() );//ls.text[ls.selectedIndex].getString() );
			menu->window->setView( menu->uiView );
		}
		else if( b->name == "Delete" )
		{	
			//std::stringstream SSr;
			//SSr << ls.GetSelectedPath() << ls.GetSelectedName() << ".brknk";
			boost::filesystem::remove( ls.GetSelectedPath() );
			ls.UpdateMapList();
		}
	}
	else
	{
			
	}

	if( b->name == "Create New" )
	{			
		showNamePopup = true;
		optionChosen = true;	
		//cout << "what" << endl;
		//cout << b->owner->name << ", " << b->owner->textBoxes.size() << ", " << b->owner->textBoxes.count( "name" ) << endl;
				
	}
	else if( b->name == "ok" )
	{
		showNamePopup = false;
		ls.newLevelName = b->owner->textBoxes["name"]->text.getString().toAnsiString();
	}
		
}

void CustomMapsHandler::TextBoxCallback( TextBox *tb, const std::string & e )
{
}

void CustomMapsHandler::GridSelectorCallback( GridSelector *gs, const std::string & e )
{
}

void CustomMapsHandler::CheckBoxCallback( CheckBox *cb, const std::string & e )
{
}

MapSelectionMenu::MapSelectionMenu(MainMenu *p_mainMenu, sf::Vector2f &p_pos )
	:mainMenu( p_mainMenu ), font( p_mainMenu->arial ), topIndex( 0 ),
	oldCurrIndex( 0 )
{
	multiSelectorState = MS_NEUTRAL;

	toMultiTransLength = 60;
	fromMultiTransLength = 60;
	

	allItems = NULL;
	int waitFrames[3] = { 10, 5, 2 };
	int waitModeThresh[2] = { 2, 2 };
	saSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, 0, 0);

	menuOffset = Vector2f(0, 0);
	topMid = p_pos + Vector2f( BOX_WIDTH / 2, 0 ) + menuOffset;

	SetupBoxes();

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		itemName[i].setFont(font);
		itemName[i].setCharacterSize(40);
		itemName[i].setFillColor(Color::White);
	}	

	descriptionText.setFont(font);
	descriptionText.setCharacterSize(20);
	descriptionText.setFillColor(Color::White);
	descriptionText.setPosition(Vector2f( 960 + 150, 680 + 40) + menuOffset);

	previewBlank = true;
	blankTest.setFillColor(Color::Blue);
	blankTest.setSize(Vector2f(960 + 24, 540 + 24));
	blankTest.setPosition(Vector2f( 960, 0 ) + menuOffset);

	previewSprite.setPosition(menuOffset + Vector2f(960 + 24, 24));//blankTest.getPosition());

	ts_bg = mainMenu->tilesetManager.GetTileset("Menu/map_select_menu.png", 1920, 1080);
	bg.setTexture(*ts_bg->texture);
	bg.setPosition(menuOffset);

	//playerindex may change later
	singleSection = new MultiSelectionSection(mainMenu, NULL, 0, Vector2f( 580 + 240, 680 ) + menuOffset);
	
	state = S_MAP_SELECTOR;

	gs = NULL;
	loadThread = NULL;

	LoadItems();

	UpdateItemText();

	UICheckbox *check = new UICheckbox(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, "testcheckbox", 300, 50);
	UICheckbox *check1 = new UICheckbox(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, "testcheckbox", 300, 50);
	UICheckbox *check2 = new UICheckbox(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, "testcheckbox", 300, 50);

	UIControl *testBlah[] = { check, check1, check2 };

	filterOptions = new UIVerticalControlList(NULL, sizeof( testBlah ) / sizeof( UIControl*), testBlah, 20);
	Vector2f tLeft = Vector2f(600, 120) + menuOffset;
	filterOptions->SetTopLeft( tLeft.x, tLeft.y );

	progressDisplay = new LoadingMapProgressDisplay(mainMenu, menuOffset + Vector2f(960, 540));
	progressDisplay->SetProgressString("hello here i am");
	progressDisplay->SetProgressString("hello here i am", 2);

	musicSelector = new MusicSelector(mainMenu, this, Vector2f( topMid.x, topMid.y + 80 ), mainMenu->musicManager );
	musicSelector->UpdateNames();

	ghostSelector = new RecordGhostMenu(mainMenu, topMid);
	ghostSelector->UpdateItemText();

	//ts_multiProfileRow = mainMenu->tilesetManager.GetTileset("Menu/multiprofile.png", 1920, 400);
	//multiProfileRow.setTexture(*ts_multiProfileRow->texture);

	ts_multiSelect = mainMenu->tilesetManager.GetTileset("Menu/multiselector.png", 960, 680);
	multiSelect.setTexture(*ts_multiSelect->texture);

	multiRowOnPos = menuOffset + Vector2f(0, 1080 - 400);
	multiRowOffPos = menuOffset + Vector2f(0, 1080);

	//multiProfileRow.setPosition( multiRowOffPos );
	

	multiSelectOnPos = menuOffset + Vector2f(0, 0);
	multiSelectOffPos = menuOffset + Vector2f(0, -680);
	

	int quarter = 1920 / 4;
	//menuOffset = Vector2f(-1920 * 2, 0);
	for (int i = 0; i < 4; ++i)
	{
		Vector2f topMid = Vector2f(quarter * i + quarter / 2, 1080 - 400) + menuOffset;
		multiPlayerSection[i] = new MultiSelectionSection(mainMenu, this, i, topMid);
	}

	for (int i = 0; i < 4; ++i)
	{
		multiPlayerSection[i]->SetTopMid(menuOffset + Vector2f(480 * i + 240, multiRowOffPos.y));
	}
	
	//filterOptions = new UIVerticalControlList()
}

int MapSelectionMenu::NumPlayersReady()
{
	return 4;
}

void MapSelectionMenu::SetupBoxes()
{
	sf::Vector2f currTopMid;
	int extraHeight = 0;

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		currTopMid = topMid + Vector2f(0, extraHeight);

		boxes[i * 4 + 0].position = Vector2f(currTopMid.x - BOX_WIDTH / 2, currTopMid.y);
		boxes[i * 4 + 1].position = Vector2f(currTopMid.x + BOX_WIDTH / 2, currTopMid.y);
		boxes[i * 4 + 2].position = Vector2f(currTopMid.x + BOX_WIDTH / 2, currTopMid.y + BOX_HEIGHT);
		boxes[i * 4 + 3].position = Vector2f(currTopMid.x - BOX_WIDTH / 2, currTopMid.y + BOX_HEIGHT);

		boxes[i * 4 + 0].color = Color::Red;
		boxes[i * 4 + 1].color = Color::Red;
		boxes[i * 4 + 2].color = Color::Red;
		boxes[i * 4 + 3].color = Color::Red;

		extraHeight += BOX_HEIGHT + BOX_SPACING;
	}
}

#include <boost/filesystem.hpp>

MapSelectionItem::~MapSelectionItem()
{
	delete headerInfo;
}

void MapSelectionMenu::LoadPath(boost::filesystem::path &p)
{
	vector<path> v;
	try
	{
		if (exists(p))    // does p actually exist?
		{
			if (is_regular_file(p))        // is p a regular file?   
			{
				if (p.extension().string() == ".brknk")
				{
					items.push_back(p);
				}
			}
			else if (is_directory(p))      // is p a directory?
			{
				copy(directory_iterator(p), directory_iterator(), back_inserter(v));

				sort(v.begin(), v.end());

				for (vector<path>::iterator it(v.begin()); it != v.end(); ++it)
				{
					LoadPath( (*it));
				}
			}
			else
				cout << p << " exists, but is neither a regular file nor a directory\n";
		}
		else
		{
			cout << p << " does not exist\n";
			assert(0);
		}
	}
	catch (const filesystem_error& ex)
	{
		cout << ex.what() << '\n';
		assert(0);
	}
}

bool MapSelectionMenu::AllPlayersReady()
{
	for (int i = 0; i < 4; ++i)
	{
		if (multiPlayerSection[i]->active)
		{
			if (!multiPlayerSection[i]->IsReady())
				return false;
		}
	}



	return true;
}

MapHeader * MapSelectionMenu::ReadMapHeader(std::ifstream &is)
{
	MapHeader *mh = new MapHeader;

	assert(is.is_open());

	int part1Num = -1;
	int part2Num = -1;

	string versionString;
	is >> versionString;
	int sepIndex = versionString.find('.');
	string part1 = versionString.substr(0, sepIndex);
	string part2 = versionString.substr(sepIndex + 1);
	try
	{
		part1Num = stoi(part1);
	}
	catch (std::exception & e)
	{
		assert(0);
	}

	try
	{
		part2Num = stoi(part2);
	}
	catch (std::exception & e)
	{
		assert(0);
	}

	//get description
	char last = 0;
	char curr = 0;
	stringstream ss;
	string tempStr;
	if (!is.get()) //get newline char out of the way
	{
		assert(0);
	}
	while (true)
	{
		last = curr;
		if (!is.get(curr))
		{
			assert(0);
		}

		if (last == '<' && curr == '>')
		{
			break;
		}
		else
		{
			if (last != 0)
			{
				ss << last;
			}
		}
	}

	int numShards;
	is >> numShards;

	mh->numShards = numShards;
	string temp;
	for (int i = 0; i < numShards; ++i)
	{
		is >> temp;
		mh->shardNameList.push_back(temp);
	}


	int numSongValues;
	is >> numSongValues;

	int oftenLevel;
	string tempSongStr;
	for (int i = 0; i < numSongValues; ++i)
	{
		is >> tempSongStr;	
		is >> oftenLevel;

		mh->songLevels[tempSongStr] = oftenLevel;
	}

	string collectionName;
	is >> collectionName;

	//string gameTypeName;
	int gameMode;
	is >> gameMode;

	is >> mh->envType;

	is >> mh->envLevel;

	is >> mh->leftBounds;
	is >> mh->topBounds;
	is >> mh->boundsWidth;
	is >> mh->boundsHeight;

	is >> mh->numVertices;

	mh->collectionName = collectionName;
	mh->ver1 = part1Num;
	mh->ver2 = part2Num;
	mh->description = ss.str();

	mh->gameMode = (MapHeader::MapType)gameMode;

	return mh;
}

void MapSelectionMenu::LoadItems()
{
	path p(current_path() / "/Maps/");
	LoadPath(p);

	map<string, MapCollection*> collectionMap;
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		ifstream is;
		is.open((*it).string());
		if (is.is_open())
		{
			MapHeader *mh = ReadMapHeader(is);
			string pFile = string( "Maps/") + (*it).relative_path().stem().string() + string("_preview_912x492.png");//string("Maps/") + (*it).filename().stem().string() + string("_preview_960x540.png");
			//string defaultFile = "Menu/nopreview_960x540.png";

			if (collectionMap.find(mh->collectionName) != collectionMap.end())
			{

				MapSelectionItem *item = new MapSelectionItem((*it), mh);
				
				item->ts_preview = mainMenu->tilesetManager.GetTileset(pFile, 960, 540);
				if (item->ts_preview == NULL)
				{
					//item->ts_preview = mainMenu->tilesetManager.GetTileset(defaultFile, 960, 540);
				}
				
				MapCollection *temp = collectionMap[mh->collectionName];

				item->collection = temp;

				temp->collectionName = mh->collectionName;
				temp->maps.push_back(item);
			}
			else
			{
				MapSelectionItem *item = new MapSelectionItem((*it), mh);

				item->ts_preview = mainMenu->tilesetManager.GetTileset(pFile, 960, 540);

				MapCollection *coll = new MapCollection;
				collectionMap[mh->collectionName] = coll;

				item->collection = coll;

				coll->collectionName = mh->collectionName;
				coll->maps.push_back(item);


			}
		}
		else
		{
			assert(0);
		}
	}

	for( auto it = collectionMap.begin(); it != collectionMap.end(); ++it )
	{
		collections.push_back((*it).second);
	}

	numTotalItems = 0;
	for (auto it = collections.begin(); it != collections.end(); ++it)
	{
		++numTotalItems;
		numTotalItems += (*it)->maps.size();
	}

	if (allItems != NULL)
	{
		delete[] allItems;
	}
	
	allItems = new pair<string, MapIndexInfo>[numTotalItems];
	
	int ind = 0;
	for (auto it = collections.begin(); it != collections.end(); ++it)
	{
		MapIndexInfo mi;
		mi.coll = (*it);
		mi.item = NULL;

		allItems[ind] = pair<string, MapIndexInfo>((*it)->collectionName, mi);
		++ind;
		assert((*it)->maps.size() > 0);
		for (auto it2 = (*it)->maps.begin(); it2 != (*it)->maps.end(); ++it2)
		{
			mi.coll = NULL;
			mi.item = (*it2);

			allItems[ind] = pair<string, MapIndexInfo>((*it2)->path.filename().stem().string(), mi);
			++ind;
		}
	}
}

void MapSelectionMenu::LoadMap()
{
	//filePath = p_path;

	//SetPreview();

	/*if (stopThread != NULL)
	{
		stopThread->join();
		stopThread = NULL;
	}*/

	//if (loadThread != NULL)
	//{
	//	//assert(loadThread->joinable());
	//	//cout << "joining111" << endl;
	//	//loadThread->join();

	//	delete loadThread;
	//}
	//
	//if (gs != NULL)
	//{
	//	delete gs;
	//}
	

	int cIndex = saSelector->currIndex;
	int pIndex = GetPairIndex(cIndex);



	gs = new GameSession(NULL, mainMenu, allItems[pIndex].second.item->path.string() );

	gs->progressDisplay = progressDisplay;

	loadThread = new boost::thread(GameSession::sLoad, gs);
}

bool MapSelectionMenu::WriteMapHeader(std::ofstream &of, MapHeader *mh)
{
	of << mh->ver1 << "." << mh->ver2 << "\n";
	of << mh->description << "<>\n";

	of << mh->numShards << "\n";
	for (auto it = mh->shardNameList.begin(); it != mh->shardNameList.end(); ++it)
	{
		of << (*it) << "\n";
	}

	of << mh->songLevels.size() << "\n";
	for (auto it = mh->songLevels.begin(); it != mh->songLevels.end(); ++it)
	{
		of << (*it).first << " " << (*it).second << "\n";
	}

	of << mh->collectionName << "\n";
	of << mh->gameMode << "\n";

	of << (int)mh->envType << " " << mh->envLevel << endl;

	of << mh->leftBounds << " " << mh->topBounds << " " << mh->boundsWidth << " " << mh->boundsHeight << endl;

	of << mh->numVertices << endl;

	return true;
}

bool MapSelectionMenu::ReplaceHeader(boost::filesystem::path &p, MapHeader *mh )
{
	ifstream is;
	is.open(p.string());

	path from("map.tmp");
	ofstream of;
	of.open(from.string());
	assert(of.is_open());
	WriteMapHeader(of, mh);

	if (is.is_open())
	{
		MapHeader *oldHeader = ReadMapHeader(is);

		is.get(); //gets rid of the extra newline char

		char c;
		while (is.get(c))
		{
			of.put(c);
		}

		of.close();
		is.close();

		delete oldHeader;
	}
	else
	{
		assert(0);
	}

	try
	{
		//assert(boost::filesystem::exists(from) && boost::filesystem::exists(p));
		boost::filesystem::copy_file(from, p, copy_option::overwrite_if_exists );
	}
	catch (const boost::system::system_error &err)
	{
		cout << "file already exists!" << endl;
		assert(0);
	}

	mh->songLevelsModified = false;

	return true;
}

void MapSelectionMenu::UpdateMultiInput()
{
	if (multiSelectorState == MS_NEUTRAL)
	{

	}
	else if( multiSelectorState == MS_MUSIC )
	{
		multiMusicPrev = multiMusicCurr;
		multiMusicCurr = ControllerState();
		for (int i = 0; i < 4; ++i)
		{
			
			

			if (multiPlayerSection[i]->profileSelect->state != ControlProfileMenu::S_MUSIC_SELECTOR )
			{
				continue;
			}

			ControllerState &pInput = mainMenu->GetPrevInput(i);
			ControllerState &cInput = mainMenu->GetCurrInput(i);
			GameController &c = mainMenu->GetController(i);

			//pInput = cInput;
			bool active = c.UpdateState();

			if (active)
			{
				cInput = c.GetState();
				multiMusicCurr.A |= (cInput.A && !pInput.A);
				multiMusicCurr.B |= (cInput.B && !pInput.B);
				multiMusicCurr.X |= (cInput.X && !pInput.X);
				multiMusicCurr.Y |= (cInput.Y && !pInput.Y);
				multiMusicCurr.rightShoulder |= (cInput.rightShoulder && !pInput.rightShoulder);
				multiMusicCurr.leftShoulder |= (cInput.leftShoulder && !pInput.leftShoulder);
				multiMusicCurr.start |= (cInput.start && !pInput.start);
				multiMusicCurr.leftTrigger = max(multiMusicCurr.leftTrigger, cInput.leftTrigger);
				multiMusicCurr.rightTrigger = max(multiMusicCurr.rightTrigger, cInput.rightTrigger);
				multiMusicCurr.back |= (cInput.back && !pInput.back);
				multiMusicCurr.leftStickPad |= cInput.leftStickPad;
			}
			else
			{
				cInput.Set(ControllerState());
			}
		}
	}
	else if (multiSelectorState == MS_GHOST)
	{
		//setup ghost curr
		multiGhostPrev = multiGhostCurr;
		for (int i = 0; i < 4; ++i)
		{
			if (multiPlayerSection[i]->profileSelect->state != ControlProfileMenu::S_GHOST_SELECTOR )
			{
				continue;
			}

			ControllerState &pInput = mainMenu->GetPrevInput(i);
			ControllerState &cInput = mainMenu->GetCurrInput(i);
			GameController &c = mainMenu->GetController(i);

			//pInput = cInput;
			bool active = c.UpdateState();

			if (active)
			{
				cInput = c.GetState();
				multiGhostCurr.A |= (cInput.A && !pInput.A);
				multiGhostCurr.B |= (cInput.B && !pInput.B);
				multiGhostCurr.X |= (cInput.X && !pInput.X);
				multiGhostCurr.Y |= (cInput.Y && !pInput.Y);
				multiGhostCurr.rightShoulder |= (cInput.rightShoulder && !pInput.rightShoulder);
				multiGhostCurr.leftShoulder |= (cInput.leftShoulder && !pInput.leftShoulder);
				multiGhostCurr.start |= (cInput.start && !pInput.start);
				multiGhostCurr.leftTrigger = max(multiGhostCurr.leftTrigger, cInput.leftTrigger);
				multiGhostCurr.rightTrigger = max(multiGhostCurr.rightTrigger, cInput.rightTrigger);
				multiGhostCurr.back |= (cInput.back && !pInput.back);
				multiGhostCurr.leftStickPad |= cInput.leftStickPad;
			}
			else
			{
				cInput.Set(ControllerState());
			}
		}
	}
}

void MapSelectionMenu::sStopLoadThread(MapSelectionMenu *mapMenu, TInfo &ti)
{
	mapMenu->StopLoadThread( ti );
}

void MapSelectionMenu::StopLoadThread( TInfo &ti )
{
	ti.gsession->SetContinueLoading(false);
	ti.loadThread->join();
	delete ti.loadThread;
	delete ti.gsession;
}

void MapSelectionMenu::Update(ControllerState &currInput,
	ControllerState &prevInput)
{
	progressDisplay->UpdateText();
	if (state == S_MAP_SELECTOR)
	{
		if (currInput.B && !prevInput.B)
		{
			mainMenu->SetMode(MainMenu::Mode::TRANS_MAPSELECT_TO_MAIN);
			return;
		}
		else if (currInput.X && !prevInput.X)
		{
			state = S_MAP_OPTIONS;
			return;
		}
		else if (currInput.Y && !prevInput.Y)
		{
			int cIndex = saSelector->currIndex;
			int pIndex = GetPairIndex(cIndex);
			MapSelectionItem *item = allItems[pIndex].second.item;
			if (item != NULL)
			{
				oldState = state;
				state = S_MUSIC_SELECTOR;
				musicSelector->UpdateNames();
				musicSelector->modifiedValues = false;
				musicSelector->SetMapName(item->path.filename().stem().string());
				
				return;
			}
		}
		

		int cIndex = saSelector->currIndex;
		int pIndex = GetPairIndex(cIndex);
		if (currInput.A && !prevInput.A)
		{
			MapCollection *mc = allItems[pIndex].second.coll;
			if (mc != NULL)
			{
				mc->expanded = !mc->expanded;
				UpdateItemText();
			}
			else
			{
				
				MapSelectionItem *item = allItems[pIndex].second.item;
				if (item->headerInfo->gameMode == MapHeader::T_RACEFIGHT)
				{
					//mainMenu->multiLoadingScreen->Reset(item->path);
					//mainMenu->SetMode(MainMenu::Mode::TRANS_MAPSELECT_TO_MULTIPREVIEW);

					state = S_TO_MULTI_TRANS;
					multiTransFrame = 0;
					//multiProfileRow.setPosition(multiRowOffPos);
					multiSelect.setPosition(multiSelectOffPos);
					for (int i = 0; i < 4; ++i)
					{
						multiPlayerSection[i]->SetTopMid(menuOffset + Vector2f(480 * i + 240, multiRowOffPos.y));
					}

					LoadMap();
					return;
				}
				else
				{
					//single player
					state = S_SELECTING_SKIN;

					LoadMap();
				}
				
				
			}
		}

		bool up = currInput.LUp();
		bool down = currInput.LDown();

		if (saSelector->totalItems > 1)
		{
			int changed = saSelector->UpdateIndex(up, down);
			cIndex = saSelector->currIndex;

			bool inc = changed > 0;
			bool dec = changed < 0;

			if (inc)
			{
				if (cIndex - topIndex == NUM_BOXES)
				{
					topIndex = cIndex - (NUM_BOXES - 1);
				}
				else if (cIndex == 0)
				{
					topIndex = 0;
				}
			}
			else if (dec)
			{
				if (cIndex == saSelector->totalItems - 1)
					topIndex = max(saSelector->totalItems - NUM_BOXES, 0);
				else if (cIndex < topIndex)
					topIndex = cIndex;
			}

			if (changed != 0)
			{
				UpdateItemText();

				int pIndex = GetPairIndex(cIndex);
				if (allItems[pIndex].second.item == NULL)
				{
					descriptionText.setString("");
					previewBlank = true;
				}
				else
				{
					descriptionText.setString(allItems[pIndex].second.item->headerInfo->description);
					if (allItems[pIndex].second.item->ts_preview != NULL)
					{
						previewSprite.setTexture(*allItems[pIndex].second.item->ts_preview->texture);
						previewBlank = false;
					}
				}

			}
		}


		UpdateBoxesDebug();
	}
	else if (state == S_SELECTING_SKIN)
	{
		if (singleSection->ShouldGoBack())
		{
			//kill loading map!

			state = S_MAP_SELECTOR;

			//assert(stopThread == NULL);
			
			CleanupStopThreads();

			TInfo ti;
			ti.gsession = gs;
			ti.loadThread = loadThread;
			stopThreads.push_back( new boost::thread(&MapSelectionMenu::sStopLoadThread, this, ti ) );
			return;
		}


		if (currInput.Y && !prevInput.Y)
		{
			int cIndex = saSelector->currIndex;
			int pIndex = GetPairIndex(cIndex);
			
			MapIndexInfo &mi = allItems[pIndex].second;

			oldState = state;
			state = S_MUSIC_SELECTOR;
			musicSelector->UpdateNames();
			musicSelector->modifiedValues = false;
			musicSelector->SetMapName(mi.item->path.filename().stem().string());
			return;
		}
		else if (currInput.X && !prevInput.X)
		{
			state = S_GHOST_SELECTOR;
			
			return;
		}
		else
		{
			singleSection->Update();
		}

		if (singleSection->IsReady() )
		{
			//loadThread->join();
			//boost::chrono::steady_clock::now()
			if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
			{
				int cIndex = saSelector->currIndex;
				int pIndex = GetPairIndex(cIndex);

				//mh->songLevelsModified = true;
				MapSelectionItem *mi = allItems[pIndex].second.item;
				if (mi->headerInfo->songLevelsModified )
				{
					ReplaceHeader(mi->path, mi->headerInfo);
				}

				mainMenu->GetController(singleSection->playerIndex).SetFilter(singleSection->profileSelect->currProfile->filter);

				list< GhostEntry*> ghosts;
				ghostSelector->GetActiveList(ghosts);
				if (!ghosts.empty())
				{
					gs->SetupGhosts(ghosts);
					//load ghosts
					//into list
				}

				int res = gs->Run();

				XBoxButton filter[ControllerSettings::Count];
				SetFilterDefault(filter);

				for (int i = 0; i < 4; ++i)
				{
					mainMenu->GetController(i).SetFilter(filter);
				}

				ghostSelector->UpdateLoadedFolders();



				TInfo ti;
				ti.gsession = gs;
				ti.loadThread = loadThread;
				stopThreads.push_back(new boost::thread(&MapSelectionMenu::sStopLoadThread, this, ti));

				/*delete loadThread;
				loadThread = NULL;
				delete gs;
				gs = NULL;*/

				View vv;
				vv.setCenter(960, 540);
				vv.setSize(1920, 1080);
				mainMenu->window->setView(vv);

				mainMenu->v.setCenter(mainMenu->leftCenter);
				mainMenu->v.setSize(Vector2f( 1920, 1080 ));
				mainMenu->preScreenTexture->setView(mainMenu->v);

				//singleSection->isReady = false;
				state = S_MAP_SELECTOR;


			}
		}
	}
	else if (state == S_MAP_OPTIONS)
	{
		if ((currInput.B && !prevInput.B) )//|| (currInput.B && !prevInput.B ) )
		{
			state = S_MAP_SELECTOR;
			return;
		}

		filterOptions->Update(currInput, prevInput);
	}
	else if (state == S_MUSIC_SELECTOR)
	{
		if ((currInput.B && !prevInput.B))
		{
			if (musicSelector->previewSong != NULL)
			{
				musicSelector->previewSong->music->stop();
				musicSelector->previewSong = NULL;
			}
			if (oldState == S_MAP_SELECTOR)
			{
				int cIndex = saSelector->currIndex;
				int pIndex = GetPairIndex(cIndex);
				
				MapSelectionItem *mi = allItems[pIndex].second.item;
				
				//guaranteed to be a file not a folder
				ReplaceHeader(mi->path, mi->headerInfo );

				musicSelector->modifiedValues = false;
				//save
			}
			else if (oldState == S_SELECTING_SKIN || oldState == S_MAP_OPTIONS )
			{

				//save after starting the level or when someone cancels the load //this might be causing a bug atm
			}
			if (musicSelector->modifiedValues)
			{

			}
			state = oldState;
		}

		musicSelector->Update(mainMenu->menuCurrInput, mainMenu->menuPrevInput);
	}
	else if (state == S_MUSIC_OPTIONS)
	{
		if (currInput.B && !prevInput.B)
		{
			state = S_MUSIC_SELECTOR;
		}
	}
	else if (state == S_GHOST_SELECTOR)
	{
		if (currInput.B && !prevInput.B)
		{
			state = S_SELECTING_SKIN;
			return;
			//state = S_MUSIC_SELECTOR;
		}
		
		ghostSelector->Update( mainMenu->menuCurrInput, mainMenu->menuPrevInput );
	}
	else if (state == S_GHOST_OPTIONS)
	{
		if (currInput.B && !prevInput.B)
		{
			state = S_GHOST_SELECTOR;
			return;
		}
	}
	else if (state == S_TO_MULTI_TRANS)
	{
		if (multiTransFrame > toMultiTransLength)
		{
			state = S_MULTI_SCREEN;
		}
		else
		{
			float r = (float)multiTransFrame / toMultiTransLength;
			multiProfileRow.setPosition(multiRowOffPos * (1 - r) + multiRowOnPos * r);

			multiSelect.setPosition(multiSelectOffPos * (1 - r) + multiSelectOnPos * r);
			
			for (int i = 0; i < 4; ++i)
			{
				multiPlayerSection[i]->SetTopMid(menuOffset + Vector2f(480 * i + 240, multiProfileRow.getPosition().y));
			}

			++multiTransFrame;
		}
	}
	else if (state == S_FROM_MULTI_TRANS)
	{
		if (multiTransFrame > fromMultiTransLength)
		{
			state = S_MAP_SELECTOR;
		}
		else
		{
			float r = (float)multiTransFrame / fromMultiTransLength;
			multiProfileRow.setPosition(multiRowOnPos * (1 - r) + multiRowOffPos * r);

			multiSelect.setPosition(multiSelectOnPos * (1 - r) + multiSelectOffPos * r);

			for (int i = 0; i < 4; ++i)
			{
				multiPlayerSection[i]->SetTopMid(menuOffset + Vector2f(480 * i + 240, multiProfileRow.getPosition().y));
			}

			++multiTransFrame;
		}
	}
	else if (state == S_MULTI_SCREEN)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (multiPlayerSection[i]->ShouldGoBack())
			{
				//kill loading map!
				state = S_FROM_MULTI_TRANS;
				multiTransFrame = 0;

				//assert(stopThread == NULL);

				CleanupStopThreads();

				TInfo ti;
				ti.gsession = gs;
				ti.loadThread = loadThread;
				stopThreads.push_back(new boost::thread(&MapSelectionMenu::sStopLoadThread, this, ti));
				return;
			}
		}

		for (int i = 0; i < 4; ++i)
		{
			multiPlayerSection[i]->Update();
		}

		bool ghostOn = false;
		bool musicOn = false;
		bool allNeutral = true;

		for (int i = 0; i < 4; ++i)
		{
			if (multiPlayerSection[i]->profileSelect->state == ControlProfileMenu::State::S_MUSIC_SELECTOR)
			{
				musicOn = true;
				allNeutral = false;
				break;
			}
		}

		for (int i = 0; i < 4; ++i)
		{
			if (multiPlayerSection[i]->profileSelect->state == ControlProfileMenu::State::S_GHOST_SELECTOR)
			{
				if (musicOn)
				{
					multiPlayerSection[i]->profileSelect->state = ControlProfileMenu::State::S_SELECTED;
				}
				else
				{
					ghostOn = true;
					allNeutral = false;
					break;
				}
			}
		}

		

		if (multiSelectorState == MS_NEUTRAL)
		{
			if (musicOn)
			{
				multiSelectorState = MS_MUSIC;
				musicSelector->SetMultiOn(true);
				return;
			}
			else if (ghostOn)
			{
				multiSelectorState = MS_GHOST;
				return;
			}
		}
		else if ((multiSelectorState == MS_MUSIC || multiSelectorState == MS_GHOST ) && allNeutral )
		{
			multiSelectorState = MS_NEUTRAL;
			musicSelector->SetMultiOn(false);
			return;
		}


		
		if (!musicOn && !ghostOn && mainMenu->menuCurrInput.start && !mainMenu->menuPrevInput.start )
		{
			if (AllPlayersReady() && NumPlayersReady() > 1 ) //replace the numplayersready based on the map specifics
			{
				//loadThread->join();
				//boost::chrono::steady_clock::now()
				if (loadThread->try_join_for(boost::chrono::milliseconds(0)))
				{
					for (int i = 0; i < 4; ++i)
					{
						mainMenu->GetController(i).SetFilter(multiPlayerSection[i]->profileSelect->currProfile->filter);
					}

					int res = gs->Run();

					XBoxButton filter[ControllerSettings::Count];
					SetFilterDefault(filter);

					for (int i = 0; i < 4; ++i)
					{
						mainMenu->GetController(i).SetFilter(filter);
					}

					/*delete loadThread;
					loadThread = NULL;
					delete gs;
					gs = NULL;*/
					CleanupStopThreads();

					TInfo ti;
					ti.gsession = gs;
					ti.loadThread = loadThread;
					stopThreads.push_back(new boost::thread(&MapSelectionMenu::sStopLoadThread, this, ti));

					View vv;
					vv.setCenter(960, 540);
					vv.setSize(1920, 1080);
					mainMenu->window->setView(vv);

					mainMenu->v.setCenter(mainMenu->leftCenter);
					mainMenu->v.setSize(Vector2f(1920, 1080));
					mainMenu->preScreenTexture->setView(mainMenu->v);

					mainMenu->SetMode(MainMenu::Mode::MAPSELECT);
					mainMenu->mapSelectionMenu->state = MapSelectionMenu::State::S_MAP_SELECTOR;
					mainMenu->v.setCenter(mainMenu->leftCenter);
					mainMenu->preScreenTexture->setView(mainMenu->v);
				}
			}
		}


		UpdateMultiInput();
		

		if (musicOn)
		{
			musicSelector->Update(multiMusicCurr, multiMusicPrev);
		}
		if ( musicOn &&  (multiMusicCurr.B && !multiMusicPrev.B))
		{
			if (musicSelector->previewSong != NULL)
			{
				musicSelector->previewSong->music->stop();
				musicSelector->previewSong = NULL;
			}

			if (multiSelectorState == MS_MUSIC_OPTIONS)
			{
				multiSelectorState == MS_MUSIC;
				return;
			}
			
			//save after starting the level or when someone cancels the load //this might be causing a bug atm
			
			if (musicSelector->modifiedValues)
			{

			}
		}


		if (ghostOn)
		{
			ghostSelector->Update(multiMusicCurr, multiMusicPrev);
		}
		if (ghostOn && (multiGhostCurr.B && !multiGhostPrev.B))
		{
			if (multiSelectorState == MS_GHOST_OPTIONS)
			{
				multiSelectorState = MS_GHOST;
				return;
			}
		}
	}
	else if (state == S_TO_MULTI_TRANS)
	{
		//hold B to go back
	}
}

void MapSelectionMenu::MoveUp()
{
	topIndex++;
	if (topIndex == items.size())
	{
		topIndex = 0;
	}
}

void MapSelectionMenu::MoveDown()
{
	topIndex--;
	if (topIndex == -1)
	{
		topIndex = items.size() - 1;
	}
}

void MapSelectionMenu::UpdateItemText()
{
	int totalShownItems = 0;
	for (auto it = collections.begin(); it != collections.end(); ++it)
	{
		++totalShownItems;
		if ((*it)->expanded)
		{
			totalShownItems += (*it)->maps.size();
		}
	}

	saSelector->totalItems = totalShownItems;

	if (numTotalItems == 0)
	{
		return;
	}

	if (topIndex > totalShownItems)
	{
		topIndex = totalShownItems - 1;
	}

	int ind = topIndex;

	int trueI;
	int i = 0;
	Color col;
	for (; i < NUM_BOXES; ++i)
	{
		trueI = (topIndex + i) % NUM_BOXES;
		if (i == totalShownItems)
		{
			for (; i < NUM_BOXES; ++i)
			{
				itemName[i].setString("");
			}
			break;
		}

		if (ind == totalShownItems)
			ind = 0;
		
		pair<string, MapIndexInfo> &p = allItems[GetPairIndex(ind)];
		string printStr = p.first;
		itemName[i].setString(printStr);
		itemName[i].setOrigin(0, 0);
		if (p.second.item != NULL)
		{
			MapSelectionItem *item = p.second.item;
			switch (item->headerInfo->gameMode)
			{
				case MapHeader::T_STANDARD:
				{
					col = Color::Cyan;
					break;
				}
				case MapHeader::T_RACEFIGHT:
				{
					col = Color::White;
					break;
				}
				default:
				{
					col = Color::Yellow;
				}
			}
			itemName[i].setFillColor(col);
		}
		else if (p.second.coll != NULL)
		{
			itemName[i].setFillColor(Color::White);
		}
		

		int xVal = topMid.x - BOX_WIDTH / 2;
		if (p.second.coll == NULL) //its just a file
		{
			MapSelectionItem *mi = p.second.item;
			
			xVal += 60;
		}
		else
		{
			xVal += 10;
			//descriptionText.setString("");
		}
		itemName[i].setPosition(xVal, topMid.y + (BOX_HEIGHT + BOX_SPACING) * i);

		++ind;
	}
}

int MapSelectionMenu::GetPairIndex(int index)
{
	int i = 0;
	for (int it = 0; it < index; ++it)
	{
		if (allItems[i].second.coll != NULL && !allItems[i].second.coll->expanded )
		{
			i += allItems[i].second.coll->maps.size();
		}

		++i;
	}

	return i;
}

void MapSelectionMenu::UpdateBoxesDebug()
{
	Color c;
	int trueI = (saSelector->currIndex - topIndex);
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		if (i == trueI)
		{
			c = Color::Blue;
		}
		else
		{
			c = Color::Red;
		}
		boxes[i * 4 + 0].color = c;
		boxes[i * 4 + 1].color = c;
		boxes[i * 4 + 2].color = c;
		boxes[i * 4 + 3].color = c;
	}
}

void MapSelectionMenu::CleanupStopThreads()
{
	for (auto it = stopThreads.begin(); it != stopThreads.end();)
	{
		if ((*it)->try_join_for(boost::chrono::milliseconds(0)))
		{
			(*it)->join();
			delete (*it);
			stopThreads.erase(it++);
		}
		else
		{
			++it;
		}
	}
}

bool MapSelectionMenu::IsMusicSelectorVisible()
{
	return state == S_MUSIC_SELECTOR || (state == S_MULTI_SCREEN && IsMultiMusicOn());
}

bool MapSelectionMenu::IsGhostSelectorVisible()
{
	return state == S_GHOST_SELECTOR || (state == S_MULTI_SCREEN && IsMultiGhostOn());
}

bool MapSelectionMenu::IsMultiMusicOn()
{
	for (int i = 0; i < 4; ++i)
	{
		if (multiPlayerSection[i]->profileSelect->state == ControlProfileMenu::S_MUSIC_SELECTOR)
		{
			return true;
		}
	}

	return false;
}

bool MapSelectionMenu::IsMultiGhostOn()
{
	for (int i = 0; i < 4; ++i)
	{
		if (multiPlayerSection[i]->profileSelect->state == ControlProfileMenu::S_GHOST_SELECTOR)
		{
			return true;
		}
	}

	return false;
}

void MapSelectionMenu::Draw(sf::RenderTarget *target)
{
	target->draw(bg);

	target->draw(boxes, NUM_BOXES * 4, sf::Quads);
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		target->draw(itemName[i]);
	}

	if (!previewBlank)
	{
		target->draw(previewSprite);
	}

	target->draw(descriptionText);


	

	if (state == S_SELECTING_SKIN)
	{
		singleSection->Draw(target);
		//target->draw(playerSprite);
		//profileSelect->Draw(target);
	}
	if (state == S_MULTI_SCREEN || state == S_TO_MULTI_TRANS
		|| state == S_FROM_MULTI_TRANS)
	{
		//target->draw(multiProfileRow);
		target->draw(multiSelect);
		for (int i = 0; i < 4; ++i)
		{
			multiPlayerSection[i]->Draw(target);
		}

		if (multiSelectorState == MS_MUSIC)
		{
			musicSelector->Draw(target);
		}
		else if (multiSelectorState == MS_GHOST)
		{
			ghostSelector->Draw(target);
		}
	}
	
	filterOptions->Draw(target);
	progressDisplay->Draw(target);

	if (IsMusicSelectorVisible())
	{
		musicSelector->Draw(target);
	}

	if (IsGhostSelectorVisible())
	{
		ghostSelector->Draw(target);
	}
}

#include <fstream>
MapCollection::MapCollection()
{
	expanded = false;
	tags = 0;
}

OptionsMenuScreen::OptionsMenuScreen(MainMenu *p_mainMenu)
	:mainMenu(p_mainMenu)
{
	int width = 500;
	int height = 500;
	Vector2f menuOffset(0, 0);

	optionsWindow = new UIWindow(NULL, mainMenu->tilesetManager.GetTileset("Menu/windows_64x24.png", 64, 24),//owner->GetTileset( "uiwindowtest_96x30.png", 96, 30 ),/*"window_64x24.png", 64, 24*/
			Vector2f(width, height));
	optionsWindow->SetTopLeftVec(Vector2f(1920/2 - width / 2, 1080/2 - height / 2) + menuOffset);

	string options[] = { "1920 x 1080", "1600 x 900" , "1280 x 720" };
	string results[] = { "blah", "blah2" , "blah3" };
	string resolutionOptions[] = { "1920 x 1080", "1600 x 900", "1366 x 768", "1280 x 800", "1280 x 720" };
	string windowModes[] = { "Borderless Windowed", "Windowed", "Fullscreen" };

	int windowModeInts[] = { sf::Style::None, sf::Style::Default, sf::Style::Fullscreen };

	Vector2i resolutions[] = { Vector2i(1920, 1080), Vector2i(1600, 900), Vector2i( 1366, 768 ),
		Vector2i(1280, 800), Vector2i(1280, 720) };

	horizResolution = new UIHorizSelector<Vector2i>(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, 4,
		resolutionOptions, "Resolution", 200, resolutions, true, 0, 400);

	horizWindowModes = new UIHorizSelector<int>(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, 3,
		windowModes, "Window Mode", 200, windowModeInts, true, 0, 200);

	int vol[101];
	for (int i = 0; i < 101; ++i)
		vol[i] = i;

	string volStrings[101];
	for (int i = 0; i < 101; ++i)
		volStrings[i] = to_string(i);

	volume = new UIHorizSelector<int>(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, 101,
		volStrings, "Volume", 200, vol, true, 0, 200);

	defaultButton = new UIButton(NULL, this, &mainMenu->tilesetManager, &mainMenu->arial, "set to defaults", 300);
	applyButton = new UIButton(NULL, this, &mainMenu->tilesetManager, &mainMenu->arial, "apply settings", 300);

	//UICheckbox *check = new UICheckbox(NULL, NULL, &mainMenu->tilesetManager, &mainMenu->arial, "testcheckbox", 300);
	//test->SetTopLeft( Vector2f( 50, 0 ) );

	

	UIControl *testBlah[] = { horizResolution, horizWindowModes, volume, defaultButton, applyButton };

	//check->SetTopLeft(100, 50);
	UIVerticalControlList *cList = new UIVerticalControlList(optionsWindow, sizeof( testBlah ) / sizeof( UIControl* ), testBlah, 20);
	cList->SetTopLeft( 50,  50);
	optionsWindow->controls.push_back(cList);
	//optionsWindow->controls.push_back(check);
}

bool OptionsMenuScreen::ButtonEvent(UIEvent eType,
	ButtonEventParams *param)
{
	UIButton *pButton = param->button;
	if (eType == UIEvent::E_BUTTON_PRESSED)
	{
		if (pButton == defaultButton)
		{
			mainMenu->config->SetToDefault();
			Load();
		}
		else if (pButton == applyButton)
		{
			Vector2i res = horizResolution->GetResult(horizResolution->currIndex);
			int winMode = horizWindowModes->GetResult(horizWindowModes->currIndex);
			int vol = volume->GetResult(volume->currIndex);
			ConfigData d;
			d.resolutionX = res.x;
			d.resolutionY = res.y;
			d.windowStyle = winMode;
			d.volume = vol;

			mainMenu->config->SetData(d);
			Config::CreateSaveThread(mainMenu->config);
			mainMenu->config->WaitForSave();

			mainMenu->ResizeWindow(res.x, res.y, winMode);
			//mainMenu->config->
		}
	}

	return true;
}

bool OptionsMenuScreen::CheckboxEvent(UIEvent eType,
	CheckboxEventParams *param)
{
	return false;
}


bool OptionsMenuScreen::SelectorEvent(UIEvent eType,
	SelectorEventParams *param)
{
	return false;
}

void OptionsMenuScreen::Load()
{
	const ConfigData &cd = mainMenu->config->GetData();

	assert( horizResolution->SetCurrAsResult(Vector2i(cd.resolutionX, cd.resolutionY)) );
	assert( horizWindowModes->SetCurrAsResult(cd.windowStyle) );
	assert( volume->SetCurrAsResult(cd.volume) );
}

void OptionsMenuScreen::Update()
{
	if (mainMenu->menuCurrInput.B && !mainMenu->menuPrevInput.B)
	{
		mainMenu->SetMode(MainMenu::Mode::TRANS_OPTIONS_TO_MAIN);
	}

	optionsWindow->Update(mainMenu->menuCurrInput, mainMenu->menuPrevInput);
}

void OptionsMenuScreen::Draw(RenderTarget *target)
{
	optionsWindow->Draw(target);
}



CreditsMenuScreen::CreditsMenuScreen(MainMenu *p_mainMenu)
	:mainMenu( p_mainMenu )
{
	menuOffset = Vector2f(0, 0);
	ts_test = mainMenu->tilesetManager.GetTileset("Menu/power_icon_128x128.png", 128, 128);
	testSprite.setTexture(*ts_test->texture);
	testSprite.setPosition(menuOffset + Vector2f(500, 500));
}

void CreditsMenuScreen::Draw(sf::RenderTarget *target)
{
	target->draw(testSprite);
}

void CreditsMenuScreen::Update()
{
	ControllerState &menuCurrInput = mainMenu->menuCurrInput;
	ControllerState &menuPrevInput = mainMenu->menuPrevInput;

	if (menuCurrInput.B && !menuPrevInput.B)
	{
		mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN );
	}
}


LoadingMapProgressDisplay::LoadingMapProgressDisplay( MainMenu *p_mainMenu,
	Vector2f &topLeft )
	:mainMenu( p_mainMenu )
{
	text = new Text[NUM_LOAD_THREADS];
	currString = new string[NUM_LOAD_THREADS];
	for (int i = 0; i < NUM_LOAD_THREADS; ++i)
	{
		sf::Text &t = text[i];
		t.setCharacterSize(20);
		t.setFont(mainMenu->arial);
		t.setPosition(topLeft + Vector2f( 0, 30 * i + 20 * i ) );
		t.setFillColor(Color::Red);
	}
}

void LoadingMapProgressDisplay::Reset()
{
	for (int i = 0; i < NUM_LOAD_THREADS; ++i)
	{
		text[i].setString("");
		currString[i] = "";
	}
}

LoadingMapProgressDisplay::~LoadingMapProgressDisplay()
{
	delete[] text;
	delete[] currString;
}

void LoadingMapProgressDisplay::SetProgressString(const std::string & str,
	int threadIndex)
{
	stringLock.lock();
	
	currString[threadIndex] = str;
	
	stringLock.unlock();
}

void LoadingMapProgressDisplay::UpdateText()
{
	string temp;
	for (int i = 0; i < NUM_LOAD_THREADS; ++i)
	{
		stringLock.lock();
		temp.assign( currString[i] );
		stringLock.unlock();

		sf::Text &t = text[i];
		if (temp != t.getString().toAnsiString())
		{
			t.setString(temp);
		}
	}
}

void LoadingMapProgressDisplay::Draw(sf::RenderTarget *target)
{
	for (int i = 0; i < NUM_LOAD_THREADS; ++i)
	{
		target->draw(text[i]);
	}
}

void MapHeader::Save(std::ofstream &of)
{
	of << ver1 << "." << ver2 << "\n";
	of << description << "<>\n";

	of << numShards << "\n";
	for (auto it = shardNameList.begin(); it != shardNameList.end(); ++it)
	{
		of << (*it) << "\n";
	}

	of << songLevels.size() << "\n";
	for (auto it = songLevels.begin(); it != songLevels.end(); ++it)
	{
		of << (*it).first << " " << (*it).second << "\n";
	}

	of << collectionName << "\n";
	of << gameMode << "\n";

	of << (int)envType << " " << envLevel << endl;

	of << leftBounds << " " << topBounds << " " << boundsWidth << " " << boundsHeight << endl;

	//of << numVertices << endl;
}
