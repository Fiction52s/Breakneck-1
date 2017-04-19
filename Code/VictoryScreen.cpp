#include "VictoryScreen.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

VictoryScreen2PlayerVS::VictoryScreen2PlayerVS( GameSession *p_owner )
	:owner( p_owner )
{
	player1Bar = new PlayerInfoBar( owner );
	player2Bar = new PlayerInfoBar( owner );

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
	player1Bar->Update( owner->currInput.A && !owner->prevInput.A );
	player2Bar->Update( owner->currInput2.A && !owner->prevInput2.A );
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

PlayerInfoBar::PlayerInfoBar( GameSession *p_owner )
	:owner( p_owner )
{
	ts_bar = owner->GetTileset( "window_92x92.png", 92, 92 );//owner->GetTileset( "playerinfobar_128x20.png", 128, 20 );

	AssignCorners();
	AssignEdges();
	
	origMiddleHeight = 20;
	currMiddleHeight = origMiddleHeight;

	windowSize = Vector2i( 200, 200 );
	//botQuadHeight = 92;
	//topQuadHeight = 92;
	//totalWidth = 128;

	minSize = 92;

	//SetBottomLeftPos( Vector2f( 0, 0 ) );

	currState = STATE_SHOW_FACE;

	frame = 0;

	framesBeforeShowFace = 60;
	framesExpandingShowFace = 60;
	framesExpandingFull = 60;
	framesToClose = 60;

	heightWait = 80;
	heightShowFace = 300;
	heightFull = 900;
	//windowVA[BOT_QUAD_INDEX]
}

void PlayerInfoBar::AssignEdges()
{
	int totalHeight = 20;
	int middleHeight = totalHeight - botQuadHeight - topQuadHeight;
	
	//walls

	Vertex *wallVA = windowVA;
	Vertex *flatVA = (windowVA+ 2 * 4 );

	//ceiling and floor

	IntRect flatRect = ts_bar->GetSubRect( 1 );

	for( int i = 0; i < 2; ++i )
	{
		wallVA[i*4+0].texCoords = Vector2f( flatRect.left, flatRect.top + flatRect.height );
		wallVA[i*4+1].texCoords = Vector2f( flatRect.left, flatRect.top );
		wallVA[i*4+2].texCoords = Vector2f( flatRect.left + flatRect.width, flatRect.top );
		wallVA[i*4+3].texCoords = Vector2f( flatRect.left + flatRect.width, flatRect.top + flatRect.height );
	}


	for( int i = 0; i < 2; ++i )
	{
		flatVA[i*4+0].texCoords = Vector2f( flatRect.left, flatRect.top );
		flatVA[i*4+1].texCoords = Vector2f( flatRect.left + flatRect.width, flatRect.top );
		flatVA[i*4+2].texCoords = Vector2f( flatRect.left + flatRect.width, flatRect.top + flatRect.height );
		flatVA[i*4+3].texCoords = Vector2f( flatRect.left, flatRect.top + flatRect.height );
	}

	

	/*windowVA[BOT_QUAD_INDEX+0].texCoords = Vector2f( 0, totalHeight - botQuadHeight );
	windowVA[BOT_QUAD_INDEX+1].texCoords = Vector2f( totalWidth, totalHeight - botQuadHeight );
	windowVA[BOT_QUAD_INDEX+2].texCoords = Vector2f( totalWidth, totalHeight );
	windowVA[BOT_QUAD_INDEX+3].texCoords = Vector2f( 0, totalHeight );

	windowVA[STRETCH_QUAD_INDEX+0].texCoords = Vector2f( 0, topQuadHeight );
	windowVA[STRETCH_QUAD_INDEX+1].texCoords = Vector2f( totalWidth, topQuadHeight );
	windowVA[STRETCH_QUAD_INDEX+2].texCoords = Vector2f( totalWidth, topQuadHeight + middleHeight );
	windowVA[STRETCH_QUAD_INDEX+3].texCoords = Vector2f( 0, topQuadHeight + middleHeight );

	windowVA[TOP_QUAD_INDEX+0].texCoords = Vector2f( 0, 0 );
	windowVA[TOP_QUAD_INDEX+1].texCoords = Vector2f( totalWidth, 0 );
	windowVA[TOP_QUAD_INDEX+2].texCoords = Vector2f( totalWidth, topQuadHeight );
	windowVA[TOP_QUAD_INDEX+3].texCoords = Vector2f( 0, topQuadHeight );*/
}

void PlayerInfoBar::AssignCorners()
{
	sf::Vertex *cornerVA = (windowVA + 4 * 4); 

	IntRect cornerRect = ts_bar->GetSubRect( 0 );
	Vector2f points[4];
	points[0] = Vector2f( cornerRect.left, cornerRect.top );
	points[1] = Vector2f( cornerRect.left + cornerRect.width, cornerRect.top );
	points[2] = Vector2f( cornerRect.left + cornerRect.width, cornerRect.top + cornerRect.height );
	points[3] = Vector2f( cornerRect.left, cornerRect.top + cornerRect.height );

	int i = 0;
	cornerVA[i*4+0].texCoords = points[0];
	cornerVA[i*4+1].texCoords = points[1];
	cornerVA[i*4+2].texCoords = points[2];
	cornerVA[i*4+3].texCoords = points[3];

	++i;
	cornerVA[i*4+0].texCoords = points[3];
	cornerVA[i*4+1].texCoords = points[0];
	cornerVA[i*4+2].texCoords = points[1];
	cornerVA[i*4+3].texCoords = points[2];

	++i;
	cornerVA[i*4+0].texCoords = points[2];
	cornerVA[i*4+1].texCoords = points[3];
	cornerVA[i*4+2].texCoords = points[0];
	cornerVA[i*4+3].texCoords = points[1];

	++i;
	cornerVA[i*4+0].texCoords = points[1];
	cornerVA[i*4+1].texCoords = points[2];
	cornerVA[i*4+2].texCoords = points[3];
	cornerVA[i*4+3].texCoords = points[0];
}

void PlayerInfoBar::SetCornerPos( sf::Vector2i &topLeft, int index )
{
	Vertex *cornerVA = (windowVA + 4 * 4 );	
	cornerVA[index*4+0].position = Vector2f( topLeft.x, topLeft.y );
	cornerVA[index*4+1].position = Vector2f( topLeft.x + minSize, topLeft.y );
	cornerVA[index*4+2].position = Vector2f( topLeft.x + minSize, topLeft.y + minSize );
	cornerVA[index*4+3].position = Vector2f( topLeft.x, topLeft.y + minSize );
}

void PlayerInfoBar::SetWallPos( int x, int index )
{
	Vertex *wallVA = windowVA;

	//wallVA
}

void PlayerInfoBar::SetBottomLeftPos( sf::Vector2i &pos )
{
	bottomLeft = pos;
	int trueBottom = pos.y;
	int botTop = pos.y - botQuadHeight;
	int midTop = botTop - currMiddleHeight;
	int topTop = midTop - topQuadHeight;

	Vertex *wallVA = windowVA;
	Vertex *flatVA = (windowVA + 2 * 4 );
	Vertex *cornerVA = (windowVA + 4 * 4 );

	int leftWall = pos.x;
	int rightWall = pos.x + windowSize.x;

	int ceiling = pos.y - windowSize.y;
	int floorH = pos.y;

	/*SetCornerPos( Vector2f( leftWall, trueBottom - minSize ), 0 );
	SetCornerPos( Vector2f( leftWall, topTop - minSize ), 1 );
	SetCornerPos( Vector2f( rightWall, trueBottom - minSize ), 2 );
	SetCornerPos( Vector2f( rightWall, topTop - minSize ), 3 );*/

	//about to set up the function for setwallpos and get the window all set up tomorrow

	/*windowVA[BOT_QUAD_INDEX+0].position = Vector2f( pos.x, botTop );
	windowVA[BOT_QUAD_INDEX+1].position = Vector2f( pos.x + totalWidth, botTop );
	windowVA[BOT_QUAD_INDEX+2].position = Vector2f( pos.x + totalWidth, trueBottom );
	windowVA[BOT_QUAD_INDEX+3].position = Vector2f( pos.x, trueBottom );

	windowVA[STRETCH_QUAD_INDEX+0].position = Vector2f( pos.x, midTop );
	windowVA[STRETCH_QUAD_INDEX+1].position = Vector2f( pos.x + totalWidth, midTop );
	windowVA[STRETCH_QUAD_INDEX+2].position = Vector2f( pos.x + totalWidth, botTop );
	windowVA[STRETCH_QUAD_INDEX+3].position = Vector2f( pos.x, botTop );

	windowVA[TOP_QUAD_INDEX+0].position = Vector2f( pos.x, topTop );
	windowVA[TOP_QUAD_INDEX+1].position = Vector2f( pos.x + totalWidth, topTop );
	windowVA[TOP_QUAD_INDEX+2].position = Vector2f( pos.x + totalWidth, midTop );
	windowVA[TOP_QUAD_INDEX+3].position = Vector2f( pos.x, midTop );*/
}

const sf::Vector2f &PlayerInfoBar::GetBottomLeftPos()
{
	return windowVA[BOT_QUAD_INDEX+2].position;
}

void PlayerInfoBar::Draw( sf::RenderTarget *target )
{
	target->draw( windowVA, 12, sf::Quads, ts_bar->texture );
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
				state = STATE_SHOW_FACE;
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
	int trueHeight = max( height, origMiddleHeight );
	currMiddleHeight = trueHeight;
	SetBottomLeftPos( bottomLeft );
}

