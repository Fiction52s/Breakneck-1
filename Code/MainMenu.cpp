#include "MainMenu.h"
#include <boost/filesystem.hpp>
#include "EditSession.h"
#include "GameSession.h"
#include "SaveFile.h"

using namespace std;
using namespace sf;
using namespace boost::filesystem;

sf::RenderTexture *MainMenu::preScreenTexture = NULL;
sf::RenderTexture *MainMenu::postProcessTexture = NULL;
sf::RenderTexture *MainMenu::postProcessTexture1 = NULL;
sf::RenderTexture *MainMenu::postProcessTexture2 = NULL;
sf::RenderTexture *MainMenu::minimapTexture = NULL;
sf::RenderTexture *MainMenu::mapTexture = NULL;


MainMenu::MainMenu()
	:controller( 0 )
{
	uiView = View( sf::Vector2f( 960, 540 ), sf::Vector2f( 1920, 1080 ) );

	files[0] = new SaveFile( "blue" );
	files[1] = new SaveFile( "green" );
	files[2] = new SaveFile( "yellow" );
	files[3] = new SaveFile( "orange" );
	files[4] = new SaveFile( "red" );
	files[5] = new SaveFile( "magenta" );

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

	ts_saveMenuBG = tilesetManager.GetTileset( "Menu/save_bg_1920x1080.png", 1920, 1080 );
	ts_saveMenuKinFace = tilesetManager.GetTileset( "Menu/save_menu_kin_256x256.png", 256, 256 );
	ts_saveMenuSelect = tilesetManager.GetTileset( "Menu/save_select_710x270.png", 710, 270 );


	saveBG.setTexture( *ts_saveMenuBG->texture );
	saveKinFace.setTexture( *ts_saveMenuKinFace->texture );
	saveSelect.setTexture( *ts_saveMenuSelect->texture );
	

	saveKinFaceFrame = 0;
	saveKinFaceTurnLength = 15;

	arial.loadFromFile( "arial.ttf" );
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
		GameSession *gs = new GameSession( controller, window, NULL, preScreenTexture, 
			postProcessTexture,postProcessTexture1, postProcessTexture2, minimapTexture, mapTexture );
		
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
		GameSession *gs = new GameSession( controller, window, NULL, preScreenTexture, postProcessTexture,postProcessTexture1,postProcessTexture2, minimapTexture,
			 mapTexture);
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