#include "VectorMath.h"
#include <iostream>

using namespace std;
using namespace sf;




double length( sf::Vector2<double> v)
{
	double vx = v.x;
	double vy = v.y;
	return sqrt( vx * vx + vy * vy );
}

float length(sf::Vector2f v)
{
	float vx = v.x;
	float vy = v.y;
	return sqrt(vx * vx + vy * vy);
}

double lengthSqr( sf::Vector2<double> v )
{
	double vx = v.x;
	double vy = v.y;
	return vx * vx + vy * vy;
}

sf::Vector2<double> normalize( sf::Vector2<double> v )
{
	double vLen = length( v );
	if( vLen > 0 )
		return sf::Vector2<double>( v.x / vLen, v.y / vLen );
	else
		return sf::Vector2<double>( 0, 0 );
}

sf::Vector2f normalize(sf::Vector2f v)
{
	float vLen = length(v);
	if (vLen > 0)
		return sf::Vector2f(v.x / vLen, v.y / vLen);
	else
		return sf::Vector2f(0, 0);
}

double dot( sf::Vector2<double> a, sf::Vector2<double> b )
{
	double ax = a.x;
	double ay = a.y;
	double bx = b.x;
	double by = b.y;
	return ax * bx + ay * by;
}

double cross( sf::Vector2<double> a, sf::Vector2<double> b )
{
	double ax = a.x;
	double ay = a.y;
	double bx = b.x;
	double by = b.y;
	return ax * by - ay * bx;
	//return a.x * b.y - a.y * b.x;
}

bool approxEquals( double a, double b )
{
	//if i get any weird physics bugs in the future its probably this acting up -_-

	//before was .00001. testing for rounding errors
	return abs( a - b ) < .0001;
}

double GetVectorAngleCW(sf::Vector2<double> &a)
{
	double angleA = atan2(a.y, a.x);
	if (angleA < 0)
	{
		angleA += PI * 2;
	}
	return angleA;
}

double GetVectorAngleCCW(sf::Vector2<double> &a)
{
	double angleA = atan2(-a.y, a.x);
	if (angleA < 0)
	{
		angleA += PI * 2;
	}
	return angleA;
}

double GetVectorAngleDiffCW(sf::Vector2<double> &a, sf::Vector2<double> &b)
{
	double angleA = GetVectorAngleCW(a);
	double angleB = GetVectorAngleCW(b);
	if (angleB > angleA)
	{
		return angleB - angleA;
	}
	else if (angleB < angleA)
	{
		return ((2 * PI) - angleA) + angleB;
	}
	else
	{
		return 2 * PI;
	}
}

double GetVectorAngleDiffCCW(sf::Vector2<double> &a, sf::Vector2<double> &b)
{
	double angleA = GetVectorAngleCCW(a);
	double angleB = GetVectorAngleCCW(b);
	if (angleB > angleA)
	{
		return angleB - angleA;
	}
	else if (angleB < angleA)
	{
		return ((2 * PI) - angleA) + angleB;
	}
	else
	{
		return 0;
	}
	
}

float GetVectorAngleCW(sf::Vector2f &a)
{
	float angleA = atan2(a.y, a.x);
	if (angleA < 0)
	{
		angleA += PI * 2;
	}
	return angleA;
}

float GetVectorAngleCCW(sf::Vector2f &a)
{
	float angleA = atan2(-a.y, a.x);
	if (angleA < 0)
	{
		angleA += PI * 2;
	}
	return angleA;
}

float GetVectorAngleDiffCW(sf::Vector2f &a, sf::Vector2f &b)
{
	float angleA = GetVectorAngleCW(a);
	float angleB = GetVectorAngleCW(b);
	if (angleB > angleA)
	{
		return angleB - angleA;
	}
	else if (angleB < angleA)
	{
		return ((2 * PI) - angleA) + angleB;
	}
	else
	{
		return 2 * PI;
	}
}

float GetVectorAngleDiffCCW(sf::Vector2f &a, sf::Vector2f &b)
{
	float angleA = GetVectorAngleCCW(a);
	float angleB = GetVectorAngleCCW(b);
	if (angleB > angleA)
	{
		return angleB - angleA;
	}
	else if (angleB < angleA)
	{
		return ((2 * PI) - angleA) + angleB;
	}
	else
	{
		return 0;
	}

}

void RotateCW(sf::Vector2<double> &a, double angle )
{
	//RotateCCW(a, -angle;c)
	double angleA = GetVectorAngleCCW(a);//GetVectorAngleCW(a);
	double ca = cos(-angle);
	double sa = sin(-angle);
	double ax = a.x;
	double ay = a.y;
	a.x = ca * ax + sa * ay;
	a.y = -sa * ax + ca * ay;
}

