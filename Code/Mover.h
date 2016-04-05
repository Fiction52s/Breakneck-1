#ifndef __MOVER_H__
#define __MOVER_H__

#include "Physics.h"
#include "GameSession.h"

//circle for now
struct SurfaceMover : QuadTreeCollider
{
	//some virtual functions

	SurfaceMover( GameSession *owner,
		Edge *startGround,
		double startQuantity,
		double radius );

	void HandleEntrant( QuadTreeEntrant *qte );
	void Update();
	void SetSpeed( double speed );
	void Move( int slowMultiple );

	bool RollClockwise( double &q,
		double &m );
	bool RollCounterClockwise( double &q,
		double &m );

	Edge *ground;
	GameSession *owner;
	double edgeQuantity;
	bool roll;
	CollisionBox physBody;
	void UpdateGroundPos();
	bool ResolvePhysics( 
		sf::Vector2<double> &vel );
	bool MoveAlongEdge( double &movement, 
		double &groundLength, double &q,
		double &m
		);

	virtual void HitTerrain( double &q );
	virtual bool StartRoll();
	virtual void FinishedRoll();
	
	double groundSpeed;

	Contact minContact;
	bool col;
	std::string queryMode;
	sf::Vector2<double> tempVel;
	//move clockwise or counterclockwise
	//and receive callbacks for stuff happening
	//
};

struct GroundMover : SurfaceMover
{
	GroundMover( GameSession *owner,
		Edge *startGround,
		double startQuantity,
		double radius,
		bool steeps );
	bool steeps;
	virtual void HitTerrain( double &q );
	virtual bool StartRoll();
	virtual void FinishedRoll();
};

#endif