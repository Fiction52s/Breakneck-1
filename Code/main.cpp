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


#define TIMESTEP 1.0 / 60.0

using namespace std;
using namespace sf;

RenderWindow *window;
View v;
using namespace boost::filesystem;

#define V2d sf::Vector2<double>
GameController controller(0);

RenderTexture *preScreenTexture;
RenderTexture *minimapTexture;

sf::Texture worldMapTex;
sf::Sprite worldMapSpr;

//sf::View uiView( sf::Vector2f( 480, 270 ), sf::Vector2f( 960, 540 ) );
sf::View uiView( sf::Vector2f( 960, 540 ), sf::Vector2f( 1920, 1080 ) );

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

void GameEditLoop( std::string filename)
{
	int result = 0;

	Vector2f lastViewSize( 0, 0 );
	Vector2f lastViewCenter( 0, 0 );
	while( result == 0 )
	{
		EditSession es(window, preScreenTexture );
		result = es.Run( filename, lastViewCenter, lastViewSize );
		if( result > 0 )
			break;

		window->setView( v );
		GameSession gs( controller, window, preScreenTexture, minimapTexture );
		
		result = gs.Run( filename );
		lastViewCenter = gs.lastViewCenter;
		lastViewSize = gs.lastViewSize;
	}
}

void GameEditLoop2( std::string filename)
{
	int result = 0;

	Vector2f lastViewSize( 0, 0 );
	Vector2f lastViewCenter( 0, 0 );
	while( result == 0 )
	{
		window->setView( v );
		GameSession gs( controller, window, preScreenTexture, minimapTexture );
		result = gs.Run( filename );
		lastViewCenter = gs.lastViewCenter;
		lastViewSize = gs.lastViewSize;
		if( result > 0 )
			break;

		EditSession es(window, preScreenTexture );
		result = es.Run( filename, lastViewCenter, lastViewSize );
	}
}

void WorldSelectMenu()
{
	window->setView( uiView );
	bool quit = false;
	int worldSel = 0;
	while( !quit )
	{
		window->clear();
		if( controller.UpdateState() )
		{
			ControllerState cs = controller.GetState();
			if( cs.LDown() )
			{
				worldSel++;
				//cout << "worldSel: " << worldSel << endl;
			}
		}
		window->draw( worldMapSpr );
		window->display();
	}
	window->setView( v );
}

void LoadMenus()
{
	worldMapTex.loadFromFile( "worldmap.png" );
	worldMapSpr.setTexture( worldMapTex );
}

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
				GameSession gs( controller, window, preScreenTexture, minimapTexture );
				gs.Run( ls.localPaths[ls.selectedIndex] );
				window->setView( uiView );
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
	GameSession gs( controller, window, preScreenTexture, minimapTexture );
	gs.Run( "Maps/aba.brknk" );

}

void LoadCampaignOption()
{
	cout << "loading!" << endl;
}

void OptionsOption()
{
	cout << "options!" << endl;
}

void ExitOption()
{
	cout << "exit!" << endl;
}

