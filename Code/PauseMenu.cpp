#include "PauseMenu.h"
#include <sstream>
#include "GameSession.h"

using namespace sf;
using namespace std;

PauseMenu::PauseMenu( GameSession *p_owner )
	:owner( p_owner ), currentTab( Tab::MAP ), show( false )
{
	ts_background[0] = owner->GetTileset( "Menu/pause_1_map_1820x980.png", 1820, 980 );
	ts_background[1] = owner->GetTileset( "Menu/pause_2_kin_1820x980.png", 1820, 980 );
	ts_background[2] = owner->GetTileset( "Menu/pause_3_shards_1820x980.png", 1820, 980 );
	ts_background[3] = owner->GetTileset( "Menu/pause_4_options_1820x980.png", 1820, 980 );
	ts_background[4] = owner->GetTileset( "Menu/pause_5_pause_1820x980.png", 1820, 980 );

	bgSprite.setPosition( 0, 0 );
	//bgSprite.setPosition( (1920 - 1820) / 2, (1080 - 980) / 2);

	SetTab( PAUSE );
}

void PauseMenu::TabLeft()
{
	int index = (int)currentTab;
	index--;
	if( index < 0 )
		index = 4;

	currentTab = (Tab)index;
	SetTab( currentTab );
}

void PauseMenu::TabRight()
{
	int index = (int)currentTab;
	index++;
	if( index > 4 )
		index = 0;

	currentTab = (Tab)index;
	SetTab( currentTab );
}

void PauseMenu::SetTab( Tab t )
{
	currentTab = t;
	bgSprite.setTexture( *ts_background[currentTab]->texture );
}

void PauseMenu::Draw( sf::RenderTarget *target )
{
	if( show )
	{
		target->draw( bgSprite );
	}
}
//using namespace std;
//using namespace sf;