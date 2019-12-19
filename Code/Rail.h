#ifndef __RAIL_H__
#define __RAIL_H__

#include <list>
#include "Mover.h"
#include "Movement.h"
#include "Physics.h"

struct Tileset;

struct Rail : QuadTreeEntrant
{
	Rail(GameSession *owner, sf::Vector2i &pos, 
		std::list<sf::Vector2i> &path, bool p_requirePower,
		bool accelerate, int level );
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);

	Rail *drawNext;

	Edge ** edges;
	int numEdges;
	sf::Vertex *va;
	bool playerAttached;

	Tileset *ts_rail;
	void UpdateSprite();
	void Draw(sf::RenderTarget *target);
	GameSession *owner;

	sf::Rect<double> aabb;

	int level;

	bool requirePower;
	bool accelerate;

};

#endif