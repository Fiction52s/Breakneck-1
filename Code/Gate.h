#ifndef __GATE_H__
#define __GATE_H__

#include "Physics.h"

struct Zone;

struct Gate : public QuadTreeEntrant//: public Edge
{
	
	enum GateType
	{
		RED,
		GREEN,
		BLUE,
		CRITICAL,
		Count		
	};
	Gate( GateType type );
	GateType type;
	bool locked;
	sf::Color c;
	sf::VertexArray thickLine;
	void UpdateLine();
	void SetLocked( bool on );

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );


	Edge *temp0prev;
	Edge *temp0next;
	Edge *temp1prev;
	Edge *temp1next;

	Edge *edgeA;
	Edge *edgeB;

	Zone *zoneA;//edgeA is part of zoneA
	Zone *zoneB;

	Gate *next;
	Gate *prev;

	Gate *activeNext;
	//

	void Draw( sf::RenderTarget *target );
};

#endif