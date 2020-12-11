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
		JUMP,
		DECELERATE,
		GRAVREVERSE,
		ANTIGRAVREVERSE, 
		BOUNCE,
		ACCELERATE,
		ANTIGRIND,
		POISON,
		ANTIWIRE,
		UNTECHABLE,
		KILL,
		Count
	};


	Grass(Tileset *p_ts_grass,
		int p_tileIndex,
		V2d &pos,
		TerrainPolygon *poly, GrassType gType);
	static sf::Color GetColor( int gType ); //temp
	void Reset();
	V2d pos;
	double radius;
	void SetVisible(bool p_visible);
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	bool IsTouchingCircle(V2d &pos, double rad);

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