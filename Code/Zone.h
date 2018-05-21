#ifndef __ZONE__H_
#define __ZONE__H_

#include <SFML/Graphics.hpp>
#include "Gate.h"
#include <list>
#include "EditSession.h"
#include "poly2tri\poly2tri.h"

//typedef std::pair <Gate*,bool> GateBool;
struct Enemy;
struct GameSession;
struct Zone
{
	Zone( GameSession *owner, TerrainPolygon & tp );
	~Zone();
	
	void Init();
	void Draw( sf::RenderTarget *target );
	void AddHoles( p2t::CDT *cdt );

	bool ContainsPoint( sf::Vector2<double> point );
	bool ContainsZone( Zone *z );
	void SetShadowColor( sf::Color c );
	//bool ContainsPlayer(); super 
	Zone * ContainsPointMostSpecific( 
		sf::Vector2i test );
	void Update( float zoom, sf::Vector2f &botLeft,
		sf::Vector2f &playertest );
	int frame;
	float GetOpeningAlpha();
	enum ZoneType
	{
		NORMAL,
		NEXUS,
		EXTRA
	};

	enum Action
	{
		UNEXPLORED,
		OPENING,
		OPEN,

		Count
	};

	Action action;
	void SetZoneType( ZoneType zt );
	ZoneType zType;
	void Reset();

	sf::VertexArray *definedArea;
	std::list<Edge*> gates;
	std::list<sf::Vector2i> points;
	std::list<Zone*> subZones;
	std::list<Enemy*> spawnEnemies;
	std::list<Enemy*> allEnemies;
	bool active;
	int requiredKeys;
	int totalStartingKeys;

	bool showShadow;
	GameSession *owner;
	Zone *activeNext;

	sf::Shader *zShader;
	Tileset *ts_z;
};



#endif