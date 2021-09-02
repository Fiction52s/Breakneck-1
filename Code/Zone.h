#ifndef __ZONE__H_
#define __ZONE__H_

#include <SFML/Graphics.hpp>
#include "Gate.h"
#include <list>
#include <set>
#include "VectorMath.h"

//typedef std::pair <Gate*,bool> GateBool;
struct Enemy;
struct GameSession;
struct TerrainPolygon;
struct Zone;



struct ZoneNode
{
	~ZoneNode();
	Zone *myZone;
	std::vector<ZoneNode*> children;
	ZoneNode *parent;

	void SetChildrenShouldNotReform();
	bool IsInMyBranch(Zone *z);
	bool SetZone(Zone *myZone);
};

struct Zone
{
	Zone( TerrainPolygon & tp );
	~Zone();
	
	void Init();
	void Draw( sf::RenderTarget *target );
	void DrawMinimap(sf::RenderTarget *target);

	bool ContainsPoint( V2d point );
	bool ContainsZone( Zone *z );
	void Close();
	void SetShadowColor( sf::Color c );
	bool HasEnemyGate();
	//bool ContainsPlayer(); super 
	Zone * ContainsPointMostSpecific( 
		sf::Vector2i test );
	bool ContainsZoneMostSpecific(
		Zone *z);
	void Update( float zoom, sf::Vector2f &topLeft,
		sf::Vector2f &playertest );
	void Update();
	int GetNumRemainingKillableEnemies();
	int frame;
	float GetOpeningAlpha();
	bool secretZone;

	bool shouldReform;

	bool visited;
	bool reexplored;
	int openFrames;
	int closeFrames;
	int totalNumKeys;

	enum ZoneType : int
	{
		NORMAL,
		MOMENTA,
		SECRET,
		NEXUS,
		EXTRA
	};

	enum Action
	{
		UNEXPLORED,
		OPENING,
		OPEN,
		CLOSING,
		CLOSED,
		Count
	};

	Action action;
	void SetZoneType( int zt );
	int zType;
	void Reset();
	void ReformAllGates( Gate *ignoreGate = NULL);

	int framesSinceActivation;
	bool hasGoal;

	sf::VertexArray *definedArea;
	std::list<Edge*> gates;
	std::vector<std::vector<sf::Vector2i>> pointVector;
	std::vector<sf::Vector2i> &PointVector();
	sf::Vector2i &GetPolyPoint(int index);
	std::list<Zone*> subZones;
	std::list<Enemy*> spawnEnemies;
	std::list<Enemy*> allEnemies;
	std::set<Zone*> connectedSet;
	bool active;
	
	//int goalIndex;
	//int connectedCount;

	bool showShadow;
	Zone *activeNext;

	Zone *parentZone;

	sf::Shader *zShader;
	sf::Shader *miniShader;
	Tileset *ts_z;
};



#endif