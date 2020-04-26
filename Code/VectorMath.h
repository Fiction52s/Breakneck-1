#ifndef __VECTOR_MATH_H__
#define __VECTOR_MATH_H__

#include <SFML/Graphics.hpp>

#define V2d sf::Vector2<double>
double cross( sf::Vector2<double> a, sf::Vector2<double> b );
float cross(sf::Vector2<float> &a, sf::Vector2<float> &b);

#define PI 3.14159265359
double length( sf::Vector2<double> v);
float length(sf::Vector2f v);
double lengthSqr( sf::Vector2<double> &v);
float lengthSqr(sf::Vector2<float> &v);

sf::Vector2<double> normalize( sf::Vector2<double> v );
sf::Vector2f normalize(sf::Vector2f v);

double dot( sf::Vector2<double> a, sf::Vector2<double> b );
float dot(sf::Vector2<float> &a, sf::Vector2<float> &b);

double GetVectorAngleCW(sf::Vector2<double> &a);
double GetVectorAngleCCW(sf::Vector2<double> &a);
double GetVectorAngleDiffCW(sf::Vector2<double> &a, sf::Vector2<double> &b);
double GetVectorAngleDiffCCW(sf::Vector2<double> &a, sf::Vector2<double> &b);

float GetVectorAngleCW(sf::Vector2f &a);
float GetVectorAngleCCW(sf::Vector2f &a);
float GetVectorAngleDiffCW(sf::Vector2f &a, sf::Vector2f &b);
float GetVectorAngleDiffCCW(sf::Vector2f &a, sf::Vector2f &b);

void RotateCCW(sf::Vector2<double> &a, double angle);
void RotateCW(sf::Vector2<double> &a, double angle);
void RotateCCW(sf::Vector2f &a, float angle);
void RotateCW(sf::Vector2f &a, float angle);

V2d Get8Dir( V2d &dir );

struct LineIntersection
{
	LineIntersection()
		:parallel(true) {}
	LineIntersection(const sf::Vector2<double> &pos, bool p );
	sf::Vector2<double> position;
	bool parallel;
};

bool approxEquals( double a, double b );

LineIntersection lineIntersection( sf::Vector2<double> a, sf::Vector2<double> b, sf::Vector2<double> c, sf::Vector2<double> d );

LineIntersection SegmentIntersect( sf::Vector2<double> a, 
	sf::Vector2<double> b, sf::Vector2<double> c, 
	sf::Vector2<double> d );

void SetRectCenter(sf::Vertex *v, float width, float height,
	sf::Vector2f &origin);
void SetRectTopLeft(sf::Vertex *v, float width, float height,
	sf::Vector2f &origin);
void SetRectColor(sf::Vertex *v, sf::Color &c);
void SetRectSubRect(sf::Vertex *v, sf::IntRect &sub);
void SetRectSubRectGL(sf::Vertex *v, sf::IntRect &sub,
	sf::Vector2f texSize );
void SetRectSubRect(sf::Vertex *v, sf::FloatRect &sub);
void SetRectRotation(sf::Vertex *v,
	float angleRad, int width, int height,
	sf::Vector2f &origin,
	bool flipHoriz = false,
	bool flipVert = false );
void TransformRect(sf::Vertex *v, sf::Transform &tr);
void ClearRect( sf::Vertex *v );

double GetRandValue(); //-1 to 1

#endif