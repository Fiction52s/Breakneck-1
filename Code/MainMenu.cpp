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

using namespace std;
using namespace sf;
using namespace boost::filesystem;

#define TIMESTEP 1.0 / 60.0

sf::RenderTexture *MainMenu::preScreenTexture = NULL;
sf::RenderTexture *MainMenu::postProcessTexture = NULL;
sf::RenderTexture *MainMenu::postProcessTexture1 = NULL;
sf::RenderTexture *MainMenu::postProcessTexture2 = NULL;
sf::RenderTexture *MainMenu::minimapTexture = NULL;
sf::RenderTexture *MainMenu::mapTexture = NULL;
sf::RenderTexture *MainMenu::pauseTexture = NULL;
sf::RenderTexture *MainMenu::saveTexture = NULL;
sf::RenderTexture *MainMenu::mapPreviewTexture = NULL;

const int MapSelectionMenu::BOX_WIDTH = 480;
const int MapSelectionMenu::BOX_HEIGHT = 50;
const int MapSelectionMenu::BOX_SPACING = 0;

sf::Font MainMenu::arial;
int MainMenu::masterVolume = 100;

MultiSelectionSection::MultiSelectionSection(MultiLoadingScreen *p_parent,
	int p_playerIndex )
	:parent( p_parent ), playerIndex( p_playerIndex ), team( T_NOT_CHOSEN ),
	skinIndex( S_STANDARD ), isReady( false ), active( false )
{
	profileSelect = new ControlProfileMenu( this,playerIndex,
		parent->cpm->profiles );

	profileSelect->UpdateNames();	
}

bool MultiSelectionSection::ButtonEvent( UIEvent eType,
		ButtonEventParams *param )
{
	return true;
}

void MultiSelectionSection::Update()
{
	MainMenu *mm = parent->mainMenu;
	ControllerState &currInput = mm->GetCurrInput( playerIndex );
	ControllerState &prevInput = mm->GetPrevInput( playerIndex );

	if( !active )
	{
		bool a = currInput.A && !prevInput.A;
		if( a )
			active = true;
		return;
	}

	if( !isReady )
	{
		bool rT = (currInput.RightTriggerPressed() && !prevInput.RightTriggerPressed() );
		bool lT = (currInput.LeftTriggerPressed() && !prevInput.LeftTriggerPressed() );

		bool rS = (currInput.rightShoulder && !prevInput.rightShoulder);
		bool lS = (currInput.leftShoulder && !prevInput.leftShoulder);
		if( rT )
		{
			switch( team )
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
		else if( lT )
		{
			switch( team )
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

		if( rS )
		{
			if( skinIndex < S_Count )
			{
				++skinIndex;
			}
			else
			{
				skinIndex = 0;
			}
		}
		else if( lS )
		{
			if( skinIndex > 0 )
			{
				--skinIndex;
			}
			else
			{
				skinIndex = S_Count - 1;
			}
		}

		profileSelect->Update( currInput, prevInput );
	}

	int numPlayersActive = parent->GetNumActivePlayers();
	if( !isReady && currInput.start && !prevInput.start
		&& profileSelect->state == ControlProfileMenu::S_SELECTED )
	{
		/*if( team != T_NOT_CHOSEN || numPlayersActive == 2 )
		{
			if( numPlayersActive
		}*/
		isReady = true;
	}
	else if( isReady && currInput.B && !currInput.B )
	{
		isReady = false;
	}
}

void MultiSelectionSection::Draw( sf::RenderTarget *target )
{
	sf::RectangleShape rs;
	Color c;
	switch( playerIndex )
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
	rs.setFillColor( c );
	rs.setPosition( 1920 / 4 *  playerIndex, 1080 - 400 );
	rs.setSize( Vector2f( 1920/4, 400 ) );
	target->draw( rs );
	target->draw( playerSprite );
	profileSelect->Draw( target );
}

MultiLoadingScreen::MultiLoadingScreen( MainMenu *p_mainMenu )
	:mainMenu( p_mainMenu ), loadThread( NULL )
{
	cpm = new ControlProfileManager;
	cpm->LoadProfiles();

	for( int i = 0; i < 4; ++i )
	{
		playerSection[i] = new MultiSelectionSection( this, i );
	}

	gs = NULL;
	loadThread = NULL;	
}

void MultiLoadingScreen::Reset( boost::filesystem::path p_path )
{
	filePath = p_path;

	SetPreview();
	
	if( gs != NULL )
	{
		delete gs;
	}
	else if( loadThread != NULL )
	{
		delete loadThread;
	}

	gs = new GameSession( NULL, mainMenu, p_path.string() );

	loadThread = new boost::thread( GameSession::sLoad, gs );
}

void MultiLoadingScreen::SetPreview()
{
	string previewFile = filePath.parent_path().relative_path().string() + string( "/" ) + filePath.stem().string() + string( "_preview.png" );
	previewTex.loadFromFile( previewFile );
	previewSprite.setTexture( previewTex );
	previewSprite.setPosition( 480, 0 );
}

void MultiLoadingScreen::Draw( sf::RenderTarget *target )
{
	target->draw( previewSprite );
	for( int i = 0; i < 4; ++i )
	{
		playerSection[i]->Draw( target );
	}
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
			if( playerSection[i]->isReady )
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
		}	
	}
}

