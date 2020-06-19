#include "EnvEffects.h"
#include "Session.h"
#include <math.h>

using namespace std;
using namespace sf;

const int Rain::TILE_WIDTH = 64;
const int Rain::TILE_HEIGHT = 64;
const int Rain::NUM_COLS = (int)(ceil( 1920.0 * 2.25 / TILE_WIDTH ));
const int Rain::NUM_ROWS = (int)(ceil( 1080.0 * 2.25 / TILE_HEIGHT ));
const int Rain::TOTAL_QUADS = NUM_ROWS * NUM_COLS;
const int Rain::ANIM_FACTOR = 5;
	


Rain::Rain()
	:va( sf::Quads, TOTAL_QUADS * 4 ) 
{
	sess = Session::GetSession();
	ts_rain = sess->GetTileset( "Env/rain_64x64.png", 64, 64 );
	frame = 0;
	loopLength = 7;
}

void Rain::Reset()
{
	frame = 0;

}

void Rain::Update()
{
	if( frame == loopLength * ANIM_FACTOR )
	{
		frame = 0;
	}

	int xPos = (int)sess->cam.pos.x;
	int yPos = (int)sess->cam.pos.y;

	xPos = xPos % TILE_WIDTH;
	yPos = yPos % TILE_HEIGHT;

	Vector2f pos = Vector2f( 0, 0 );//owner->cam.pos;// - Vector2f( 1920 / 2, 1080 / 2 );
	pos.x -= NUM_COLS / 2 * TILE_WIDTH;
	pos.y -= NUM_ROWS / 2 * TILE_HEIGHT;
	//pos.x += xPos;
	//pos.y += yPos;

	int index = 0;
	for( int x = 0; x < NUM_COLS; ++x )
	{
		for( int y = 0; y < NUM_ROWS; ++y )
		{
			va[index+0].position = Vector2f( pos.x + x * TILE_WIDTH, pos.y + y * TILE_HEIGHT );
			va[index+1].position = Vector2f( pos.x + x * TILE_WIDTH + TILE_WIDTH, pos.y + y * TILE_HEIGHT );
			va[index+2].position = Vector2f( pos.x + x * TILE_WIDTH + TILE_WIDTH, pos.y + y * TILE_HEIGHT + TILE_HEIGHT );
			va[index+3].position = Vector2f( pos.x + x * TILE_WIDTH, pos.y + y * TILE_HEIGHT + TILE_HEIGHT );

			/*va[index+0].color = Color::Red;
			va[index+1].color = Color::Red;
			va[index+2].color = Color::Red;
			va[index+3].color = Color::Red;*/

			IntRect subRect = ts_rain->GetSubRect( frame / ANIM_FACTOR );
			va[index+0].texCoords = Vector2f( subRect.left, subRect.top );
			va[index+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
			va[index+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
			va[index+3].texCoords =Vector2f( subRect.left, subRect.top + subRect.height );

			index += 4;
		}
	}

	++frame;
}

void Rain::Draw( RenderTarget *target )
{
	target->draw( va, ts_rain->texture );
}

