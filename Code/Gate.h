#ifndef __GATE_H__
#define __GATE_H__

#include "Physics.h"
#include "Tileset.h"

struct GameSession;
struct Zone;

struct Gate : public QuadTreeEntrant//: public Edge
{
	
	enum GateType
	{
		BLACK,
		KEYGATE,
		BIRDFIGHT,
		CRAWLER_UNLOCK,
		NEXUS1_UNLOCK,
		Count
	};

	enum GateState
	{
		HARDEN,
		HARD,
		SOFTEN,
		SOFT,
		DISSOLVE,
		REFORM,
		LOCKFOREVER,
		OPEN,
		State_Count
	};
	
	//bool keyGate;
	//int requiredKeys;
	bool reformBehindYou;
	Gate( GameSession *owner, GateType type,
		bool reformBehindYou );
	~Gate();
	GateType type;
	GameSession *owner;
	GateState gState;
	bool locked;
	int frame;
	sf::Color c;
	sf::Vertex testLine[4];
	sf::Vertex centerLine[4];
	sf::VertexArray thickLine;
	sf::VertexArray *gQuads;
	Tileset *ts;
	void UpdateLine();
	void SetLocked( bool on );
	void Update();

	sf::Shader gateShader;
	sf::Shader centerShader;

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );

	void Draw( sf::RenderTarget *target );
	void CalcAABB();

	sf::Rect<double> aabb;

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

	
};

#endif