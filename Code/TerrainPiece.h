#ifndef __TERRAINPIECE_H__
#define __TERRAINPIECE_H__

//#include "Physics.h"
//#include "Tileset.h"
//#include <list>

//#include <SFML/Graphics.hpp>
//#include "Light.h"
//#include "Camera.h"
//#include "Gate.h"
//#include "Zone.h"
//#include "AirParticles.h"
//#include "Movement.h"
//#include "SoundManager.h"
//#include <map>
//#include "BarrierReactions.h"
//#include "Buf.h"
//#include "EnvEffects.h"
//#include "EffectLayer.h"
//#include <boost/filesystem.hpp>"
//#include "DecorTypes.h"

#include "QuadTree.h"
#include <list>
#include "Physics.h"

struct GameSession;
struct TerrainRender;
struct DecorExpression;
struct TouchGrassCollection;
struct Edge;
struct Tileset;

struct PlantInfo
{
	PlantInfo(Edge*e, double q, double w)
		:edge(e), quant(q), quadWidth(w)
	{
	}
	Edge *edge;
	double quant;
	double quadWidth;
};

struct GrassSegment
{
	CollisionBox explosion;
	GrassSegment(int edgeI, int grassIndex, int rep)
		:edgeIndex(edgeI), index(grassIndex),
		reps(rep)
	{
	}
	int edgeIndex;
	int index;
	int reps;

};

struct TerrainPiece : QuadTreeEntrant
{
	TerrainPiece(GameSession *owner);
	~TerrainPiece();
	void Reset();
	TerrainRender *tr;
	void AddDecorExpression(DecorExpression *expr);
	void AddTouchGrass(int gType);
	void UpdateBushSprites();
	void DrawBushes(sf::RenderTarget *target);
	GameSession *owner;
	std::list<TouchGrassCollection*> touchGrassCollections;
	void QueryTouchGrass(QuadTreeCollider *qtc, sf::Rect<double> &r);
	void UpdateTouchGrass();


	void SetupGrass(Edge * e, int &i);
	int GetNumGrass(Edge *e, bool &rem);
	void SetupGrass(std::list<GrassSegment> &segments);
	int grassSize;
	int grassSpacing;

	bool visible;
	std::list<DecorExpression*> bushes;
	sf::VertexArray *groundva;
	Tileset *ts_border;
	sf::VertexArray *slopeva;
	sf::VertexArray *steepva;
	sf::VertexArray *wallva;
	sf::VertexArray *triva;
	sf::VertexArray *flowva;
	sf::VertexArray *plantva;
	sf::VertexArray *decorLayer0va;
	sf::VertexArray *bushVA;
	bool inverse;
	Tileset *ts_plant;
	Tileset *ts_terrain;
	Tileset *ts_bush; //plant = surface
	int numPoints;
	//bush = middle area

	sf::Shader *pShader;
	//TerrainPolygon::TerrainType terrainType;
	int terrainWorldType;
	int terrainVariation;
	//int terrainType;
	//EditSession
	//TerrainPolygon::material
	static void UpdateBushFrame();
	sf::VertexArray *terrainVA;
	sf::VertexArray *grassVA;
	bool show;
	//TerrainPiece *prev;
	TerrainPiece *next;
	sf::Rect<double> aabb;
	int startEdgeIndex;
	//double polyArea;
	void UpdateBushes();
	void Draw(sf::RenderTarget *target);
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
};

#endif