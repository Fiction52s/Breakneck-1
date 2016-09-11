#include <iostream>
//#include "PlayerChar.h"
#include <sstream>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <assert.h>
#include <fstream>
#include <list> 
#include <stdlib.h>
#include "EditSession.h"
#include "VectorMath.h"
#include "Input.h"
#include "poly2tri/poly2tri.h"
#include "Physics.h"
#include "Actor.h"
#include "Tileset.h"
#include "GameSession.h"
#include "LevelSelector.h"
#include <boost/filesystem.hpp>
#include "Primitive3D.h"
#include <SFML/OpenGL.hpp>
#include "WorldMap.h"
#include "SaveFile.h"


#define TIMESTEP 1.0 / 60.0

using namespace std;
using namespace sf;

using namespace boost::filesystem;

#define V2d sf::Vector2<double>


void perspectiveGL( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
    const GLdouble pi = 3.1415926535897932384626433832795;
    GLdouble fW, fH;

    //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
    fH = tan( fovY / 360 * pi ) * zNear;
    fW = fH * aspect;

    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}


void collideShapes( Actor &a, const CollisionBox &b, Actor &a1, const CollisionBox &b1 )
{
	if( b.isCircle && b1.isCircle )
	{
		//circle circle
	}
	else if( b.isCircle )
	{
		//circle rect
	}
	else if( b1.isCircle )
	{
		//circle rect
	}
	else
	{
		//rect rect
	}
}



//void WorldSelectMenu()
//{
//	window->setView( uiView );
//	bool quit = false;
//	int worldSel = 0;
//	while( !quit )
//	{
//		window->clear();
//		if( controller.UpdateState() )
//		{
//			ControllerState cs = controller.GetState();
//			if( cs.LDown() )
//			{
//				worldSel++;
//				//cout << "worldSel: " << worldSel << endl;
//			}
//		}
//		window->draw( worldMapSpr );
//		window->display();
//	}
//	window->setView( v );
//}

//void LoadMenus()
//{
//	worldMapTex.loadFromFile( "worldmap.png" );
//	worldMapSpr.setTexture( worldMapTex );
//}



struct CustomMapsHandler : GUIHandler
{
	LevelSelector &ls;
	bool optionChosen;
	bool showNamePopup;

