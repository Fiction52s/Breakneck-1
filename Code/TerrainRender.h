#ifndef __TERRAIN_RENDER_H__
#define __TERRAIN_RENDER_H__

#include <SFML\Graphics.hpp>
#include "Physics.h"

struct Tileset;
struct Edge;

#define STEEP_THRESH .4

enum EdgeType : int
{
	E_FLAT_GROUND,
	E_SLOPED_GROUND,
	E_STEEP_GROUND,
	E_SLOPED_CEILING,
	E_STEEP_CEILING,
	E_WALL,
	E_TRANS_FLAT_TO_SLOPED,
	E_TRANS_FLAT_TO_STEEP,
	E_TRANS_FLAT_TO_STEEP_CEILING,
	E_TRANS_FLAT_TO_WALL,
	E_TRANS_FLAT_TO_SLOPED_CEILING,
	E_TRANS_SLOPED_TO_STEEP,
	E_TRANS_SLOPED_TO_STEEP_CEILING,
	E_TRANS_SLOPED_TO_WALL,
	E_TRANS_SLOPED_TO_SLOPED_CEILING,
	E_TRANS_STEEP_TO_STEEP_CEILING,
	E_TRANS_STEEP_TO_WALL,
	E_TRANS_STEEP_TO_SLOPED_CEILING,
	E_TRANS_WALL_TO_STEEP_CEILING,
	E_TRANS_STEEP_CEILING_TO_SLOPED_CEILING,
	E_TRANS_WALL_TO_SLOPED_CEILING
	//E_CEILING
};
static EdgeType GetEdgeNormalType(sf::Vector2<double> &norm);
static EdgeType GetEdgeType(sf::Vector2<double> &dir);
static EdgeType GetEdgeTransType(Edge *e);

enum TerrainWorldType
{
	T_MOUNTAIN,
	T_GLADE,
	T_DESERT,
	T_COVE,
	T_JUNGLE,
	T_FORTRESS,
	T_CORE
	//Count
};


struct Edge;
struct QuadTree;

struct TerrainRender : RayCastHandler
{
	sf::Vertex *borderVA;
	sf::Vertex *centerVA;
	void SetType(int tWorldType, int tWorldVariation);
	int terrainWorldType;
	int terrainWorldVariation;
	void GenerateCenterMesh();
	void GenerateBorderMesh( QuadTree *qt );

	static sf::IntRect GetBorderSubRect(int tileWidth, EdgeType et, int var);
	static int GetBorderQuadIntersect(int tileWidth);
	static int GetBorderRealWidth(int tileWidth, bool border );
	static double GetExtraForInward(Edge *e);
	static double GetSubForOutward(Edge *e);

	//void ApplyUpdates();
	void Draw(sf::RenderTarget *target);

	Tileset *ts_border;
	int totalNumBorderQuads;
	void HandleRayCollision(Edge *edge,
		double edgeQuantity, double rayPortion);
	//TerrainPoint *startPoint;
	//TerrainPoint *endPoint;
	Edge *startEdge;

	Edge * rcEdge;
	double rcQuant;
	Edge *ignoreEdge;
	double rcPortion;
};

#endif