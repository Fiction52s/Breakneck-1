#ifndef __MOVER_H__
#define __MOVER_H__

#include "Physics.h"
//#include "GameSession.h"

//struct SurfaceMoverHandler
struct GroundMoverHandler
{
	//virtual void HitGround() = 0;
	virtual void HitOther() = 0;
	virtual void ReachCliff() = 0;
	virtual void HitOtherAerial( Edge *e ) = 0;
	virtual void Land() = 0;

//	virtual void Land(){};
};

struct SurfaceMoverHandler
{
	virtual void HitTerrainAerial(Edge *, double) {};
	virtual void TransferEdge(Edge *) {};
};

struct GameSession;
//circle for now
struct SurfaceMover : QuadTreeCollider
{
	//some virtual functions

	SurfaceMover( GameSession *owner,
		Edge *startGround,
		double startQuantity,
		double radius);
	void HandleEntrant( QuadTreeEntrant *qte );
	void Update();
	void SetSpeed( double speed );
	void Move( int slowMultiple, int numPhysSteps );
	bool RollClockwise( double &q,
		double &m );
	bool RollCounterClockwise( double &q,
		double &m );
	void UpdateGroundPos();
	bool ResolvePhysics( 
		sf::Vector2<double> &vel );
	bool MoveAlongEdge( double &movement, 
		double &groundLength, double &q,
		double &m
		);

	virtual void HitTerrain( double &q );
	virtual void HitTerrainAerial();
	virtual bool StartRoll();
	virtual void FinishedRoll();
	void Jump( sf::Vector2<double> &vel );

	Edge *ground;
	GameSession *owner;
	double edgeQuantity;
	bool roll;
	CollisionBox physBody;
	double groundSpeed;
	Contact minContact;
	bool col;
	//std::string queryMode;
	sf::Vector2<double> tempVel;

	//sf::Vector2<double> gravity;
	//double maxSpeed;
	sf::Vector2<double> velocity;

	int framesInAir;

	SurfaceMoverHandler *surfaceHandler;
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
		bool steeps,
		GroundMoverHandler *handler);
	bool steeps;
	void HitTerrain( double &q );
	
	GroundMoverHandler *handler;
	//bool startRoll;
	//bool finishedRoll;

	
	bool StartRoll();
	void FinishedRoll();
	void HitTerrainAerial();



	//sf::Vector2<double> velocity;


};

#endif