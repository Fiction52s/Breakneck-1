#include "VictoryScreen.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

VictoryScreen2PlayerVS::VictoryScreen2PlayerVS( GameSession *p_owner )
	:owner( p_owner )
{
	player1Bar = new PlayerInfoBar( owner );
	player2Bar = new PlayerInfoBar( owner );

	player2Bar->SetBottomLeftPos( Vector2f( 0, 128 ) );
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
	ts_bar = owner->GetTileset( "playerinfobar_128x20.png", 128, 20 );

	AssignTiles();
	
	currMiddleHeight = 20;

	botQuadHeight = 6;
	topQuadHeight = 6;
	totalWidth = 128;

	SetBottomLeftPos( Vector2f( 0, 0 ) );

	currState = STATE_SHOW_FACE;
	//infoBarVA[BOT_QUAD_INDEX]
}

void PlayerInfoBar::AssignTiles()
{
	int totalHeight = 20;
	int middleHeight = totalHeight - botQuadHeight - topQuadHeight;
	
	infoBarVA[BOT_QUAD_INDEX+0].texCoords = Vector2f( 0, totalHeight - botQuadHeight );
	infoBarVA[BOT_QUAD_INDEX+1].texCoords = Vector2f( totalWidth, totalHeight - botQuadHeight );
	infoBarVA[BOT_QUAD_INDEX+2].texCoords = Vector2f( totalWidth, totalHeight );
	infoBarVA[BOT_QUAD_INDEX+3].texCoords = Vector2f( 0, totalHeight );

	infoBarVA[STRETCH_QUAD_INDEX+0].texCoords = Vector2f( 0, topQuadHeight );
	infoBarVA[STRETCH_QUAD_INDEX+1].texCoords = Vector2f( totalWidth, topQuadHeight );
	infoBarVA[STRETCH_QUAD_INDEX+2].texCoords = Vector2f( totalWidth, topQuadHeight + middleHeight );
	infoBarVA[STRETCH_QUAD_INDEX+3].texCoords = Vector2f( 0, topQuadHeight + middleHeight );

	infoBarVA[TOP_QUAD_INDEX+0].texCoords = Vector2f( 0, 0 );
	infoBarVA[TOP_QUAD_INDEX+1].texCoords = Vector2f( totalWidth, 0 );
	infoBarVA[TOP_QUAD_INDEX+2].texCoords = Vector2f( totalWidth, topQuadHeight );
	infoBarVA[TOP_QUAD_INDEX+3].texCoords = Vector2f( 0, topQuadHeight );
}

void PlayerInfoBar::SetBottomLeftPos( sf::Vector2f &pos )
{
	int trueBottom = pos.y;
	int botTop = pos.y - botQuadHeight;
	int midTop = botTop - currMiddleHeight;
	int topTop = midTop - topQuadHeight;

	infoBarVA[BOT_QUAD_INDEX+0].position = Vector2f( pos.x, botTop );
	infoBarVA[BOT_QUAD_INDEX+1].position = Vector2f( pos.x + totalWidth, botTop );
	infoBarVA[BOT_QUAD_INDEX+2].position = Vector2f( pos.x + totalWidth, trueBottom );
	infoBarVA[BOT_QUAD_INDEX+3].position = Vector2f( pos.x, trueBottom );

	infoBarVA[STRETCH_QUAD_INDEX+0].position = Vector2f( pos.x, midTop );
	infoBarVA[STRETCH_QUAD_INDEX+1].position = Vector2f( pos.x + totalWidth, midTop );
	infoBarVA[STRETCH_QUAD_INDEX+2].position = Vector2f( pos.x + totalWidth, botTop );
	infoBarVA[STRETCH_QUAD_INDEX+3].position = Vector2f( pos.x, botTop );

	infoBarVA[TOP_QUAD_INDEX+0].position = Vector2f( pos.x, topTop );
	infoBarVA[TOP_QUAD_INDEX+1].position = Vector2f( pos.x + totalWidth, topTop );
	infoBarVA[TOP_QUAD_INDEX+2].position = Vector2f( pos.x + totalWidth, midTop );
	infoBarVA[TOP_QUAD_INDEX+3].position = Vector2f( pos.x, midTop );
}

const sf::Vector2f &PlayerInfoBar::GetBottomLeftPos()
{
	return infoBarVA[BOT_QUAD_INDEX+2].position;
}

void PlayerInfoBar::Draw( sf::RenderTarget *target )
{
	target->draw( infoBarVA, 12, sf::Quads, ts_bar->texture );
}