int main()
{
	preScreenTexture = new RenderTexture;
	preScreenTexture->create( 1920, 1080 );
	preScreenTexture->clear();

	minimapTexture = new RenderTexture;
	minimapTexture->create( 300, 300 );
	minimapTexture->clear();

	sf::Font arial;
	arial.loadFromFile( "arial.ttf" );

	std::cout << "starting program" << endl;
	bool fullWindow = true;

	int windowWidth = 1920;
	int windowHeight = 1080;

	LevelSelector ls( arial );
	//ls.windowStretch = Vector2f( windowWidth / 1920, windowHeight / 1080 );
	LoadMenus();

	int currentMenuSelect = 0;

	
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
		
		
		window = new sf::RenderWindow( i.front(), "Breakneck", sf::Style::Fullscreen);
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
	t.loadFromFile( "title.png" );
	
	Sprite titleSprite;
	titleSprite.setTexture( t );
	titleSprite.setOrigin( titleSprite.getLocalBounds().width / 2, titleSprite.getLocalBounds().height / 2 );
	titleSprite.setPosition( 0, 0 );
	titleSprite.setScale( 2, 2 );
	
	
	v.setCenter( 0, 0 );
	v.setSize( 1920 / 2, 1080 / 2 );
	window->setView( v );

	sf::Text menu;
	menu.setString( "\t\tPress any button to start \nFor help and information check CONTROLS AND INFO.txt\n\t\tBreakneck Alpha 12-8-2015");
	menu.setCharacterSize( 20 );
	menu.setColor( Color::Red );
	
	menu.setFont( arial );
	menu.setOrigin( menu.getLocalBounds().width / 2, menu.getLocalBounds().height / 2 );
	menu.setPosition( 0, 0 );

	sf::Event ev;
	bool quit = false;



	//window->setVerticalSyncEnabled( true );

	window->setView( v );
	window->draw( titleSprite );
	window->draw( menu );		

	window->display();

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

	bool moveDown = false;
	bool moveUp = false;
	while( !quit )
	{
		window->clear();
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

		while( window->pollEvent( ev ) )
		{
		//	cout << "some input" << endl;
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
						quit = true;
					}
					else if( ev.key.code == Keyboard::M )
					{
						//WorldSelectMenu();
					}
					else if( ev.key.code == Keyboard::Return || ev.key.code == Keyboard::Space )
					{
						GameSession *gs = NULL;
						//select from option menu
						switch( currentMenuSelect )
						{
						case 0:
							gs = new GameSession( controller, window, preScreenTexture, minimapTexture );
							gs->Run( "Maps/1-1.brknk" );
							//NewCampaignOption();
							break;
						case 1:
							gs = new GameSession( controller, window, preScreenTexture, minimapTexture );
							gs->Run( "Maps/1-2.brknk" );
							//LoadCampaignOption();
							break;
						case 2:
							gs = new GameSession( controller, window, preScreenTexture, minimapTexture );
							gs->Run( "Maps/1-3.brknk" );
							//CustomMapsOption( ls );
							break;
						case 3:
							gs = new GameSession( controller, window, preScreenTexture, minimapTexture );
							gs->Run( "Maps/1-4.brknk" );
							//OptionsOption();
							break;
						case 4:
							//CustomMapsOption( ls );
							gs = new GameSession( controller, window, preScreenTexture, minimapTexture );
							gs->Run( "Maps/1-5.brknk" );
							//ExitOption();
							break;
						}

						delete gs;
					}
					else if( ev.key.code == Keyboard::Up )
					{
						currentMenuSelect--;
						if( currentMenuSelect < 0 )
							currentMenuSelect = 4;
					}
					else if( ev.key.code == Keyboard::Down )
					{
						currentMenuSelect++;
						if( currentMenuSelect > 4 )
							currentMenuSelect = 0;
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

		if( controller.UpdateState() )
		{
			ControllerState cs = controller.GetState();
			if( cs.A || cs.back || cs.Y || cs.X || cs.rightShoulder || cs.leftShoulder )
			{
				GameSession *gs = NULL;
				//select from option menu
				switch( currentMenuSelect )
				{
				case 0:
					gs = new GameSession( controller, window, preScreenTexture, minimapTexture );
					gs->Run( "Maps/1-1.brknk" );
					//NewCampaignOption();
					break;
				case 1:
					gs = new GameSession( controller, window, preScreenTexture, minimapTexture );
					gs->Run( "Maps/1-2.brknk" );
					//LoadCampaignOption();
					break;
				case 2:
					gs = new GameSession( controller, window, preScreenTexture, minimapTexture );
					gs->Run( "Maps/1-3.brknk" );
					//CustomMapsOption( ls );
					break;
				case 3:
					gs = new GameSession( controller, window, preScreenTexture, minimapTexture );
					gs->Run( "Maps/1-4.brknk" );
					//OptionsOption();
					break;
				case 4:
					//replace this before releasing
					//CustomMapsOption( ls );
					gs = new GameSession( controller, window, preScreenTexture, minimapTexture );
					gs->Run( "Maps/1-5.brknk" );
					//ExitOption();
					break;
				}

				delete gs;
				/*switch( currentMenuSelect )
				{
				case 0:
					NewCampaignOption();
					break;
				case 1:
					LoadCampaignOption();
					break;
				case 2:
					CustomMapsOption( ls );
					break;
				case 3:
					OptionsOption();
					break;
				case 4:
					ExitOption();
					break;
				}*/
				//GameEditLoop2( "test3.brknk" );
				//window->setView( v );
			}
			if( (cs.LDown() || cs.PDown()) && !moveDown )
			{
				currentMenuSelect++;
				if( currentMenuSelect > 4 )
					currentMenuSelect = 0;
				moveDown = true;
			}
			else if( ( cs.LUp() || cs.PUp() ) && !moveUp )
			{
				currentMenuSelect--;
				if( currentMenuSelect < 0 )
					currentMenuSelect = 4;
				moveUp = true;
			}
			else
			{
			}

			if( !(cs.LDown() || cs.PDown()) )
			{
				moveDown = false;
			}
			if( ! ( cs.LUp() || cs.PUp() ) )
			{
				moveUp = false;
			}
		}

		//window->pushGLStates();
		window->setView( v );
		window->draw( titleSprite );
		window->draw( menu );	
		//window->popGLStates();
		
		//window->setView( window->getDefaultView() );
		
		
		//window->setView( //window->getDefaultView() );

		
		
		//prim.DrawTetrahedron( window );

		
		//prim.Draw2( window );

		//window->pushGLStates();
		
		window->setView( uiView );


		for( int i = 0; i < 5; ++i )
		{
			if( i == currentMenuSelect )
			{
				mainMenu[i].setColor( Color::White );
			}
			else
			{
				mainMenu[i].setColor( Color::Red );
			}
			//cout << "drawing i: " << i <<  endl;
			window->draw( mainMenu[i] );
		}

		//window->popGLStates();

		window->display();
	}


	sf::Vector2i pos( 0, 0 );	
	
	View view( Vector2f( 300, 300 ), sf::Vector2f( 960 * 2, 540 * 2 ) );
	window->setView( view );
	
	bool edit = false;

	

	window->close();
	delete window;
}

