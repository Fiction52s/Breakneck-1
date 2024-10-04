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
struct TilesetManager;
struct TerrainPolygon;

struct TouchGrass : QuadTreeEntrant
{
	enum TouchGrassType
	{
		TYPE_NORMAL_W1,
		TYPE_NORMAL_W2,
		TYPE_NORMAL_W3,
		TYPE_NORMAL_W4,
		TYPE_NORMAL_W5,
		TYPE_NORMAL_W6,
		//TYPE_NORMAL_W7,
		TYPE_LARGE_W1,
		TYPE_LARGE_W2,
		TYPE_LARGE_W3,
		TYPE_LARGE_W4,
		TYPE_LARGE_W5,
		TYPE_LARGE_W6,
		//TYPE_LARGE_W7,
		TYPE_TREE,
		TYPE_PALM,
	};

	static bool IsPlacementOkay(
		TouchGrassType grassType, int variation,
		Edge *edge, int quadIndex);
	static bool IsEdgeOkay(TouchGrassType grassType, Edge *e);
	static int GetQuadWidth(TouchGrassType gt);
	static int GetRandomVariation(TouchGrassType gt);
	TouchGrass( TouchGrassCollection *coll, int index,
		Edge *e, double quant);
	~TouchGrass();

	void Move(V2d &move);
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

	CollisionBody hurtBody;
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
	TouchGrassCollection(TerrainPolygon *tp);
	void Reset();
	~TouchGrassCollection();
	QuadTree * touchGrassTree;
	sf::Vertex *touchGrassVA;

	void Move(V2d &move);
	bool IsDrawnBehind();
	int numTouchGrasses;
	void Draw(sf::RenderTarget *target);
	void Query(QuadTreeCollider *qtc, sf::Rect<double> &r);
	void UpdateGrass();
	void CreateGrass(int index, Edge *edge, double quant, int variation);
	//static Tileset *GetTileset(GameSession *owner, TouchGrass::TouchGrassType gt);
	static Tileset *GetTileset(TilesetManager *tm, TouchGrass::TouchGrassType gt);
	std::list<TouchGrass*> myGrass;
	Tileset *ts_grass;
	TouchGrass::TouchGrassType gType;
	TerrainPiece *poly;
	HitboxInfo *hitboxInfo;

	TerrainPolygon *myTerrain;
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
	//Tileset *myTS;
	int currTile;
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

	int currTile;
};

struct TouchTree : TouchGrass
{
	enum Action
	{
		STILL,
		TOUCHEDLEFT,
		TOUCHEDRIGHT,
		TOUCHEDLAND,
	};

	Action action;
	int currTile;
	float treeHeight;

	TouchTree(TouchGrassCollection *coll, int index,
		Edge *e, double quant);
	void Reset();
	void Update();
	void Touch(Actor *a);
	void Destroy(Actor *a);
	void UpdateSprite();
};

struct TouchPalm : TouchGrass
{
	enum Action
	{
		STILL,
		TOUCHEDLEFT,
		TOUCHEDRIGHT,
		TOUCHEDLAND,
	};

	sf::Vector2i size;
	Action action;
	int currTile;

	sf::Vector2i spriteOrigin;

	TouchPalm(TouchGrassCollection *coll, int index,
		Edge *e, double quant, int variation);
	void Reset();
	void Update();
	void Touch(Actor *a);
	void Destroy(Actor *a);
	void UpdateSprite();
};

struct PlantInfo
{
	PlantInfo(Edge*e, double q, double w, int p_variation)
		:edge(e), quant(q), quadWidth(w), variation(p_variation)
	{
	}
	Edge *edge;
	double quant;
	double quadWidth;
	int variation;
};

#endif