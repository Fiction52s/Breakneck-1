#ifndef __RAIL_H__
#define __RAIL_H__


#include "Actor.h"
#include <list>
#include "Mover.h"
#include "Movement.h"
#include "Physics.h"

struct Rail : QuadTreeEntrant
{
	Rail(GameSession *owner, sf::Vector2i &pos, 
		std::list<sf::Vector2i> &path, bool energized );
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);

	Rail *drawNext;

	Edge ** edges;
	int numEdges;
	sf::Vertex *va;
	bool energized;
	bool playerAttached;

	Tileset *ts_rail;
	void UpdateSprite();
	void Draw(sf::RenderTarget *target);
	GameSession *owner;

	sf::Rect<double> aabb;

};

#endif