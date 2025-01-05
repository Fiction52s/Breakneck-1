#include "EnvEffects.h"
#include "Session.h"
#include <math.h>

using namespace std;
using namespace sf;

const int Rain::TILE_WIDTH = 160;
const int Rain::TILE_HEIGHT = 160;
const int Rain::NUM_COLS = (int)(ceil( 1920.0 * 2.25 / TILE_WIDTH ));
const int Rain::NUM_ROWS = (int)(ceil( 1080.0 * 2.25 / TILE_HEIGHT ));
const int Rain::TOTAL_QUADS = NUM_ROWS * NUM_COLS;
const int Rain::ANIM_FACTOR = 10;
	


Rain::Rain() 
{
	va = new Vertex[TOTAL_QUADS * 4];
	sess = Session::GetSession();
	ts_rain = sess->GetSizedTileset("Env/rain_160x160.png");
	frame = 0;
	loopLength = 3;
	angle = PI * .1;
}

Rain::~Rain()
{
	delete[] va;
}

void Rain::Reset()
{
	frame = 0;
	for (int i = 0; i < TOTAL_QUADS; ++i)
	{
		ClearRect(va + i * 4);
	}
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

	Vector2f tempPos;

	Transform t;

	float angD = angle / PI * 180.f;

	t.rotate(angD);

	int index = 0;
	for( int x = 0; x < NUM_COLS; ++x )
	{
		for( int y = 0; y < NUM_ROWS; ++y )
		{
			tempPos = Vector2f(pos.x + x * TILE_WIDTH + TILE_WIDTH / 2, pos.y + y * TILE_HEIGHT + TILE_HEIGHT / 2);
			tempPos = t.transformPoint(tempPos);

			SetRectRotation( va + index * 4, angle, TILE_WIDTH, TILE_HEIGHT, tempPos );
			IntRect subRect = ts_rain->GetSubRect(frame / ANIM_FACTOR);
			SetRectSubRect(va + index * 4, subRect);

			//va[index+0].position = Vector2f( pos.x + x * TILE_WIDTH, pos.y + y * TILE_HEIGHT );
			//va[index+1].position = Vector2f( pos.x + x * TILE_WIDTH + TILE_WIDTH, pos.y + y * TILE_HEIGHT );
			//va[index+2].position = Vector2f( pos.x + x * TILE_WIDTH + TILE_WIDTH, pos.y + y * TILE_HEIGHT + TILE_HEIGHT );
			//va[index+3].position = Vector2f( pos.x + x * TILE_WIDTH, pos.y + y * TILE_HEIGHT + TILE_HEIGHT );

			
			//va[index+0].texCoords = Vector2f( subRect.left, subRect.top );
			//va[index+1].texCoords = Vector2f( subRect.left + subRect.width, subRect.top );
			//va[index+2].texCoords = Vector2f( subRect.left + subRect.width, subRect.top + subRect.height );
			//va[index+3].texCoords =Vector2f( subRect.left, subRect.top + subRect.height );

			index++;
		}
	}

	++frame;
}

void Rain::Draw( RenderTarget *target )
{
	sf::View oldView = target->getView();
	rainView.setCenter((int)oldView.getCenter().x % TILE_WIDTH, (int)oldView.getCenter().y % TILE_HEIGHT); //was 64 before?
	rainView.setSize(oldView.getSize());
	target->setView(rainView);
	target->draw( va, TOTAL_QUADS * 4, sf::Quads, ts_rain->texture );
	target->setView(oldView);
}

