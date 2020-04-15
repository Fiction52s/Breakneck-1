#ifndef __TOUCHGRASS_H__
#define __TOUCHGRASS_H__

#include "QuadTree.h"
#include "Physics.h"

struct TerrainPiece;
struct Edge;
struct Tileset;
struct Actor;
struct QuadTree;
struct TouchGrassCollection;

struct TouchGrass : QuadTreeEntrant
{
	enum TouchGrassType
	{
		TYPE_NORMAL,
		TYPE_TEST,
	};

	static bool IsPlacementOkay(
		TouchGrassType grassType, 
		int eat,
		Edge *edge, int quadIndex);
	static int GetQuadWidth(TouchGrassType gt);
	TouchGrass( TouchGrassCollection *coll, int index,
		Edge *e, double quant);
	~TouchGrass();
	void CommonInit(double yOff, double angle, double hitboxYOff,
		double hitboxXSize, double hitboxYSize);
	virtual void Reset() = 0;
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	virtual void Update() = 0;
	virtual void Touch(Actor *a) = 0;
	virtual void Destroy(Actor *a) = 0;
	bool Intersects(CollisionBody *cb, int frame);
	virtual void UpdateSprite() = 0;

	bool visible;

	CollisionBody *hurtBody;
	//CollisionBody *hitBody;


	V2d center;
	float angle;
	
	Edge *edge;
	double quant;
	int gIndex;
	
	double yOffset;

	sf::Vertex *myQuad;
	int frame;

	sf::Vector2<double> points[4];
	TouchGrassCollection *coll;
};

struct TouchGrassCollection
{
	TouchGrassCollection();
	void Reset();
	~TouchGrassCollection();
	QuadTree * touchGrassTree;
	sf::Vertex *touchGrassVA;
	int numTouchGrasses;
	void Draw(sf::RenderTarget *target);
	void Query(QuadTreeCollider *qtc, sf::Rect<double> &r);
	void UpdateGrass();
	void CreateGrass(int index, Edge *edge, double quant);
	static Tileset *GetTileset(GameSession *owner, TouchGrass::TouchGrassType gt);
	std::list<TouchGrass*> myGrass;
	Tileset *ts_grass;
	TouchGrass::TouchGrassType gType;
	TerrainPiece *poly;
	HitboxInfo *hitboxInfo;
};



struct BasicTouchGrass : TouchGrass
{
	enum Action
	{
		STILL,
		TOUCHEDLEFT,
		TOUCHEDRIGHT,
		TOUCHEDLAND,
	};
	Action action;

	BasicTouchGrass(TouchGrassCollection *coll, int index,
		Edge *e, double quant);
	void Reset();
	void Update();
	void Touch(Actor *a);
	void Destroy(Actor *a);
	void UpdateSprite();
	Tileset *myTS;
};

struct TestTouchGrass : TouchGrass
{
	enum Action
	{
		STILL,
		TOUCHEDLEFT,
		TOUCHEDRIGHT,
		TOUCHEDLAND,
	};
	Action action;

	TestTouchGrass(TouchGrassCollection *coll, int index,
		Edge *e, double quant);
	void Reset();
	void Update();
	void Touch(Actor *a);
	void Destroy(Actor *a);
	void UpdateSprite();
};

#endif