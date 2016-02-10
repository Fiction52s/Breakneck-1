#ifndef __ZONE__H_
#define __ZONE__H_

#include <SFML/Graphics.hpp>
#include "Gate.h"
#include <list>
#include "EditSession.h"

struct Zone
{
	Zone( TerrainPolygon & tp );
	~Zone();
	sf::VertexArray *definedArea;
	std::list<Gate*> gates;
	void Draw( sf::RenderTarget *target );
};



#endif