#ifndef __ZONE__H_
#define __ZONE__H_

#include <SFML/Graphics.hpp>
#include "Gate.h"
#include <list>
#include "EditSession.h"
#include "poly2tri\poly2tri.h"

//typedef std::pair <Gate*,bool> GateBool;

struct Zone
{
	Zone( TerrainPolygon & tp );
	~Zone();
	
	void Init();
	void Draw( sf::RenderTarget *target );
	void AddHoles( p2t::CDT *cdt );

	bool ContainsPoint( sf::Vector2i point );
	bool ContainsZone( Zone *z );
	//bool ContainsPlayer();
	Zone * ContainsPointMostSpecific( 
		sf::Vector2i test );

	sf::VertexArray *definedArea;
	std::list<Edge*> gates;
	std::list<sf::Vector2i> points;
	std::list<Zone*> subZones;
	bool active;

	Zone *activeNext;
};



#endif