GameController &MainMenu::GetController( int index )
{
	return *controllers[index];
}

MainMenu::MainMenu()
	:windowWidth(1920), windowHeight( 1080 )
{
	int wholeX = 1920;
	int wholeY = 1080;
	int halfX = wholeX / 2;
	int halfY = wholeY / 2;
	trueCenter = Vector2f(halfX, halfY);
	leftCenter = Vector2f(halfX - wholeX, halfY);
	rightCenter = Vector2f(halfX + wholeX, halfY);
	topCenter = Vector2f(halfX, halfY - wholeY );
	bottomCenter = Vector2f(-halfX, halfY + wholeY );


	for( int i = 0; i < 4; ++i )
	{
		controllers[i] = new GameController( i );
	}

	fadeRect.setFillColor( Color::Black );
	fadeRect.setSize( Vector2f( 1920, 1080 ) );
	fadeRect.setPosition( 0, 0 );

	config = new Config();
	config->WaitForLoad();
	windowWidth = config->GetData().resolutionX;
	windowHeight = config->GetData().resolutionY;

	cout << "start mm constfr" << endl;
	//load a preferences file at some point for window resolution and stuff

	//could be bad cuz its static
	arial.loadFromFile( "Breakneck_Font_01.ttf" );
		
	uiView = View( sf::Vector2f( 960, 540 ), sf::Vector2f( 1920, 1080 ) );
	v = View( Vector2f( 1920/2, 1080/2 ), Vector2f( 1920, 1080 ) );


	splashFadeFrame = 0;
	splashFadeOutLength = 40;

	

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
		mapPreviewTexture->create( 960, 540 );
		mapPreviewTexture->clear();
	}

	transWorldMapFrame = 0;
	

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

	mapSelectionMenu = new MapSelectionMenu(this, Vector2f(0, 0));

	optionsMenu = new OptionsMenuScreen(this);

	saveMenu = new SaveMenuScreen(this);
}

MainMenu::~MainMenu()
{
	window->close();

	delete config;

	delete window;
	
	delete preScreenTexture;
	delete postProcessTexture;
	delete postProcessTexture1;
	delete postProcessTexture2;
	delete minimapTexture;
	delete mapTexture;

	delete soundNodeList;
}

void MainMenu::Init()
{

	cout << "init started" << endl;

	

	
	ts_splashScreen = tilesetManager.GetTileset( "Menu/splashscreen_1920x1080.png", 1920, 1080 );
	splashSprite.setTexture( *ts_splashScreen->texture );

	ts_kinTitle[0] = tilesetManager.GetTileset( "Title/kin_title_1_1216x1080.png", 1216, 1080 );
	ts_kinTitle[1] = tilesetManager.GetTileset( "Title/kin_title_2_1216x1080.png", 1216, 1080 );
	ts_kinTitle[2] = tilesetManager.GetTileset( "Title/kin_title_3_1216x1080.png", 1216, 1080 );
	ts_kinTitle[3] = tilesetManager.GetTileset( "Title/kin_title_4_1216x1080.png", 1216, 1080 );
	ts_kinTitle[4] = tilesetManager.GetTileset( "Title/kin_title_5_1216x1080.png", 1216, 1080 );
	ts_kinTitle[5] = tilesetManager.GetTileset( "Title/kin_title_6_1216x1080.png", 1216, 1080 );
	ts_kinTitle[6] = tilesetManager.GetTileset( "Title/kin_title_7_1216x1080.png", 1216, 1080 );
	
	ts_breakneckTitle = tilesetManager.GetTileset( "Title/kin_title_1920x416.png", 1920, 416 );
	ts_backgroundTitle = tilesetManager.GetTileset( "Title/title_bg_1920x1080.png", 1920, 1080 );

	
	backgroundTitleSprite.setTexture( *ts_backgroundTitle->texture );
	breakneckTitleSprite.setTexture( *ts_breakneckTitle->texture );

	soundBuffers[S_DOWN] = soundManager.GetSound( "Audio/Sounds/menu_down.ogg" );
	soundBuffers[S_UP] = soundManager.GetSound( "Audio/Sounds/menu_up.ogg" );
	soundBuffers[S_SELECT] = soundManager.GetSound( "Audio/Sounds/menu_select.ogg" );

	

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
		EditSession *es = new EditSession(window, preScreenTexture );
		//boost::filesystem::path( p_path );
		result = es->Run( p_path, lastViewCenter, lastViewSize );
		delete es;
		if( result > 0 )
			break;

		//v.setSize( 1920, 1080 );
		window->setView( v );
		GameSession *gs = new GameSession( NULL, this, p_path );
		gs->Load();
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
		gs->Load();
		result = gs->Run();
		lastViewCenter = gs->lastViewCenter;
		lastViewSize = gs->lastViewSize;
		delete gs;
		if( result > 0 )
			break;

		EditSession *es = new EditSession(window, preScreenTexture );
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
		slideStart = leftCenter;
		slideEnd = trueCenter;
		break;
	}
	case TRANS_MAIN_TO_OPTIONS:
	{
		slideCurrFrame = 0;
		slideStart = trueCenter;
		slideEnd = leftCenter;
		break;
	}
	}

	menuMode = m;
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
										GameEditLoop( toNode->GetLocalPath() );

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

