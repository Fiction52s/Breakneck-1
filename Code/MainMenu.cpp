#include "MainMenu.h"
#include <boost/filesystem.hpp>
#include "EditSession.h"
#include "GameSession.h"
#include "SaveFile.h"
#include <iostream>
#include <sstream>

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

sf::Font *MainMenu::arial = NULL;


MainMenu::MainMenu()
	:controller( 0 ), windowWidth(1920), windowHeight( 1080 )
{
	if( arial == NULL )
	{
		arial = new Font();
		arial->loadFromFile( "arial.ttf" );
	}
		
	uiView = View( sf::Vector2f( 960, 540 ), sf::Vector2f( 1920, 1080 ) );
	v = View( Vector2f( 1920/2, 1080/2 ), Vector2f( 1920, 1080 ) );

	selectCreateNew = false;

	files[0] = new SaveFile( "blue" );
	files[1] = new SaveFile( "green" );
	files[2] = new SaveFile( "yellow" );
	files[3] = new SaveFile( "orange" );
	files[4] = new SaveFile( "red" );
	files[5] = new SaveFile( "magenta" );

	soundNodeList = new SoundNodeList( 10 );
	soundNodeList->SetGlobalVolume( 100 );

	menuMode = MAINMENU;

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
		minimapTexture->create( 300, 300 );
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

	saveKinFaceFrame = 0;
	saveKinFaceTurnLength = 15;

	worldMap = new WorldMap( this );
	levelSelector = new LevelSelector( this );

	selectedSaveIndex = 0;

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

	if( !fullWindow )
	{
		window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
			sf::VideoMode( windowWidth, windowHeight ), "Breakneck", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ));
		window->setPosition( Vector2i(800, 0 ));
	}
	else
	{
		std::vector<sf::VideoMode> i = sf::VideoMode::getFullscreenModes();
		VideoMode vm( 1600, 900 );
        //sf::RenderWindow window(i.front(), "SFML WORKS!", sf::Style::Fullscreen);
		//window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
		//	sf::VideoMode( 1920 / 1, 1079 / 1), "Breakneck", sf::Style::Fullscreen, sf::ContextSettings( 0, 0, 0, 0, 0 ));
		//window = new sf::RenderWindow( VideoMode( 1920, 1080 ), "Breakneck", sf::Style::None);
		
		
		window = new sf::RenderWindow( i.front(), "Breakneck", sf::Style::None);
		//window = new sf::RenderWindow( vm, "Breakneck", sf::Style::None);

			//sf::VideoMode( 1920 / 1, 1080 / 1), "Breakneck", sf::Style::Fullscreen, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	}

	window->setVerticalSyncEnabled( true );
	std::cout << "opened window" << endl;
	
	window->setView( v );

	betaText.setString( " Press any button to start \n For help and info check \n - beta_info.txt\n Breakneck Beta\n Updated 9/2/2016");
	betaText.setCharacterSize( 20 );
	betaText.setColor( Color::Red );
	betaText.setPosition( 50, 200 );
	betaText.setFont( *arial );

	Init();
}

MainMenu::~MainMenu()
{
	

	window->close();

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
	ts_saveMenuBG = tilesetManager.GetTileset( "Menu/save_bg_1920x1080.png", 1920, 1080 );
	ts_saveMenuKinFace = tilesetManager.GetTileset( "Menu/save_menu_kin_256x256.png", 256, 256 );
	ts_saveMenuSelect = tilesetManager.GetTileset( "Menu/save_select_710x270.png", 710, 270 );

	ts_kinTitle[0] = tilesetManager.GetTileset( "Title/kin_title_1_1216x1080.png", 1216, 1080 );
	ts_kinTitle[1] = tilesetManager.GetTileset( "Title/kin_title_2_1216x1080.png", 1216, 1080 );
	ts_kinTitle[2] = tilesetManager.GetTileset( "Title/kin_title_3_1216x1080.png", 1216, 1080 );
	ts_kinTitle[3] = tilesetManager.GetTileset( "Title/kin_title_4_1216x1080.png", 1216, 1080 );
	ts_kinTitle[4] = tilesetManager.GetTileset( "Title/kin_title_5_1216x1080.png", 1216, 1080 );
	ts_kinTitle[5] = tilesetManager.GetTileset( "Title/kin_title_6_1216x1080.png", 1216, 1080 );
	ts_kinTitle[6] = tilesetManager.GetTileset( "Title/kin_title_7_1216x1080.png", 1216, 1080 );
	
	ts_breakneckTitle = tilesetManager.GetTileset( "Title/kin_title_1920x416.png", 1920, 416 );
	ts_backgroundTitle = tilesetManager.GetTileset( "Title/title_bg_1920x1080.png", 1920, 1080 );

	saveBG.setTexture( *ts_saveMenuBG->texture );
	saveKinFace.setTexture( *ts_saveMenuKinFace->texture );
	saveKinFace.setTextureRect( ts_saveMenuKinFace->GetSubRect( 0 ) );
	saveSelect.setTexture( *ts_saveMenuSelect->texture );
	backgroundTitleSprite.setTexture( *ts_backgroundTitle->texture );
	breakneckTitleSprite.setTexture( *ts_breakneckTitle->texture );

	soundBuffers[S_DOWN] = soundManager.GetSound( "Audio/Sounds/menu_down.ogg" );
	soundBuffers[S_UP] = soundManager.GetSound( "Audio/Sounds/menu_up.ogg" );
	soundBuffers[S_SELECT] = soundManager.GetSound( "Audio/Sounds/menu_select.ogg" );
}

