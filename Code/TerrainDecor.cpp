#include "Tileset.h"
#include "TerrainDecor.h"
#include <assert.h>
#include "VectorMath.h"

using namespace sf;
using namespace std;


DecorLayer::DecorLayer(Tileset *p_ts, int p_animLength, int p_animFactor, int p_startTile, int p_loopWait)
	:ts(p_ts), frame(0), animLength(p_animLength), startTile(p_startTile), animFactor(p_animFactor),
	loopWait(p_loopWait)
{

}

DecorLayer::~DecorLayer()
{
}

void DecorLayer::Update()
{
	++frame;
	if (frame == animLength * animFactor + loopWait)
	{
		frame = 0;
	}
}


DecorExpression::DecorExpression(std::list<sf::Vector2f> &pointList,
	DecorLayer *p_layer)
	:layer(p_layer)
{
	int numBushes = pointList.size();
	//cout << "numBushes: " << numBushes << endl;
	Tileset *ts = layer->ts;

	vaSize = numBushes * 4;

	va = new Vertex[vaSize];

	IntRect subRect = ts->GetSubRect(0);
	list<Vector2f>::iterator posIt;
	if (numBushes > 0)
		posIt = pointList.begin();

	Vector2f p;
	for (int i = 0; i < numBushes; ++i)
	{
		p = (*posIt);
		//cout << "i: " << i << ", p: " <<  p.x << ", " << p.y << endl;
		va[i * 4 + 0].position = Vector2f(p.x - subRect.width / 2, p.y - subRect.height / 2);
		va[i * 4 + 1].position = Vector2f(p.x + subRect.width / 2, p.y - subRect.height / 2);
		va[i * 4 + 2].position = Vector2f(p.x + subRect.width / 2, p.y + subRect.height / 2);
		va[i * 4 + 3].position = Vector2f(p.x - subRect.width / 2, p.y + subRect.height / 2);

		/*VA[i*4+0].color= Color::Red;
		VA[i*4+1].color= Color::Red;
		VA[i*4+2].color= Color::Red;
		VA[i*4+3].color= Color::Red;*/

		va[i * 4 + 0].texCoords = Vector2f(subRect.left, subRect.top);
		va[i * 4 + 1].texCoords = Vector2f(subRect.left + subRect.width, subRect.top);
		va[i * 4 + 2].texCoords = Vector2f(subRect.left + subRect.width, subRect.top + subRect.height);
		va[i * 4 + 3].texCoords = Vector2f(subRect.left, subRect.top + subRect.height);

		++posIt;
	}
}

void DecorExpression::Move(sf::Vector2f &move)
{
	for (int i = 0; i < vaSize; ++i)
	{
		va[i].position += move;
	}
}

void DecorExpression::Draw(sf::RenderTarget *target)
{
	target->draw(va, vaSize, sf::Quads, layer->ts->texture);
}

DecorExpression::~DecorExpression()
{
	delete [] va;
}

void DecorExpression::UpdateSprites()
{
	int numBushes = vaSize / 4;

	Tileset *ts_bush = layer->ts;
	int frame = max(layer->frame - layer->loopWait, 0);
	int animLength = layer->animLength;
	int animFactor = layer->animFactor;

	IntRect subRect = ts_bush->GetSubRect((layer->startTile + frame) / animFactor);

	for (int i = 0; i < numBushes; ++i)
	{
		SetRectSubRect(va + i * 4, subRect);
	}
}

TerrainDecorInfo::TerrainDecorInfo(int p_numDecors)
{
	numDecors = p_numDecors;
	assert(numDecors != 0);
	decors = new DecorType[numDecors];
	percents = new int[numDecors];

}

TerrainDecorInfo::~TerrainDecorInfo()
{
	delete[] decors;
	delete[] percents;
}

void DecorRect::HandleQuery(QuadTreeCollider * qtc)
{
	if (qtc != NULL)
		qtc->HandleEntrant(this);
}

bool DecorRect::IsTouchingBox(const sf::Rect<double> &r)
{
	return rect.intersects(r);
}