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
	ZoneNode();
	~ZoneNode();
	Zone *myZone;
	std::vector<ZoneNode*> children;
	ZoneNode *parent;
	//int zoneNodeID;

	void SetChildrenShouldNotReform();
	bool IsInMyBranch(Zone *z);
	bool SetZone(Zone *myZone);
};

struct Zone
{
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
		CountZ
	};

	struct MyData
	{
		int action;
		int framesSinceActivation;
		int frame;

		bool visited;
		bool active;
		bool reexplored;
		bool shouldReform;
	};


	int zoneIndex;

	ZoneNode *myNode;
	TerrainPolygon *zonePoly; //for raycasts to check if I'm inside it etc

	MyData data;

	Zone *parentZone;
	bool secretZone;
	int openFrames;
	int closeFrames;
	int zType;
	bool hasGoal;
	int totalNumKeys;
	sf::VertexArray *definedArea;
	std::list<Edge*> gates;
	std::vector<std::vector<sf::Vector2i>> pointVector;
	std::list<Zone*> subZones;
	std::list<Enemy*> spawnEnemies;
	std::list<Enemy*> allEnemies;
	std::set<Zone*> connectedSet;
	bool showShadow;
	sf::Shader *zShader;
	sf::Shader *miniShader;
	Tileset *ts_z;
	

	Zone( TerrainPolygon * tp );
	~Zone();
	void Init();
	void Draw( sf::RenderTarget *target );
	void DrawMinimap(sf::RenderTarget *target);
	bool ContainsPoint( V2d point );
	bool ContainsZone( Zone *z );
	void Close();
	void SetShadowColor( sf::Color c );
	bool HasEnemyGate();
	bool HasKeyGateOfNumber(int n);
	Zone * ContainsPointMostSpecific( 
		V2d point );
	bool ContainsZoneMostSpecific(
		Zone *z);
	void Update( float zoom, sf::Vector2f &topLeft,
		sf::Vector2f &playertest );
	void Update();
	int GetNumRemainingKillableEnemies();
	int GetNumRemainingCollectiblePowers();
	float GetOpeningAlpha();
	void SetZoneType( int zt );
	void Reset();
	void ReformAllGates( Gate *ignoreGate = NULL);
	std::vector<sf::Vector2i> &PointVector();
	sf::Vector2i &GetPolyPoint(int index);
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	bool Activate(bool instant);
	bool IsOpening();
	void SetClosing( int alreadyOpenFrames );
	void CloseOffIfLimited();
	void SetShouldReform(bool on);
	bool ShouldReform();
	bool IsActive();
	int GetFramesSinceActivation();
	bool IsShowingEnemyZoneSprites();
	bool IsStartingToOpen();
	int GetFrame();
};



#endif