void RotateCCW(sf::Vector2<double> &a, double angle)
{
	double angleA = GetVectorAngleCCW(a);
	double ca = cos(-angle);
	double sa = sin(-angle);
	double ax = a.x;
	double ay = a.y;
	a.x = ca * ax - sa * ay;
	a.y = sa * ax + ca * ay;
}

void RotateCW(sf::Vector2f &a, float angle)
{
	//RotateCCW(a, -angle;c)
	float angleA = GetVectorAngleCCW(a);//GetVectorAngleCW(a);
	double ca = cos(-angle);
	double sa = sin(-angle);
	double ax = a.x;
	double ay = a.y;
	a.x = ca * ax + sa * ay;
	a.y = -sa * ax + ca * ay;
}

void RotateCCW(sf::Vector2f &a, float angle)
{
	float angleA = GetVectorAngleCCW(a);
	double ca = cos(-angle);
	double sa = sin(-angle);
	double ax = a.x;
	double ay = a.y;
	a.x = ca * ax - sa * ay;
	a.y = sa * ax + ca * ay;
}

LineIntersection::LineIntersection(const sf::Vector2<double> &pos, bool p )
{
	position = pos;
	parallel = p;
}


LineIntersection lineIntersection( sf::Vector2<double> a, Vector2<double> b, Vector2<double> c, Vector2<double> d )
{
	double ax = a.x;
	double ay = a.y;
	double bx = b.x;
	double by = b.y;
	double cx = c.x;
	double cy = c.y;
	double dx = d.x;
	double dy = d.y;

	double x= 0,y = 0;
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

	return LineIntersection( sf::Vector2<double>(x,y), parallel );
}

LineIntersection SegmentIntersect( sf::Vector2<double> a, 
	sf::Vector2<double> b, sf::Vector2<double> c, 
	sf::Vector2<double> d )
{
	LineIntersection li = lineIntersection( sf::Vector2<double>( a.x, a.y ), sf::Vector2<double>( b.x, b.y ), 
				sf::Vector2<double>( c.x, c.y ), sf::Vector2<double>( d.x, d.y ) );
	if( !li.parallel )
	{
		double e1Left = min( a.x, b.x );
		double e1Right = max( a.x, b.x );
		double e1Top = min( a.y, b.y );
		double e1Bottom = max( a.y, b.y );

		double e2Left = min( c.x, d.x );
		double e2Right = max( c.x, d.x );
		double e2Top = min( c.y, d.y );
		double e2Bottom = max( c.y, d.y );
		//cout << "e1left: " << e1Left << ", e1Right: " << e1Right << ", e1Top: " << e1Top << ", e1Bottom: " << e1Bottom << endl;
	//	cout << "e2left: " << e2Left << ", e2Right: " << e2Right << ", e2Top: " << e2Top << ", e2Bottom: " << e2Bottom << endl;
	//	cout << "li: " << li.position.x << ", " << li.position.y << endl;

		//cout << "compares: " << e1Left << ", " << e2Right << " .. " << e1Right << ", " << e2Left << endl;
		//cout << "compares y: " << e1Top << " <= " << e2Bottom << " && " << e1Bottom << " >= " << e2Top << endl;
		if( e1Left <= e2Right && e1Right >= e2Left && e1Top <= e2Bottom && e1Bottom >= e2Top )
		{
			//cout << "---!!!!!!" << endl;
			if( li.position.x <= e1Right + .0001 && li.position.x >= e1Left - .0001 && li.position.y >= e1Top - .0001 && li.position.y <= e1Bottom + .0001)
			{
				if( li.position.x <= e2Right + .0001 && li.position.x >= e2Left - .0001 && li.position.y >= e2Top - .0001 && li.position.y <= e2Bottom + .0001)
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

void SetRectCenter(sf::Vertex *v, int width, int height,
	sf::Vector2f &origin)
{
	v[0].position = origin + Vector2f(-width / 2, -height / 2);
	v[1].position = origin + Vector2f(width / 2, -height / 2);
	v[2].position = origin + Vector2f(width / 2, height / 2);
	v[3].position = origin + Vector2f(-width / 2, height / 2);
}

void SetRectColor(sf::Vertex *v, sf::Color &c)
{
	v[0].color = c;
	v[1].color = c;
	v[2].color = c;
	v[3].color = c;
}

void SetRectSubRect(sf::Vertex *v, sf::IntRect &sub)
{
	v[0].texCoords = Vector2f(sub.left, sub.top);
	v[1].texCoords = Vector2f(sub.left + sub.width, sub.top);
	v[2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);
	v[3].texCoords = Vector2f(sub.left, sub.top + sub.height);
}

void SetRectSubRect(sf::Vertex *v, sf::FloatRect &sub)
{
	v[0].texCoords = Vector2f(sub.left, sub.top);
	v[1].texCoords = Vector2f(sub.left + sub.width, sub.top);
	v[2].texCoords = Vector2f(sub.left + sub.width, sub.top + sub.height);
	v[3].texCoords = Vector2f(sub.left, sub.top + sub.height);
}