void MainMenu::GameEditLoop( const std::string &filename)
{
	int result = 0;

	Vector2f lastViewSize( 0, 0 );
	Vector2f lastViewCenter( 0, 0 );
	while( result == 0 )
	{
		EditSession *es = new EditSession(window, preScreenTexture );
		result = es->Run( filename, lastViewCenter, lastViewSize );
		delete es;
		if( result > 0 )
			break;

		//v.setSize( 1920, 1080 );
		window->setView( v );
		GameSession *gs = new GameSession( controller, NULL, this );
		
		result = gs->Run( filename );
		lastViewCenter = gs->lastViewCenter;
		lastViewSize = gs->lastViewSize;
		delete gs;
	}
}

void MainMenu::GameEditLoop2( const std::string &filename)
{
	int result = 0;

	Vector2f lastViewSize( 0, 0 );
	Vector2f lastViewCenter( 0, 0 );
	while( result == 0 )
	{
		window->setView( v );
		GameSession *gs = new GameSession( controller, NULL, this );
		result = gs->Run( filename );
		lastViewCenter = gs->lastViewCenter;
		lastViewSize = gs->lastViewSize;
		delete gs;
		if( result > 0 )
			break;

		EditSession *es = new EditSession(window, preScreenTexture );
		result = es->Run( filename, lastViewCenter, lastViewSize );
		delete es;
	}
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
								
								if( ls.text[ls.selectedIndex].getColor() == Color::Red )
								{
									path from( "Maps/empty.brknk" );
									string toString = ls.localPaths[ls.selectedIndex] + ls.newLevelName + ".brknk";
									path to( toString );

									try 
									{
										boost::filesystem::copy_file( from, to, copy_option::fail_if_exists );

										GameEditLoop( ls.localPaths[ls.selectedIndex] + ls.newLevelName + ".brknk" );

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

										//cout << "copying to: " << to.string() << endl;
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
								
								if( ls.text[ls.selectedIndex].getColor() == Color::Blue )
								{
									path from( "Maps/empty.brknk" );
									string toString = ls.localPaths[ls.selectedIndex] + ls.newLevelName + ".brknk";
									path to( toString );

									try 
									{
										boost::filesystem::copy_file( from, to, copy_option::fail_if_exists );

										GameEditLoop( ls.localPaths[ls.selectedIndex] + ls.newLevelName + ".brknk" );

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
							if( ls.text[ls.selectedIndex].getColor() == Color::Red )
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

void MainMenu::Run()
{
	sf::Event ev;

	bool quit = false;

	bool worldMapUpdate = false;
	double currentTime = 0;
	double accumulator = TIMESTEP + .1;

	bool moveDown = false;
	bool moveUp = false;
	bool moveLeft = false;
	bool moveRight = false;

	/*int moveDownFrames = 0;
	int moveUpFrames = 0;
	int moveLeftFrames = 0;
	int moveRightFrames = 0;*/
	int moveDelayCounter = 0;
	int moveDelayFrames = 15;
	int moveDelayFramesSmall = 6;

	gameClock.restart();


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
		
		
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

		while ( accumulator >= TIMESTEP  )
        {
			worldMapUpdate = false;

			

			prevInput = currInput;
			controller.UpdateState();
			currInput = controller.GetState();

			switch( menuMode )
		{
		case MAINMENU:
			{
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

				if( currInput.B && !prevInput.B )
				{
					quit = true;
					break;
				}

				if( currInput.A || currInput.back || currInput.Y || currInput.X || 
					currInput.rightShoulder || currInput.leftShoulder )
				{
					switch( currentMenuSelect )
					{
					case M_NEW_GAME:
						{
							menuMode = SAVEMENU;
							selectedSaveIndex = 0;
							saveKinFaceFrame = 0;
							moveDelayCounter = 0;
							selectCreateNew = true;
							for( int i = 0; i < 6; ++i )
							{
								files[i]->LoadFromFile();
							}
							break;
						}
					case M_CONTINUE:
						{
							menuMode = SAVEMENU;
							selectedSaveIndex = 0;
							saveKinFaceFrame = 0;
							moveDelayCounter = 0;
							selectCreateNew = false;
							for( int i = 0; i < 6; ++i )
							{
								files[i]->LoadFromFile();
							}
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

				bool canMoveSame = (moveDelayCounter == 0);

				
				if( (currInput.LDown() || currInput.PDown()) && ( !moveDown || canMoveSame ) )
				{
					currentMenuSelect++;
					if( currentMenuSelect == M_Count )
						currentMenuSelect = 0;
					//moveDown = true;
					moveDelayCounter = moveDelayFrames;
				}
				else if( ( currInput.LUp() || currInput.PUp() ) && ( !moveUp || canMoveSame ) )
				{
					currentMenuSelect--;
					if( currentMenuSelect < 0 )
						currentMenuSelect = M_Count - 1;
					//moveUp = true;
					moveDelayCounter = moveDelayFrames;
				}
				else
				{
				}
				

				if( moveDelayCounter > 0 )
				{
					moveDelayCounter--;
				}
				
				if( !(currInput.LDown() || currInput.PDown()) )
					{
						moveDelayCounter = 0;
						moveDown = false;
					}
					else if( ! ( currInput.LUp() || currInput.PUp() ) )
					{
						moveDelayCounter = 0;
						moveUp = false;
					}
				

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

				if( currInput.B && !prevInput.B )
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
				if( worldMap->Update( prevInput, currInput ) )
				{
					worldMapUpdate = true;
				}
				else
				{
					string file = worldMap->GetSelected();
					stringstream ss; 
					ss << "Maps/" << file;
					//cout << "-----------------------------" << endl;
					//cout << "file: " << file << endl;
					GameSession *gs = new GameSession( controller, NULL, this );
					int result = gs->Run( ss.str() );
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
				if( currInput.B && !prevInput.B )
				{
					menuMode = MAINMENU;
					break;
				}
				else if( currInput.A && !prevInput.A )
				{
					/*GameSession * gs = new GameSession( controller, window, 
						files[selectedSaveIndex], preScreenTexture, postProcessTexture,
						postProcessTexture1, postProcessTexture2, minimapTexture, mapTexture );
					gs->Run(  );

					delete gs;*/

					menuMode = Mode::TRANS_SAVE_TO_WORLDMAP;
					worldMap->state = WorldMap::PLANET_AND_SPACE;//WorldMap::PLANET_AND_SPACE;
					worldMap->frame = 0;
					worldMap->UpdateMapList();
					soundNodeList->ActivateSound( soundBuffers[S_SELECT] );
					break;
				}

				bool canMoveOther = ((moveDelayCounter - moveDelayFramesSmall) <= 0);
				bool canMoveSame = (moveDelayCounter == 0);
				if( (currInput.LDown() || currInput.PDown()) && ( 
					(!moveDown && canMoveOther) || ( moveDown && canMoveSame ) ) )
				{
					selectedSaveIndex+=2;
					//currentMenuSelect++;
					if( selectedSaveIndex > 5 )
						selectedSaveIndex -= 6;
					moveDown = true;
					moveDelayCounter = moveDelayFrames;
					soundNodeList->ActivateSound( soundBuffers[S_DOWN] );
				}
				else if( ( currInput.LUp() || currInput.PUp() ) && ( 
					(!moveUp && canMoveOther) || ( moveUp && canMoveSame ) ) )
				{
					selectedSaveIndex-=2;
					if( selectedSaveIndex < 0 )
						selectedSaveIndex += 6;
					moveUp = true;
					moveDelayCounter = moveDelayFrames;
					soundNodeList->ActivateSound( soundBuffers[S_UP] );
				}

				if( (currInput.LRight() || currInput.PRight()) && ( 
					(!moveRight && canMoveOther) || ( moveRight && canMoveSame ) ) )
				{
					selectedSaveIndex++;
					//currentMenuSelect++;
					if( selectedSaveIndex % 2 == 0 )
						selectedSaveIndex-= 2;
					moveRight = true;
					moveDelayCounter = moveDelayFrames;
				}
				else if( ( currInput.LLeft() || currInput.PLeft() ) && ( 
					(!moveLeft && canMoveOther) || ( moveLeft && canMoveSame ) ) )
				{
					selectedSaveIndex--;
					if( selectedSaveIndex % 2 == 1 )
						selectedSaveIndex += 2;
								else if( selectedSaveIndex < 0 )
				{
					selectedSaveIndex += 2;
				}
					moveLeft = true;

					moveDelayCounter = moveDelayFrames;
				}
				
				if( moveDelayCounter > 0 )
				{
					moveDelayCounter--;
				}
				

				if( !(currInput.LDown() || currInput.PDown()) )
				{
					moveDown = false;
				}
				if( ! ( currInput.LUp() || currInput.PUp() ) )
				{
					moveUp = false;
				}

				if( !(currInput.LRight() || currInput.PRight()) )
				{
					moveRight = false;
				}
				if( !(currInput.LLeft() || currInput.PLeft() ) )
				{
					moveLeft = false;
				}

				saveSelect.setTextureRect( ts_saveMenuSelect->GetSubRect( selectedSaveIndex ) );
				saveKinFace.setTextureRect( ts_saveMenuKinFace->GetSubRect( 0 ) );

				Vector2f topLeftPos;
				topLeftPos.x += ts_saveMenuSelect->tileWidth * ( selectedSaveIndex % 2 );
				topLeftPos.y += ts_saveMenuSelect->tileHeight * ( selectedSaveIndex / 2 );

				saveSelect.setPosition( topLeftPos );
				saveKinFace.setPosition( topLeftPos );

				break;
			}
		case TRANS_MAIN_TO_SAVE:
			break;
		case TRANS_SAVE_TO_MAIN:
			break;
		case TRANS_SAVE_TO_WORLDMAP:
			{
				if( saveKinFaceFrame == saveKinFaceTurnLength * 3 + 20 )
				{
					menuMode = WORLDMAP;
					break;
					//saveKinFaceFrame = 0;
				}

				if( saveKinFaceFrame < saveKinFaceTurnLength * 3 )
				{
					saveKinFace.setTextureRect( ts_saveMenuKinFace->GetSubRect( saveKinFaceFrame / 3 ) );
				}
				else
				{
					saveKinFace.setTextureRect( ts_saveMenuKinFace->GetSubRect( saveKinFaceTurnLength - 1 ) );
				}
				

				saveKinFaceFrame++;
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
				preScreenTexture->setView( v );
				preScreenTexture->draw( saveBG );
				preScreenTexture->draw( saveSelect );
				preScreenTexture->draw( saveKinFace );
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

		preScreenTexture->display();
		sf::Sprite pspr;
		pspr.setTexture( preScreenTexture->getTexture() );
		window->draw( pspr );

		window->display();
	}
}

CustomMapsHandler::CustomMapsHandler( MainMenu *p_menu )
		:menu( p_menu ), optionChosen( false ), showNamePopup( false )
{
}

void CustomMapsHandler::ButtonCallback( Button *b, const std::string & e )
{
	LevelSelector &ls = *menu->levelSelector;
	if( ls.text[ls.selectedIndex].getColor() == Color::Blue )
	{
		if( b->name == "Play" )
		{
			optionChosen = true;
			GameSession *gs = new GameSession( menu->controller, NULL, menu );
			gs->Run( ls.localPaths[ls.selectedIndex] );
			menu->window->setView( menu->uiView );
			delete gs;
		}
		else if( b->name == "Edit" )
		{
			optionChosen = true;
			menu->GameEditLoop( ls.localPaths[ls.selectedIndex] );//ls.paths[ls.selectedIndex].().string() );//ls.text[ls.selectedIndex].getString() );
			menu->window->setView( menu->uiView );
		}
		else if( b->name == "Delete" )
		{	
			boost::filesystem::remove(ls.localPaths[ls.selectedIndex]);
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