#ifndef __ZONE__H_
#define __ZONE__H_

#include <SFML/Graphics.hpp>
#include "Gate.h"
#include <list>
#include "EditSession.h"

//typedef std::pair <Gate*,bool> GateBool;

struct Zone
{
	Zone( TerrainPolygon & tp );
	~Zone();
	
	void Draw( sf::RenderTarget *target );

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
};



#endif