#include <sfeMovie/Movie.hpp>

void MainMenu::Slide()
{
	CubicBezier bez(0, 0, 1, 1);
	float f = bez.GetValue((double)slideCurrFrame / numSlideFrames);
	v.setCenter(slideStart * (1 - f) + slideEnd * f);

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
	assert( m.openFromFile("Movie/cube.mp4") );
	m.fit(sf::FloatRect(0, 0, 1920, 1080));

	
	m.play();
#endif
	/*int moveDownFrames = 0;
	int moveUpFrames = 0;
	int moveLeftFrames = 0;
	int moveRightFrames = 0;*/
	

	gameClock.restart();

	saveTexture->setView( v );
	//menuMode = MAPSELECT;//menuMode = SPLASH;//DEBUG_RACEFIGHT_RESULTS;
	//menuMode = OPTIONS;

	SetMode(TRANS_MAIN_TO_OPTIONS);
	//menuMode = MainMenu::Mode::TRANS_MAIN_TO_OPTIONS;//MainMenu::Mode::MULTIPREVIEW;
#if defined( USE_MOVIE_TEST )
	sf::Shader sh;
	assert( sh.loadFromFile("test.frag", sf::Shader::Fragment ) );
	
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
				ControllerState &prevInput = GetPrevInput( i );
				ControllerState &currInput = GetCurrInput( i );
				GameController &c = GetController( i );

				prevInput = currInput;
				bool active = c.UpdateState();

				if( active )
				{
					currInput = c.GetState();

					menuCurrInput.A |= ( currInput.A && !prevInput.A );
					menuCurrInput.B |= ( currInput.B && !prevInput.B );
					menuCurrInput.X |= ( currInput.B && !prevInput.B );
					menuCurrInput.Y |= ( currInput.B && !prevInput.B );
					menuCurrInput.rightShoulder |= ( currInput.rightShoulder && !prevInput.rightShoulder );
					menuCurrInput.leftShoulder |= ( currInput.leftShoulder && !prevInput.leftShoulder );
					menuCurrInput.start |= ( currInput.start && !prevInput.start );
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
				gs->Load();
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
						switch( currentMenuSelect )
						{
						case M_NEW_GAME:
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
						}
					}

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
				

					if( kinTitleSpriteFrame == kinTotalFrames )
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
					}

					//cout << "kinsprite: " << trueKinFrame << endl;
					kinTitleSprite.setTexture( *(ts_kinTitle[ trueKinFrame ]->texture) );
					kinTitleSprite.setOrigin( 0, kinTitleSprite.getLocalBounds().height );

					kinTitleSpriteFrame++;	
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

					//worldMap->prevInput = worldMap->currInput;

					//worldMap->currInput = currInput;

					if( menuCurrInput.B && !menuPrevInput.B )
					{
						menuMode = MAINMENU;
						//quit = true;
						break;
					}



					//if( controller.UpdateState() )
					//{
					//	worldMap->currInput = controller.GetState();
					//	//cout << "tjhingff" << endl;
					//	//cout << "thing" << endl;
					//	//ControllerState &cs = worldMap->currInput;
					//}
					//else
					//{
					//	//cout << "tjhingff" << endl;
					//	worldMap->currInput.A = Keyboard::isKeyPressed( Keyboard::A );
					//	worldMap->currInput.B = Keyboard::isKeyPressed( Keyboard::S );

					//	bool up = Keyboard::isKeyPressed( Keyboard::Up );// || Keyboard::isKeyPressed( Keyboard::W );
					//	bool down = Keyboard::isKeyPressed( Keyboard::Down );// || Keyboard::isKeyPressed( Keyboard::S );
					//	bool left = Keyboard::isKeyPressed( Keyboard::Left );// || Keyboard::isKeyPressed( Keyboard::A );
					//	bool right = Keyboard::isKeyPressed( Keyboard::Right );

					//	worldMap->currInput.leftStickPad = 0;

					//	if( up && down )
					//	{
					//		if( worldMap->prevInput.LUp() )
					//			worldMap->currInput.leftStickPad += 1;
					//		else if( worldMap->prevInput.LDown() )
					//			worldMap->currInput.leftStickPad += ( 1 && down ) << 1;
					//	}
					//	else
					//	{
					//		worldMap->currInput.leftStickPad += 1 && up;
					//		worldMap->currInput.leftStickPad += ( 1 && down ) << 1;
					//	}

					//	if( left && right )
					//	{
					//		if( worldMap->prevInput.LLeft() )
					//		{
					//			worldMap->currInput.leftStickPad += ( 1 && left ) << 2;
					//		}
					//		else if( worldMap->prevInput.LRight() )
					//		{
					//			worldMap->currInput.leftStickPad += ( 1 && right ) << 3;
					//		}
					//	}
					//	else
					//	{
					//		worldMap->currInput.leftStickPad += ( 1 && left ) << 2;
					//		worldMap->currInput.leftStickPad += ( 1 && right ) << 3;
					//	}
					//	//worldMap->currInput.ld
					//}
				

					//cout << "worldmap" << endl;
					if( worldMap->Update( menuPrevInput, menuCurrInput ) )
					{
						worldMapUpdate = true;
					}
					else
					{

					
						//stringstream ss; 

						//size_t lastindex = file.find_last_of("."); 
						//string rawname = fullname.substr(0, lastindex); 
						//ss << "Maps/" << file;
						//cout << "-----------------------------" << endl;
						//cout << "file: " << file << endl;
						GameSession *gs = new GameSession( NULL, this, worldMap->GetSelected() );
						gs->Load();
						int result = gs->Run(  );
						delete gs;

						if( result == 0 || result == 1 )
						{
							v.setSize( 1920, 1080 );
							v.setCenter( 1920/2, 1080/ 2);
							window->setView( v );
							worldMap->state = WorldMap::PLANET_AND_SPACE;
							worldMap->frame = 0;
							worldMap->UpdateMapList();
						}
						else if( result == 2 )
						{
							v.setSize( 1920, 1080 );
							v.setCenter( 1920/2, 1080/ 2);
							window->setView( v );
							menuMode = MainMenu::MAINMENU;
						}
						else if( result == 3 )
						{
							quit = true;
							break;
						}

					
						continue;
					}
				
					break;
				}
			case SAVEMENU:
				{
				
				saveMenu->Update();
					//parBack->Update( offset0 );
					//parFront->Update( offset1 );

					//backPar->Update( 

					break;
				}
			case TRANS_MAIN_TO_SAVE:
				break;
			case TRANS_SAVE_TO_MAIN:
				break;
			case TRANS_SAVE_TO_WORLDMAP:
				{
				
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
			case TRANS_MAIN_TO_MULTIPREVIEW:
				{
					break;
				}
			case TRANS_MULTIPREVIEW_TO_MAIN:
				{
					break;
				}
			case TRANS_MAIN_TO_MAPSELECT:
				{
					break;
				}
			case MAPSELECT:
				{
				mapSelectionMenu->Update(menuCurrInput, menuPrevInput);
					break;
				}
			case TRANS_MAIN_TO_OPTIONS:
			{
				if (slideCurrFrame > numSlideFrames)
				{
					menuMode = OPTIONS;
				}
				else
				{
					Slide();
				}
				break;
			}
			case OPTIONS:
			{
				optionsMenu->Update();
				break;
			}
			case TRANS_OPTIONS_TO_MAIN:
			{
				if (slideCurrFrame > numSlideFrames)
				{
					menuMode = OPTIONS;
				}
				else
				{
					Slide();
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
				preScreenTexture->setView( v );
				preScreenTexture->draw( backgroundTitleSprite );
				preScreenTexture->draw( kinTitleSprite );
				preScreenTexture->draw( breakneckTitleSprite );

				preScreenTexture->draw( betaText );
				
				//window->draw( titleSprite );
				//preScreenTexture->draw( menu );	

				preScreenTexture->setView( uiView );

				if( splashFadeFrame <= splashFadeOutLength )
				{
					preScreenTexture->draw( fadeRect );
				}
				

				//for( int i = 0; i < 5; ++i )
				//{
				//	if( i == currentMenuSelect )
				//	{
				//		mainMenu[i].setColor( Color::White );
				//	}
				//	else
				//	{
				//		mainMenu[i].setColor( Color::Red );
				//	}
				//	//cout << "drawing i: " << i <<  endl;
				//	preScreenTexture->draw( mainMenu[i] );
				//}
				break;
			}
		case WORLDMAP:
			{
				if( worldMapUpdate )
				{
					preScreenTexture->setView( v );
					worldMap->Draw( preScreenTexture );
				}
			}
			break;
		case TRANS_SAVE_TO_WORLDMAP:
		case SAVEMENU:
			{
			///	preScreenTexture->draw( worldMap->
				
			saveMenu->Draw(preScreenTexture);

				
				
				break;
			}
		case MULTIPREVIEW:
			{
				multiLoadingScreen->Draw( preScreenTexture );
				break;
			}
		case MAPSELECT:
			{
			mapSelectionMenu->Draw( preScreenTexture );
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
			v.setCenter(-960, 540);
			preScreenTexture->setView(v);
			optionsMenu->Draw(preScreenTexture);
			break;
		}

		}
		//window->pushGLStates();
		
		

		
		//window->popGLStates();
		
		//window->setView( window->getDefaultView() );
		
		
		//window->setView( //window->getDefaultView() );

		
		
		//prim.DrawTetrahedron( window );

		
		//prim.Draw2( window );

		//window->pushGLStates();
		
		

		//window->popGLStates();

		if( menuMode == SAVEMENU || menuMode == TRANS_SAVE_TO_WORLDMAP )
		{
			saveTexture->display();
			sf::Sprite saveSpr;
			saveSpr.setTexture( saveTexture->getTexture() );

			if( menuMode == TRANS_SAVE_TO_WORLDMAP )
			{
				saveSpr.setColor( Color( 255, 255, 255, transAlpha ) );
			}
			preScreenTexture->draw( saveSpr );
		}

		//preScreenTexture->draw( ff, 4, sf::Quads,  &sh );
#if defined( USE_MOVIE_TEST )
		preScreenTexture->draw(m, &sh);
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

	//window->res
	//window->close();
	window->create( VideoMode( windowWidth, windowHeight ), "Breakneck", style );
	View blahV;
	blahV.setCenter( 0, 0 );
	blahV.setSize( 1920/ 2, 1080 / 2 );
	//v.setCenter( 960, 540 );
	window->setView( blahV );
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
			gs->Load();
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
	:mainMenu( p_mainMenu ), font( p_mainMenu->arial ), topIndex( 0 ), currIndex( 0 ),
	oldCurrIndex( 0 )
{
	topMid = p_pos + Vector2f( BOX_WIDTH / 2, 0 );

	SetupBoxes();

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		itemName[i].setFont(font);
		itemName[i].setCharacterSize(40);
		itemName[i].setFillColor(Color::White);
	}

	waitFrames[0] = 10;
	waitFrames[1] = 5;
	waitFrames[2] = 2;

	waitModeThresh[0] = 2;
	waitModeThresh[1] = 2;

	currWaitLevel = 0;
	flipCounterUp = 0;
	flipCounterDown = 0;
	framesWaiting = 0;

	LoadItems();

	UpdateItemText();
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
void MapSelectionMenu::LoadItems()
{
	//path p(current_path() / "/Maps/");

	//vector<path> v;
	//try
	//{
	//	if (exists(p))    // does p actually exist?
	//	{
	//		if (is_regular_file(p))        // is p a regular file?   
	//		{
	//			if (p.extension().string() == ".brknk")
	//			{
	//				//string name = p.filename().string();
	//				parentNode->files.push_back(p);//name.substr( 0, name.size() - 6 ) );
	//				numTotalEntries++;
	//			}
	//		}
	//		else if (is_directory(p))      // is p a directory?
	//		{
	//			//cout << p << " is a directory containing:\n";

	//			TreeNode *newDir = new TreeNode;
	//			newDir->parent = parentNode;
	//			newDir->next = NULL;
	//			newDir->name = p.filename().string();
	//			newDir->filePath = p;

	//			copy(directory_iterator(p), directory_iterator(), back_inserter(v));

	//			sort(v.begin(), v.end());             // sort, since directory iteration
	//												  // is not ordered on some file systems

	//			if (parentNode == NULL)
	//			{
	//				entries = newDir;
	//			}
	//			else
	//			{
	//				parentNode->dirs.push_back(newDir);
	//			}
	//			numTotalEntries++;


	//			for (vector<path>::const_iterator it(v.begin()); it != v.end(); ++it)
	//			{
	//				UpdateMapList(newDir, relativePath + "/" + (*it).filename().string());
	//				cout << "   " << *it << '\n';
	//			}
	//		}
	//		else
	//			cout << p << " exists, but is neither a regular file nor a directory\n";
	//	}
	//	else
	//		cout << p << " does not exist\n";
	//}
	//catch (const filesystem_error& ex)
	//{
	//	cout << ex.what() << '\n';
	//}













	/*MapCollection *mc = new MapCollection;
	if (!mc->Load("Maps/testcollection.col"))
	{
		assert(0);
	}

	MapSelectionItem( "Maps/testcollection.col", )

	items.push_back(mc);*/
	/*items.push_back(new MapSelectionItem( "one", MapSelectionItem::F_CONTAINER ) );
	items.push_back(new MapSelectionItem("two", MapSelectionItem::F_MAP));
	items.push_back(new MapSelectionItem("three", MapSelectionItem::F_MAP));
	items.push_back(new MapSelectionItem("four", MapSelectionItem::F_MAP));
	items.push_back(new MapSelectionItem("five", MapSelectionItem::F_MAP));*/

	currItemIt = items.begin();
}

void MapSelectionMenu::Update(ControllerState &currInput,
	ControllerState &prevInput)
{
	bool up = currInput.LUp();
	bool down = currInput.LDown();

	int oldIndex = currIndex;
	//bool consumed = controls[focusedIndex]->Update( curr, prev );

	if (down)
	{
		if (flipCounterDown == 0
			|| (flipCounterDown > 0 && framesWaiting == waitFrames[currWaitLevel])
			)
		{
			if (flipCounterDown == 0)
			{
				currWaitLevel = 0;
			}

			++flipCounterDown;

			if (flipCounterDown == waitModeThresh[currWaitLevel] && currWaitLevel < 2)
			{
				currWaitLevel++;
			}

			flipCounterUp = 0;
			framesWaiting = 0;

			if (currIndex < items.size() - 1)
			{
				currIndex++;
				++currItemIt;

				if (currIndex - topIndex == NUM_BOXES)
				{
					topIndex = currIndex - (NUM_BOXES - 1);
				}
			}
			else
			{
				currIndex = 0;
				topIndex = 0;

				assert(items.size() > 0);
				currItemIt = items.begin();
			}
		}
		else
		{
			++framesWaiting;
		}

	}
	else if (up)
	{
		if (flipCounterUp == 0
			|| (flipCounterUp > 0 && framesWaiting == waitFrames[currWaitLevel])
			)
		{
			if (flipCounterUp == 0)
			{
				currWaitLevel = 0;
			}

			++flipCounterUp;

			if (flipCounterUp == waitModeThresh[currWaitLevel] && currWaitLevel < 2)
			{
				currWaitLevel++;
			}

			flipCounterDown = 0;
			framesWaiting = 0;
			if (currIndex > 0)
			{
				currIndex--;
				currItemIt--;

				if (currIndex < topIndex)
				{
					topIndex = currIndex;
				}
			}
			else
			{
				assert(items.size() > 0);
				currItemIt = items.end();
				--currItemIt;

				currIndex = items.size() - 1;
				topIndex = items.size() - NUM_BOXES;
			}
		}
		else
		{
			++framesWaiting;
		}

	}
	else
	{
		flipCounterUp = 0;
		flipCounterDown = 0;
		currWaitLevel = 0;
		framesWaiting = 0;
	}

	if (currIndex != oldIndex)
	{
		UpdateItemText();
		//cout << "currIndex: " << currIndex << ", topIndex: " << topIndex << endl;
		//controls[oldIndex]->Unfocus();
		//controls[focusedIndex]->Focus();
	}
	UpdateBoxesDebug();
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
	if (items.size() == 0)
	{
		return;
	}


	list<MapSelectionItem*>::iterator lit = items.begin();
	if (topIndex > items.size())
	{
		topIndex = items.size() - 1;
	}

	for (int i = 0; i < topIndex; ++i)
	{
		++lit;
	}

	int trueI;
	int i = 0;
	int numProfiles = items.size();
	for (; i < NUM_BOXES; ++i)
	{
		trueI = (topIndex + i) % NUM_BOXES;
		if (i == numProfiles)
		{
			for (; i < NUM_BOXES; ++i)
			{
				itemName[i].setString("");
			}
			break;
		}

		if (lit == items.end())
			lit = items.begin();
		string printStr = (*lit)->path.filename().stem().string().c_str();
		itemName[i].setString(printStr);
		itemName[i].setOrigin(itemName[i].getLocalBounds().width / 2, 0);
		itemName[i].setPosition(topMid.x, topMid.y + (BOX_HEIGHT + BOX_SPACING) * i);

		++lit;
	}
}

void MapSelectionMenu::UpdateBoxesDebug()
{
	Color c;
	int trueI = (currIndex - topIndex);// % NUM_BOXES;
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

void MapSelectionMenu::Draw(sf::RenderTarget *target)
{
	target->draw(boxes, NUM_BOXES * 4, sf::Quads);
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		target->draw(itemName[i]);
	}
}

#include <fstream>
MapCollection::MapCollection()
{
	tags = 0;
}

bool MapCollection::Load( boost::filesystem::path path )
{
	ifstream is;
	is.open(path.string());
	if (is.is_open())
	{
		string map;
		while (cin >> map)
		{
			map += string(".brknk");
			maps.push_back(new MapSelectionItem(map, MapSelectionItem::FileType::F_MAP));
		}


		return true;
	}
	else
	{
		assert(0);
		return false;
	}
}

OptionsMenuScreen::OptionsMenuScreen(MainMenu *p_mainMenu)
	:mainMenu(p_mainMenu)
{
	int width = 500;
	int height = 500;
	Vector2f menuOffset(-1920, 0);

	optionsWindow = new UIWindow(NULL, mainMenu->tilesetManager.GetTileset("Menu/windows_64x24.png", 64, 24),//owner->GetTileset( "uiwindowtest_96x30.png", 96, 30 ),/*"window_64x24.png", 64, 24*/
			Vector2f(width, height));
	optionsWindow->SetTopLeftVec(Vector2f(1920/2 - width / 2, 1080/2 - height / 2) + menuOffset);
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

SaveMenuScreen::SaveMenuScreen(MainMenu *p_mainMenu)
	:mainMenu( p_mainMenu )
{
	TilesetManager &tsMan = mainMenu->tilesetManager;

	kinFaceFrame = 0;
	kinFaceTurnLength = 15;
	selectedSaveIndex = 0;
	cloudLoopLength = 8;
	cloudLoopFactor = 5;

	saveJumpFactor = 5;
	saveJumpLength = 6;

	files[0] = new SaveFile("blue");
	files[1] = new SaveFile("green");
	files[2] = new SaveFile("yellow");
	files[3] = new SaveFile("orange");
	files[4] = new SaveFile("red");
	files[5] = new SaveFile("magenta");

	ts_background = tsMan.GetTileset("Menu/save_bg_1920x1080.png", 1920, 1080);
	ts_kinFace = tsMan.GetTileset("Menu/save_menu_kin_256x256.png", 256, 256);
	ts_selectSlot = tsMan.GetTileset("Menu/save_select_710x270.png", 710, 270);

	background.setTexture(*ts_background->texture);
	kinFace.setTexture(*ts_kinFace->texture);
	kinFace.setTextureRect(ts_kinFace->GetSubRect(0));
	selectSlot.setTexture(*ts_selectSlot->texture);
	selectSlot.setTextureRect(ts_selectSlot->GetSubRect(0));

	ts_kinJump1 = tsMan.GetTileset("Menu/save_kin_jump1_500x1080.png", 500, 1080);
	ts_kinJump2 = tsMan.GetTileset("Menu/save_kin_jump2_500x1080.png", 500, 1080);
	ts_kinClouds = tsMan.GetTileset("Menu/save_kin_clouds_500x384.png", 500, 384);
	ts_kinWindow = tsMan.GetTileset("Menu/save_kin_window_500x1080.png", 500, 1080);
	//ts_saveKinWindow = tilesetManager.GetTileset( "Menu/save_kin_window_500x1080.png", 500, 1080 );
	ts_kinSky = tsMan.GetTileset("Menu/save_menu_sky_01_500x1080.png", 500, 1080);

	kinClouds.setTexture(*ts_kinClouds->texture);
	kinClouds.setTextureRect(ts_kinClouds->GetSubRect(0));
	kinClouds.setOrigin(kinClouds.getLocalBounds().width, kinClouds.getLocalBounds().height);
	kinClouds.setPosition(1920, 1080);

	kinWindow.setTexture(*ts_kinWindow->texture);
	kinWindow.setOrigin(kinWindow.getLocalBounds().width, 0);
	kinWindow.setPosition(1920, 0);

	kinSky.setTexture(*ts_kinSky->texture);
	kinSky.setOrigin(kinSky.getLocalBounds().width, 0);
	kinSky.setPosition(1920, 0);
	//saveKinJump.setTexture( ts_saveKin

	cloudFrame = 0;

	ts_starBackground = tsMan.GetTileset("WorldMap/map_z1_stars.png", 1920, 1080);
	starBackground.setTexture(*ts_starBackground->texture);

	ts_world = tsMan.GetTileset("WorldMap/map_z1_world.png", 1120, 1080);
	world.setTexture(*ts_world->texture);
	world.setOrigin(world.getLocalBounds().width / 2, world.getLocalBounds().height / 2);
	world.setPosition(960, 540);

	Tileset *ts_asteroid0 = tsMan.GetTileset("Menu/w0_asteroid_01_960x1080.png", 960, 1080);
	Tileset *ts_asteroid1 = tsMan.GetTileset("Menu/w0_asteroid_02_1920x1080.png", 1920, 1080);
	Tileset *ts_asteroid2 = tsMan.GetTileset("Menu/w0_asteroid_03_1920x1080.png", 1920, 1080);

	asteroid0.setTexture(*ts_asteroid0->texture);
	asteroid1.setTexture(*ts_asteroid1->texture);
	asteroid2.setTexture(*ts_asteroid2->texture);

	asteroid0.setPosition(0, 0);
	asteroid1.setPosition(0, 0);
	asteroid2.setPosition(0, 0);

	a0start = Vector2f(-1920, 0);
	a0end = Vector2f(1920, 0);

	a1start = Vector2f(1920, 0);
	a1end = Vector2f(-1920, 0);

	a2start = Vector2f(-1920, 0);
	a2end = Vector2f(1920, 0);

	asteroidScrollFrames0 = 2000;
	asteroidScrollFrames1 = 500;
	asteroidScrollFrames2 = 120;

	asteroidFrameBack = asteroidScrollFrames0 / 2;
	asteroidFrameFront = asteroidScrollFrames1 / 2;
}

void SaveMenuScreen::Update()
{
	ControllerState &menuCurrInput = mainMenu->menuCurrInput;
	ControllerState &menuPrevInput = mainMenu->menuPrevInput;

	bool moveDown = false;
	bool moveUp = false;
	bool moveLeft = false;
	bool moveRight = false;

	
	int moveDelayFrames = 15;
	int moveDelayFramesSmall = 6;

	if (menuCurrInput.B && !menuPrevInput.B)
	{
		mainMenu->menuMode = MainMenu::MAINMENU;
		return;
	}
	else if (menuCurrInput.A && !menuPrevInput.A)
	{
		mainMenu->menuMode = MainMenu::Mode::TRANS_SAVE_TO_WORLDMAP;
		mainMenu->transAlpha = 255;
		mainMenu->worldMap->state = WorldMap::PLANET;//WorldMap::PLANET_AND_SPACE;
		mainMenu->worldMap->frame = 0;
		mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_SELECT]);
		return;
	}

	bool canMoveOther = ((moveDelayCounter - moveDelayFramesSmall) <= 0);
	bool canMoveSame = (moveDelayCounter == 0);
	if ((menuCurrInput.LDown() || menuCurrInput.PDown()) && (
		(!moveDown && canMoveOther) || (moveDown && canMoveSame)))
	{
		selectedSaveIndex += 2;
		//currentMenuSelect++;
		if (selectedSaveIndex > 5)
			selectedSaveIndex -= 6;
		moveDown = true;
		moveDelayCounter = moveDelayFrames;
		mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_DOWN]);
	}
	else if ((menuCurrInput.LUp() || menuCurrInput.PUp()) && (
		(!moveUp && canMoveOther) || (moveUp && canMoveSame)))
	{
		selectedSaveIndex -= 2;
		if (selectedSaveIndex < 0)
			selectedSaveIndex += 6;
		moveUp = true;
		moveDelayCounter = moveDelayFrames;
		mainMenu->soundNodeList->ActivateSound(mainMenu->soundBuffers[MainMenu::S_UP]);
	}

	if ((menuCurrInput.LRight() || menuCurrInput.PRight()) && (
		(!moveRight && canMoveOther) || (moveRight && canMoveSame)))
	{
		selectedSaveIndex++;
		//currentMenuSelect++;
		if (selectedSaveIndex % 2 == 0)
			selectedSaveIndex -= 2;
		moveRight = true;
		moveDelayCounter = moveDelayFrames;
	}
	else if ((menuCurrInput.LLeft() || menuCurrInput.PLeft()) && (
		(!moveLeft && canMoveOther) || (moveLeft && canMoveSame)))
	{
		selectedSaveIndex--;
		if (selectedSaveIndex % 2 == 1)
			selectedSaveIndex += 2;
		else if (selectedSaveIndex < 0)
		{
			selectedSaveIndex += 2;
		}
		moveLeft = true;

		moveDelayCounter = moveDelayFrames;
	}

	if (moveDelayCounter > 0)
	{
		moveDelayCounter--;
	}


	if (!(menuCurrInput.LDown() || menuCurrInput.PDown()))
	{
		moveDown = false;
	}
	if (!(menuCurrInput.LUp() || menuCurrInput.PUp()))
	{
		moveUp = false;
	}

	if (!(menuCurrInput.LRight() || menuCurrInput.PRight()))
	{
		moveRight = false;
	}
	if (!(menuCurrInput.LLeft() || menuCurrInput.PLeft()))
	{
		moveLeft = false;
	}

	selectSlot.setTextureRect(ts_selectSlot->GetSubRect(selectedSaveIndex));
	kinFace.setTextureRect(ts_kinFace->GetSubRect(0));

	Vector2f topLeftPos;
	topLeftPos.x += ts_selectSlot->tileWidth * (selectedSaveIndex % 2);
	topLeftPos.y += ts_selectSlot->tileHeight * (selectedSaveIndex / 2);

	selectSlot.setPosition(topLeftPos);
	kinFace.setPosition(topLeftPos);

	UpdateClouds();
	++asteroidFrameBack;
	++asteroidFrameFront;

	int r0 = asteroidFrameBack % asteroidScrollFrames0;
	int r1 = asteroidFrameFront % asteroidScrollFrames1;
	//int r2 = asteroidFrame % asteroidScrollFrames2;

	Vector2f offset0(0, 0);
	Vector2f offset1(0, 0);
	CubicBezier bez(0, 0, 1, 1);
	double v = bez.GetValue(r0 / (double)asteroidScrollFrames0);
	double v1 = bez.GetValue(r1 / (double)asteroidScrollFrames1);
	offset0.x = 1920 * 3 * v;
	offset1.x = -1920 * v1;

	//cout << "asteroidframe: " << asteroidFrame << ", offset0: " << offset0.x << ", offset1: " << offset1.x << endl;


	asteroid0.setPosition(a0start * (float)(1.0 - v1) + a0end * (float)v1);
	asteroid1.setPosition(a1start * (float)(1.0 - v) + a1end * (float)v);
	asteroid2.setPosition(a2start * (float)(1.0 - v1) + a2end * (float)v1);
}

