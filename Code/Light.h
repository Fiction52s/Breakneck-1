#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <SFML/Graphics.hpp>
#include "QuadTree.h"

struct GameSession;
struct Light : QuadTreeEntrant
{
	Light( GameSession *owner, sf::Vector2i &p, sf::Color &c, double radius, double brightness );
	Light * next;
	sf::Shader sh;
	sf::CircleShape cs;
	GameSession *owner;
	//sf::Vector3f falloff;
	float depth;
	float radius;
	float brightness;
	void Draw( sf::RenderTarget *target );

	sf::Vector2i pos;
	sf::Color color;

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );
};

#endif