#ifndef __TOUCHGRASS_H__
#define __TOUCHGRASS_H__

#include "QuadTree.h"

struct TerrainPiece;
struct Edge;
struct Tileset;


struct TouchGrass : QuadTreeEntrant
{
	enum TouchGrassType
	{
		TYPE_NORMAL
	};

	enum Action
	{
		STILL,
		TOUCHED
	};


	TouchGrass(TouchGrassType type, int index, sf::Vertex *va,
		Tileset *ts, Edge *e, double quant,
		double rad, double yOff);
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	void Update();
	void Touch();
	Action action;
	TouchGrassType gType;
	Edge *edge;
	double quant;
	int gIndex;
	TerrainPiece *poly;
	double yOffset;
	double radius;
	Tileset *ts;
	sf::Vertex *va;
	sf::Vertex *myQuad;
	int frame;
	void UpdateSprite();
	sf::Vector2<double> points[4];
};

struct QuadTree;
struct TouchGrassCollection
{
	TouchGrassCollection();
	~TouchGrassCollection();
	QuadTree * touchGrassTree;
	sf::Vertex *touchGrassVA;
	int numTouchGrasses;
	void Draw(sf::RenderTarget *target);
	void Query(QuadTreeCollider *qtc, sf::Rect<double> &r);
	void UpdateGrass();
	std::list<TouchGrass*> myGrass;
};

#endif