void SaveMenuScreen::Draw(sf::RenderTarget *target)
{
	RenderTexture *saveTexture = mainMenu->saveTexture;

	target->draw(starBackground);
	
	target->draw(asteroid1);
	target->draw(world);
	target->draw(asteroid0);
	target->draw(asteroid2);

	target->setView(mainMenu->v);

	saveTexture->clear(Color::Transparent);
	saveTexture->setView(mainMenu->v);
	saveTexture->draw(background);
	saveTexture->draw(kinSky);
	saveTexture->draw(kinClouds);
	saveTexture->draw(kinWindow);
	//TODO
	//if (menuMode == SAVEMENU ||
	//	kinFaceFrame < saveJumpLength * saveJumpFactor)
	saveTexture->draw(kinJump);

	saveTexture->draw(selectSlot);
	saveTexture->draw(kinFace);
}

void SaveMenuScreen::Reset()
{
	selectedSaveIndex = 0;
	asteroidFrameBack = 0;
	asteroidFrameFront = 0;
	moveDelayCounter = 0;

	kinJump.setTexture(*ts_kinJump1->texture);
	kinJump.setTextureRect(ts_kinJump1->GetSubRect(0));
	kinJump.setOrigin(kinJump.getLocalBounds().width, 0);
	kinJump.setPosition(1920, 0);

	for (int i = 0; i < 6; ++i)
	{
		files[i]->LoadFromFile();
	}
}

void SaveMenuScreen::UpdateClouds()
{
	if (cloudFrame == cloudLoopLength * cloudLoopFactor)
	{
		cloudFrame = 0;
	}

	int f = cloudFrame / cloudLoopFactor;

	//cout << "cloud frame: " << f << endl;
	kinClouds.setTextureRect(ts_kinClouds->GetSubRect(f));

	cloudFrame++;
}