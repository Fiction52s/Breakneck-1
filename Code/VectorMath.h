#ifndef __VECTOR_MATH_H__
#define __VECTOR_MATH_H__

#include <SFML/Graphics.hpp>

float cross( sf::Vector2f a, sf::Vector2f b );

#define PI 3.14159265359
float length( sf::Vector2f v);

sf::Vector2f normalize( sf::Vector2f v );

float dot( sf::Vector2f a, sf::Vector2f b );

struct LineIntersection
{
	LineIntersection(const sf::Vector2f &pos, bool p );
	sf::Vector2f position;
	bool parallel;
};

bool approxEquals( float a, float b );

LineIntersection lineIntersection( sf::Vector2f a, sf::Vector2f b, sf::Vector2f c, sf::Vector2f d );

LineIntersection SegmentIntersect( sf::Vector2f a, 
	sf::Vector2f b, sf::Vector2f c, 
	sf::Vector2f d );


#endif