	CustomMapsHandler( LevelSelector &levelSelector )
		:ls( levelSelector ), optionChosen( false ), showNamePopup( false )
	{
	}

	
	void ButtonCallback( Button *b, const std::string & e )
	{
		if( ls.text[ls.selectedIndex].getColor() == Color::Blue )
		{
			if( b->name == "Play" )
			{
				optionChosen = true;
				GameSession *gs = new GameSession( controller, window, NULL, preScreenTexture, postProcessTexture,postProcessTexture1, postProcessTexture2, minimapTexture, mapTexture );
				gs->Run( ls.localPaths[ls.selectedIndex] );
				window->setView( uiView );
				delete gs;
			}
			else if( b->name == "Edit" )
			{
				optionChosen = true;
				GameEditLoop( ls.localPaths[ls.selectedIndex] );//ls.paths[ls.selectedIndex].().string() );//ls.text[ls.selectedIndex].getString() );
				window->setView( uiView );
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

	void TextBoxCallback( TextBox *tb, const std::string & e )
	{
	}

	void GridSelectorCallback( GridSelector *gs, const std::string & e )
	{
	}

	void CheckBoxCallback( CheckBox *cb, const std::string & e )
	{
	}
};


void LoadSaveFiles()
{

}

void CustomMapsOption( LevelSelector &ls )
{
	sf::Event ev;
	window->setView( uiView );
	bool quit = false;

	CustomMapsHandler customMapHandler( ls );

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

void NewCampaignOption()
{
	GameSession gs( controller, window, NULL, preScreenTexture, postProcessTexture,postProcessTexture1,postProcessTexture2, minimapTexture, mapTexture );
	gs.Run( "Maps/aba.brknk" );

}

//void LoadCampaignOption()
//{
//	cout << "loading!" << endl;
//}

//void SaveLoadScreen( bool newGame )
//{
//	
//}

//void OptionsOption()
//{
//	cout << "options!" << endl;
//}

//void ExitOption()
//{
//	cout << "exit!" << endl;
//}


int main()
{
	bool selectCreateNew = false;

	std::cout << "starting program" << endl;
	bool fullWindow = true;

	int windowWidth = 1920;
	int windowHeight = 1080;

	//LevelSelector ls( arial );
	//ls.windowStretch = Vector2f( windowWidth / 1920, windowHeight / 1080 );
	//LoadMenus();

	

	

	
	if( sf::Keyboard::isKeyPressed( Keyboard::W ) )
	{
		fullWindow = false;
		windowWidth /= 2;
		windowHeight /= 2;
	}

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
	//window->setVerticalSyncEnabled( false );
	//window->setFramerateLimit( 60 );



	/*Clock gameClock;
	double currentTime = 0;
	while( true )
	{
		double newTime = gameClock.getElapsedTime().asSeconds();
		double frameTime = newTime - currentTime;

		window->clear(Color::Blue);

		sf::Event ev;
		while( window->pollEvent( ev ) )
		{
		}

		preScreenTexture->clear(Color::Red);
		preScreenTexture->display();
		Sprite spr( preScreenTexture->getTexture() );

		window->draw( spr );
		window->draw( spr );
		window->draw( spr );
		window->draw( spr );
		window->draw( spr );

		window->display();
	}*/
	
	//window->setFramerateLimit( 60 );
	//test.clear();



	//sf::Music titleMusic;
	//if( !titleMusic.openFromFile( "titletheme.ogg" ))
	//	assert( false && "no music found" );

	//titleMusic.setLoop( true );
	//titleMusic.setVolume( 0 );
	//titleMusic.play();

	std::cout << "opened window" << endl;
	sf::Texture t;
	t.loadFromFile( "title_1920x1080.png" );

	

	

	//should just put in tilesets here like cmon
	
	window->setView( v );

	sf::Text menu;
	menu.setString( " Press any button to start \n For help and info check \n - beta_info.txt\n Breakneck Beta\n Updated 9/2/2016");
	menu.setCharacterSize( 20 );
	menu.setColor( Color::Red );
	
	menu.setFont( arial );
	//menu.setOrigin( menu.getLocalBounds().width / 2, menu.getLocalBounds().height / 2 );
	menu.setPosition( 0, 200 );

	sf::Event ev;
	bool quit = false;



	//window->setVerticalSyncEnabled( true );

	window->setView( v );
	//window->draw( titleSprite );
	//window->draw( menu );		

	//window->display();

	Text mainMenu[5];
	int fontSize = 32;

	int h = 100;
	int yoffset = 200;
	int xoffset = 200;
	int index = 0;
	Text &newText = mainMenu[index];
	//newText.setString( "New Campaign" );
	newText.setString( "Level 1" );
	newText.setFont( arial );
	newText.setColor( Color::Red );
	newText.setCharacterSize( fontSize );
	newText.setPosition( xoffset, yoffset + h * index );
	
	++index;

	Text &loadText = mainMenu[index];
	//loadText.setString( "Load Campaign" );
	loadText.setString( "Level 2" );
	loadText.setFont( arial );
	loadText.setColor( Color::Red );
	loadText.setCharacterSize( fontSize );
	loadText.setPosition( xoffset, yoffset + h * index );

	++index; 

	Text &customText = mainMenu[index];
	customText.setString( "Level 3" );
	//customText.setString( "Custom Maps" );
	customText.setFont( arial );
	customText.setColor( Color::Red );
	customText.setCharacterSize( fontSize );
	customText.setPosition( xoffset, yoffset + h * index );

	++index;

	Text &optionsText = mainMenu[index];
	//optionsText.setString( "Options" );
	optionsText.setString( "Level 4" );
	optionsText.setFont( arial );
	optionsText.setColor( Color::Red );
	optionsText.setCharacterSize( fontSize );
	optionsText.setPosition( xoffset, yoffset + h * index );

	++index;

	Text &exitText = mainMenu[index];
	//exitText.setString( "Exit" );
	exitText.setString( "Level 5 ( Boss )" );

	exitText.setFont( arial );
	exitText.setColor( Color::Red );
	exitText.setCharacterSize( fontSize );
	exitText.setPosition( xoffset, yoffset + h * index );

	ControllerState currInput;
	
	
	//ls.UpdateMapList();
	//ls.Print();
	
	/*Primitive prim;

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	glViewport(0,0,1920,1080);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	perspectiveGL(45.0f,(GLfloat)1920/(GLfloat)1080,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();	*/


	//cout << "beginning input loop" << endl;

	sf::Clock gameClock;
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

	bool worldMapUpdate = false;

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
		
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

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
						CustomMapsOption( ls );
						//WorldSelectMenu();
					}
					else if( ev.key.code == Keyboard::Return || ev.key.code == Keyboard::Space )
					{
						//menuMode = WORLDMAP;
						//worldMap->state = WorldMap::PLANET_AND_SPACE;//WorldMap::PLANET_AND_SPACE;
						//worldMap->frame = 0;
						//worldMap->UpdateMapList();
						
						
						
						//GameSession *gs = NULL;
						//select from option menu
						//switch( currentMenuSelect )
						//{
						//case 0:
						//	//gs = new GameSession( controller, window, preScreenTexture, postProcessTexture, postProcessTexture1, postProcessTexture2, minimapTexture, mapTexture );
						//	//gs->Run( "Maps/1-1.brknk" );
						
												
						//	break;
						//case 1:
						//	gs = new GameSession( controller, window, preScreenTexture, postProcessTexture, postProcessTexture1, postProcessTexture2, minimapTexture, mapTexture );
						//	gs->Run( "Maps/1-2.brknk" );
						//	//LoadCampaignOption();
						//	break;
						//case 2:
						//	gs = new GameSession( controller, window, preScreenTexture, postProcessTexture, postProcessTexture1, postProcessTexture2, minimapTexture, mapTexture );
						//	gs->Run( "Maps/1-3.brknk" );
						//	//CustomMapsOption( ls );
						//	break;
						//case 3:
						//	gs = new GameSession( controller, window, preScreenTexture, postProcessTexture, postProcessTexture1, postProcessTexture2, minimapTexture, mapTexture );
						//	gs->Run( "Maps/1-4.brknk" );
						//	//OptionsOption();
						//	break;
						//case 4:
						//	CustomMapsOption( ls );
						//	//gs = new GameSession( controller, window, preScreenTexture, minimapTexture );
						//	//gs->Run( "Maps/1-5.brknk" );
						//	//ExitOption();
						//	break;
						//}

						//delete gs;
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
				kinTitleSprite.setTexture( kinTitleTextures[ trueKinFrame ] );
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
					GameSession *gs = new GameSession( controller, window, NULL, preScreenTexture, postProcessTexture, postProcessTexture1, postProcessTexture2, minimapTexture, mapTexture );
					gs->Run( ss.str() );
					delete gs;

					v.setSize( 1920, 1080 );
					v.setCenter( 1920/2, 1080/ 2);
					window->setView( v );
					worldMap->state = WorldMap::PLANET_AND_SPACE;
					worldMap->frame = 0;
					worldMap->UpdateMapList();
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
				}
				else if( ( currInput.LUp() || currInput.PUp() ) && ( 
					(!moveUp && canMoveOther) || ( moveUp && canMoveSame ) ) )
				{
					selectedSaveIndex-=2;
					if( selectedSaveIndex < 0 )
						selectedSaveIndex += 6;
					moveUp = true;
					moveDelayCounter = moveDelayFrames;
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

				saveSelect.setTextureRect( ts_saveSelected.GetSubRect( selectedSaveIndex ) );
				saveKinFace.setTextureRect( ts_saveKinFace.GetSubRect( 0 ) );

				Vector2f topLeftPos;
				topLeftPos.x += ts_saveSelected.tileWidth * ( selectedSaveIndex % 2 );
				topLeftPos.y += ts_saveSelected.tileHeight *( selectedSaveIndex / 2 );

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
					saveKinFace.setTextureRect( ts_saveKinFace.GetSubRect( saveKinFaceFrame / 3 ) );
				}
				else
				{
					saveKinFace.setTextureRect( ts_saveKinFace.GetSubRect( saveKinFaceTurnLength - 1 ) );
				}
				

				saveKinFaceFrame++;
				break;
			}
		}
			
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
				
				//window->draw( titleSprite );
				//preScreenTexture->draw( menu );	

				preScreenTexture->setView( uiView );

				preScreenTexture->draw( menu );

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


	sf::Vector2i pos( 0, 0 );	
	
	View view( Vector2f( 300, 300 ), sf::Vector2f( 960 * 2, 540 * 2 ) );
	window->setView( view );
	
	bool edit = false;

	

	window->close();

	delete window;
	/*delete saveMenuSelectTex;*/
	
	delete preScreenTexture;
	delete postProcessTexture;
	delete postProcessTexture1;
	delete postProcessTexture2;
	delete minimapTexture;
	delete mapTexture;
}

