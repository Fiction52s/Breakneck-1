#include "VictoryScreen.h"
#include "GameSession.h"
#include "UIWindow.h"

using namespace std;
using namespace sf;

VictoryScreen2PlayerVS::VictoryScreen2PlayerVS( GameSession *p_owner )
	:owner( p_owner )
{
	player1Bar = new PlayerInfoBar( owner, 1920/2, 0 );
	player2Bar = new PlayerInfoBar( owner, 1920/2, 1 );
	//player2Bar->SetBottomLeftPos( Vector2f( 0, 128 ) );
}

void VictoryScreen::Reset()
{
	frame = 0;
}

void VictoryScreen2PlayerVS::Draw( sf::RenderTarget *target )
{
	player1Bar->Draw( target );
	player2Bar->Draw( target );
}

void VictoryScreen2PlayerVS::Update()
{
	player1Bar->Update( owner->GetCurrInput( 0 ).A && !owner->GetPrevInput( 0 ).A );
	player2Bar->Update( owner->GetCurrInput( 1 ).A && !owner->GetPrevInput( 1 ).A );
	++frame;
}

void VictoryScreen2PlayerVS::ResetSprites()
{
}
void VictoryScreen2PlayerVS::UpdateSprites()
{
}

void VictoryScreen2PlayerVS::Reset()
{
	VictoryScreen::Reset();
}

PlayerInfoBar::PlayerInfoBar( GameSession *p_owner, int width, int playerIndex )
	:owner( p_owner )
{
	//SetBottomLeftPos( Vector2f( 0, 0 ) );

	state = STATE_INITIAL_WAIT;

	frame = 0;

	framesBeforeShowFace = 60;
	framesExpandingShowFace = 60;
	framesExpandingFull = 60;
	framesToClose = 60;

	heightWait = 80;
	heightShowFace = 300;
	heightFull = 900;

	//windows = new UIWindow*[2];
	Tileset *ts_window = owner->GetTileset( "Menu/windows_64x24.png", 64, 24 );

	uiWindow = new UIWindow( NULL, ts_window, Vector2f( width, 1080 ) );
	uiWindow->SetTopLeft( playerIndex * width, 1080 ); //will be 1080 soon
	//windowVA[BOT_QUAD_INDEX]
}

void PlayerInfoBar::Draw( sf::RenderTarget *target )
{
	uiWindow->Draw( target );
	//target->draw( windowVA, 12, sf::Quads, ts_bar->texture );
}

void PlayerInfoBar::Update( bool pressedA )
{
	switch( state )
	{
	case STATE_INITIAL_WAIT:
		{
			if( frame == framesBeforeShowFace )
			{
				state = STATE_SHOW_FACE;
				frame = 0;
			}
			break;
		}
	case STATE_SHOW_FACE:
		{
			if( pressedA && frame > framesExpandingShowFace )
			{
				state = STATE_WAIT_EXPANDED;
				frame = 0;
			}
			break;
		}
	case STATE_WAIT_EXPANDED:
		{
			if( pressedA && frame >= framesExpandingFull )
			{
				state = STATE_CLOSED;
				frame = 0;
			}
			break;
		}
	case STATE_CLOSED:
		{
			
			break;
		}
	}

	switch( state )
	{
	case STATE_INITIAL_WAIT:
		{
			if( frame == 0 )
			{
				
				SetHeight( heightWait );
			}
			break;
		}
	case STATE_SHOW_FACE:
		{
			if( frame > framesExpandingShowFace )
			{
				frame = framesExpandingShowFace;
			}
			else
			{
				CubicBezier bez( 0, 0, 1, 1 );
				double f = bez.GetValue( (double)frame / framesExpandingShowFace );
				double h = heightWait * ( 1.0 - f ) + heightShowFace * f;
				SetHeight( floor( h + .5 ) );
			}
			break;
		}
	case STATE_WAIT_EXPANDED:
		{
			if( frame > framesExpandingFull )
			{
				frame = framesExpandingFull;
			}
			else
			{
				CubicBezier bez( 0, 0, 1, 1 );
				double f = bez.GetValue( (double)frame / framesExpandingFull );
				double h = heightShowFace * ( 1.0 - f ) + heightFull * f;
				SetHeight( floor( h + .5 ) );
			}
			break;
		}
	case STATE_CLOSED:
		{
			if( frame >= framesToClose )
			{
				frame = framesToClose;
			}
			break;
		}
	}
	++frame;
}

void PlayerInfoBar::SetHeight( int height )
{
	uiWindow->SetTopLeft( uiWindow->GetTopLeftRel().x, 1080 - height );
	//uiWindow->Resize( uiWindow->GetWidth(), height );
}

