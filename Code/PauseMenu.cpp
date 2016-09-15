#include "PauseMenu.h"
#include <sstream>
#include "GameSession.h"

using namespace sf;
using namespace std;

PauseMenu::PauseMenu( GameSession *p_owner )
	:owner( p_owner ), currentTab( Tab::MAP )
{
	ts_background[0] = owner->GetTileset( "Menu/pause_1_map_1820x980.png", 1820, 980 );
	ts_background[1] = owner->GetTileset( "Menu/pause_2_kin_1820x980.png", 1820, 980 );
	ts_background[2] = owner->GetTileset( "Menu/pause_3_shards_1820x980.png", 1820, 980 );
	ts_background[3] = owner->GetTileset( "Menu/pause_4_options_1820x980.png", 1820, 980 );
	ts_background[4] = owner->GetTileset( "Menu/pause_5_pause_1820x980.png", 1820, 980 );

	bgSprite.setPosition( 0, 0 );
	//bgSprite.setPosition( (1920 - 1820) / 2, (1080 - 980) / 2);

	//SetTab( MAP );
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

	switch( t )
	{
	case MAP:
		mapCenter.x = owner->player->position.x;
		mapCenter.y = owner->player->position.y;
		mapZoomFactor = 16;	
		break;
	case KIN:
		break;
	case SHARDS:
		break;
	case OPTIONS:
		break;
	case PAUSE:
		break;
	}
}

void PauseMenu::Draw( sf::RenderTarget *target )
{
	target->draw( bgSprite );
}

void PauseMenu::Update( ControllerState &currInput,
		ControllerState &prevInput )
{
	if( (currInput.start && !prevInput.start) || (currInput.back && !prevInput.back ) )
	{
		owner->state = GameSession::State::RUN;
		owner->soundNodeList->Pause( false );
	}

	if( currInput.leftShoulder && !prevInput.leftShoulder )
	{
		TabLeft();
		return;
	}
	else if( currInput.rightShoulder && !prevInput.rightShoulder )
	{
		TabRight();
		return;
	}


	switch( currentTab )
	{
	case MAP:
		{
			float fac = .05;
			if( currInput.A )
			{
				mapZoomFactor -= fac * mapZoomFactor;
			}
			else if( currInput.B )
			{
				mapZoomFactor += fac * mapZoomFactor;
			}

			if( mapZoomFactor < 1.f )
			{
				mapZoomFactor = 1.f;
			}
			else if( mapZoomFactor > 128.f )
			{
				mapZoomFactor = 128.f;
			}

			float move = 20.0 * mapZoomFactor / 2.0;
			if( currInput.LLeft() )
			{
				mapCenter.x -= move;
			}
			else if( currInput.LRight() )
			{
				mapCenter.x += move;
			}

			if( currInput.LUp() )
			{
				mapCenter.y -= move;
			}
			else if( currInput.LDown() )
			{
				mapCenter.y += move;
			}

			if( mapCenter.x < owner->leftBounds )
			{
				mapCenter.x = owner->leftBounds;
			}
			else if( mapCenter.x > owner->leftBounds + owner->boundsWidth )
			{
				mapCenter.x = owner->leftBounds + owner->boundsWidth;
			}

			if( mapCenter.y < owner->topBounds )
			{
				mapCenter.y = owner->topBounds;
			}
			else if( mapCenter.y > owner->topBounds + owner->boundsHeight )
			{
				mapCenter.y = owner->topBounds + owner->boundsHeight;
			}
			break;
		}
	case KIN:
		{
			break;
		}
	case SHARDS:
		{
			break;
		}
	case OPTIONS:
		{
			break;
		}
	case PAUSE:
		{
			break;
		}
	}
}
//using namespace std;
//using namespace sf;