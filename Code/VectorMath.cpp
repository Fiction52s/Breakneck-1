#include "VectorMath.h"
#include <iostream>

using namespace std;
using namespace sf;

float cross( sf::Vector2f a, sf::Vector2f b )
{
	float ax = a.x;
	float ay = a.y;
	float bx = b.x;
	float by = b.y;
	return ax * by - ay * bx;
	//return a.x * b.y - a.y * b.x;
}


float length( sf::Vector2f v)
{
	float vx = v.x;
	float vy = v.y;
	return sqrt( vx * vx + vy * vy );
}

sf::Vector2f normalize( sf::Vector2f v )
{
	float vLen = length( v );
	if( vLen > 0 )
		return sf::Vector2f( v.x / vLen, v.y / vLen );
	else
		return sf::Vector2f( 0, 0 );
}

float dot( sf::Vector2f a, sf::Vector2f b )
{
	float ax = a.x;
	float ay = a.y;
	float bx = b.x;
	float by = b.y;
	return ax * bx + ay * by;
}

bool approxEquals( float a, float b )
{
	//before was .f0001. testing for rounding errors
	return abs( a - b ) < .000001f;
}


LineIntersection::LineIntersection(const sf::Vector2f &pos, bool p )
{
	position = pos;
	parallel = p;
}


LineIntersection lineIntersection( sf::Vector2f a, Vector2<float> b, Vector2<float> c, Vector2<float> d )
{
	float ax = a.x;
	float ay = a.y;
	float bx = b.x;
	float by = b.y;
	float cx = c.x;
	float cy = c.y;
	float dx = d.x;
	float dy = d.y;

	float x= 0,y = 0;
	bool parallel = false;
	if( (ax-bx)*(cy - dy) - (ay - by) * (cx - dx ) == 0 )
	{
		parallel = true;
	}
	else
	{
		x = ((ax * by - ay * bx ) * ( cx - dx ) - (ax - bx ) * ( cx * dy - cy * dx ))
			/ ( (ax-bx)*(cy - dy) - (ay - by) * (cx - dx ) );
		y = ((ax * by - ay * bx ) * ( cy - dy ) - (ay - by ) * ( cx * dy - cy * dx ))
			/ ( (ax-bx)*(cy - dy) - (ay - by) * (cx - dx ) );
	}

	return LineIntersection( sf::Vector2f(x,y), parallel );
}

LineIntersection SegmentIntersect( sf::Vector2f a, 
	sf::Vector2f b, sf::Vector2f c, 
	sf::Vector2f d )
{
	LineIntersection li = lineIntersection( sf::Vector2f( a.x, a.y ), sf::Vector2f( b.x, b.y ), 
				sf::Vector2f( c.x, c.y ), sf::Vector2f( d.x, d.y ) );
	if( !li.parallel )
	{
		float e1Left = min( a.x, b.x );
		float e1Right = max( a.x, b.x );
		float e1Top = min( a.y, b.y );
		float e1Bottom = max( a.y, b.y );

		float e2Left = min( c.x, d.x );
		float e2Right = max( c.x, d.x );
		float e2Top = min( c.y, d.y );
		float e2Bottom = max( c.y, d.y );
		//cout << "e1left: " << e1Left << ", e1Right: " << e1Right << ", e1Top: " << e1Top << ", e1Bottom: " << e1Bottom << endl;
	//	cout << "e2left: " << e2Left << ", e2Right: " << e2Right << ", e2Top: " << e2Top << ", e2Bottom: " << e2Bottom << endl;
	//	cout << "li: " << li.position.x << ", " << li.position.y << endl;

		//cout << "compares: " << e1Left << ", " << e2Right << " .. " << e1Right << ", " << e2Left << endl;
		//cout << "compares y: " << e1Top << " <= " << e2Bottom << " && " << e1Bottom << " >= " << e2Top << endl;
		if( e1Left <= e2Right && e1Right >= e2Left && e1Top <= e2Bottom && e1Bottom >= e2Top )
		{
			//cout << "---!!!!!!" << endl;
			if( li.position.x <= e1Right + .0001f && li.position.x >= e1Left - .0001f && li.position.y >= e1Top - .0001f && li.position.y <= e1Bottom + .0001f)
			{
				if( li.position.x <= e2Right + .0001f && li.position.x >= e2Left - .0001f && li.position.y >= e2Top - .0001f && li.position.y <= e2Bottom + .0001f)
				{
					//cout << "seg intersect!!!!!!" << endl;
					//assert( 0 );
					return li;
				}
			}
		}
	}

	//cout << "return false" << endl;
	li.parallel = true;
	return li;
}