#include "TopClouds.h"
#include "Session.h"
#include "MapHeader.h"

using namespace std;
using namespace sf;

TopClouds::TopClouds()
{
	sess = Session::GetSession();
	ts = sess->GetTileset("Env/cloud_ceiling_256x256.png", 256, 256);
	animLength = 11;
	animFactor = 10;
	cloudVA = NULL;
}

void TopClouds::SetToHeader()
{
	//assert(sess->mapHeader != NULL);

	int width = sess->mapHeader->boundsWidth;
	int left= sess->mapHeader->leftBounds;
	int top = sess->mapHeader->topBounds;

	if (cloudVA != NULL)
	{
		if (currWidth == width && currLeft == left && currTop == top)
		{
			return;
		}
		else
		{
			delete[] cloudVA;
			cloudVA = NULL;
		}
	}

	currWidth = width;
	currLeft = left;
	currTop = top;

	numClouds = currWidth / ts->tileWidth;

	int remainder = currWidth  % ts->tileWidth;
	//deal with remainder later
	if (remainder > 0)
	{
		numClouds++;
	}


	//numClouds = 1;
	cloudVA = new Vertex[numClouds * 4];

	for (int i = 0; i < numClouds; ++i)
	{
		SetRectCenter(cloudVA + i * 4, 256, 256,
			Vector2f(currLeft + ts->tileWidth / 2 + i * ts->tileWidth, 
				currTop + ts->tileHeight / 2));
		//SetRectColor(cloudVA + i * 4, Color(Color::Red));
	}
}


TopClouds::~TopClouds()
{
	delete[] cloudVA;
}

void TopClouds::Update()
{
	int frame = (sess->totalGameFrames % (animFactor * animLength)) / animFactor;

	FloatRect camRect = sess->cam.GetRect();

	startIndex = (camRect.left - sess->mapHeader->leftBounds) / ts->tileWidth;
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