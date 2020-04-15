#ifndef __TERRAINDECOR_H__
#define __TERRAINDECOR_H__

#include <SFML/Graphics.hpp>
#include <list>
#include "DecorTypes.h"

#include "QuadTree.h"

struct Tileset;


struct DecorLayer
{
	DecorLayer(Tileset *ts, int animLength,
		int animFactor, int tileStart = 0,
		int loopWait = 0);
	~DecorLayer();

	void Update();
	Tileset *ts;
	int frame;
	int animLength;
	int animFactor;
	int startTile;
	int loopWait;
};

struct DecorExpression
{
	DecorExpression(
		std::list<sf::Vector2f> &pointList,
		DecorLayer *layer);
	~DecorExpression();

	sf::VertexArray *va;
	DecorLayer *layer;

	void UpdateSprites();
};

struct TerrainDecorInfo
{
	TerrainDecorInfo(int numDecors);
	~TerrainDecorInfo();
	DecorType *decors;
	int *percents;
	int numDecors;
};

struct DecorRect : QuadTreeEntrant
{
	DecorRect(sf::Rect<double> &r)
		:rect(r)
	{

	}
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	sf::Rect<double> rect;
};

#endif