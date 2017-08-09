#ifndef __RAIL_H__
#define __RAIL_H__


#include "Actor.h"
#include <list>
#include "Mover.h"
#include "Movement.h"
#include "Physics.h"

struct Rail
{
	Rail(GameSession *owner, sf::Vector2i &pos, 
		std::list<sf::Vector2i> &path, bool energized );
	Edge ** edges;
	int numEdges;
	sf::Vertex *va;
	bool energized;
	bool playerAttached;

	Tileset *ts_rail;
	void UpdateSprite();
	void Draw(sf::RenderTarget *target);
	GameSession *owner;
};

#endif