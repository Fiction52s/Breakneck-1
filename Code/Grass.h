#ifndef __GRASS_H__
#define __GRASS_H__

#include "QuadTree.h"
#include "Physics.h"

struct Tileset;
struct GameSession;
struct TerrainPolygon;
struct Grass : QuadTreeEntrant
{
	enum GrassType
	{
		DECELERATE,
		JUMP,
		GRAVREVERSE,
		ANTIGRAVREVERSE, 
		BOUNCE,
		ACCELERATE,
		ANTIGRIND,
		POISON,
		ANTIWIRE,
		BOOST,
		HIT,
		UNTECHABLE,
		Count
	};


	Grass(Tileset *p_ts_grass,
		int p_tileIndex,
		V2d &pos,
		TerrainPolygon *poly, GrassType gType);
	sf::Color GetColor(); //temp
	void Reset();
	V2d pos;
	double radius;
	void SetVisible(bool p_visible);
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	bool IsTouchingCircle(V2d &pos, double rad);
	static int GetGrassTypeFromString(const std::string &s);
	static std::string GetGrassStringFromType(int t);

	GrassType grassType;


	void Update();
	bool exploding;
	int tileIndex;
	Grass *next;
	Grass *prev;
	bool visible;
	Tileset *ts_grass;
	CollisionBox explosion;
	int explodeFrame;
	int explodeLimit;
	TerrainPolygon *poly;
	sf::IntRect aabb;
};

#endif