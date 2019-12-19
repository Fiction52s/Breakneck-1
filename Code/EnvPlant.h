#ifndef __ENVPLANT_H__
#define __ENVPLANT_H__

#include "QuadTree.h"
#include <SFML\Graphics.hpp>
#include "Physics.h"

struct Tileset;
struct AirParticleEffect;

struct EnvPlant : QuadTreeEntrant
{
	EnvPlant(V2d&a,
		V2d&b,
		V2d&c,
		V2d&d,
		int vi, sf::VertexArray *v,
		Tileset *ts);
	~EnvPlant();
	void Reset();
	void SetupQuad();

	//Tileset *ts;
	int vaIndex;
	Tileset *ts;
	AirParticleEffect *particle;
	sf::VertexArray *va;
	sf::Vector2<double> A;
	sf::Vector2<double> B;
	sf::Vector2<double> C;
	sf::Vector2<double> D;
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	EnvPlant *next;
	//EnvPlant *prev;
	bool activated;
	int frame;
	int idleLength;
	int idleFactor;
	int disperseLength;
	int disperseFactor;
	//EnvPlant *prev;
};

#endif