#ifndef __SPECIALTERRAIN_H__
#define __SPECIALTERRAIN_H__

#include "QuadTree.h"
#include <fstream>
#include "SpecialTerrainTypes.h"

struct GameSession;
struct TerrainRender;
struct DecorExpression;
struct Tileset;
struct Edge;

struct SpecialTerrainPiece : QuadTreeEntrant, QuadTreeCollider, RayCastHandler
{
	SpecialTerrainType specialType;

	SpecialTerrainPiece(GameSession *owner);
	~SpecialTerrainPiece();

	bool Load(std::ifstream &is);
	void Reset();
	void GenerateCenterMesh();
	void Draw(sf::RenderTarget *target);
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	Edge *GetEdge(int index);

	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdateAABB();

	bool IsInsideArea(V2d &point);

	V2d center;

	TerrainRender *tr;

	GameSession *owner;

	sf::VertexArray *groundva;
	Tileset *ts_border;
	sf::VertexArray *slopeva;
	sf::VertexArray *steepva;
	sf::VertexArray *wallva;
	sf::VertexArray *triva;

	Edge **edges;
	int numPoints;

	QuadTree *edgeTree;
	
	int numEdgesHit;

	V2d insideQueryPoint;


	Tileset *ts_terrain;

	sf::Shader *pShader;
	sf::VertexArray *terrainVA;
	bool show;
	SpecialTerrainPiece *next;
	sf::Rect<double> aabb;
	//double polyArea;
	
};

#endif