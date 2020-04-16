#include "TopClouds.h"
#include "GameSession.h"
#include "MapHeader.h"

using namespace std;
using namespace sf;

TopClouds::TopClouds( GameSession *p_owner )
	:owner( p_owner )
{
	ts = owner->GetTileset("FX/cloud_ceiling_256x256.png", 256, 256);
	assert(owner->mapHeader != NULL);
	int boundsWidth = owner->mapHeader->boundsWidth;
	int left = owner->mapHeader->leftBounds;
	int top = owner->mapHeader->topBounds;

	numClouds = boundsWidth / ts->tileWidth;

	int remainder = boundsWidth % ts->tileWidth;
	//deal with remainder later
	/*if (remainder != 0)
	{
		numClouds++;
	*/
	animLength = 11;
	animFactor = 10;

	//numClouds = 1;

	cloudVA = new Vertex[numClouds * 4];

	
	for (int i = 0; i < numClouds; ++i)
	{
		SetRectCenter(cloudVA + i * 4, 256, 256, 
			Vector2f(left + ts->tileWidth / 2 + i * ts->tileWidth, top + ts->tileHeight / 2 ));
		//SetRectColor(cloudVA + i * 4, Color(Color::Red));
	}
}

TopClouds::~TopClouds()
{
	delete[] cloudVA;
}

void TopClouds::Update()
{
	int frame = (owner->totalGameFrames % (animFactor * animLength)) / animFactor;

	FloatRect camRect = owner->cam.GetRect();

	startIndex = (camRect.left - owner->mapHeader->leftBounds) / ts->tileWidth;
	endIndex = startIndex + camRect.width / ts->tileWidth;

	//fix efficiency later

	startIndex = 0;
	endIndex = numClouds;
	for (int i = startIndex; i < endIndex; ++i)
	{
		SetRectSubRect(cloudVA + i * 4, ts->GetSubRect(frame));
	}
}

void TopClouds::Draw(sf::RenderTarget *target)
{
	int numVerts = (endIndex - startIndex) * 4;
	target->draw(cloudVA, numVerts, sf::Quads, ts->texture);
}