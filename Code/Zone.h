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
	sf::VertexArray *definedArea;
	std::list<Edge*> gates;
	void Draw( sf::RenderTarget *target );
};



#endif