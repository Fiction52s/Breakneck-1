#ifndef __MOVER_H__
#define __MOVER_H__

#include "Physics.h"
#include "Movement.h"
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
	void AddAirForce(V2d &force);
	void ClearAirForces();
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
	V2d force;
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


struct CubicCurve
{
	CubicCurve();
	CubicCurve(V2d &A,
		V2d &B,
		V2d &C,
		V2d &D,
		CubicBezier &bez );

	sf::Vector2<double> A;
	sf::Vector2<double> B;
	sf::Vector2<double> C;
	sf::Vector2<double> D;

	sf::Vector2<double> GetPosition(double t);
};

struct QuadraticCurve
{
	QuadraticCurve();
	QuadraticCurve(V2d &A,
		V2d &B, V2d &C);
	sf::Vector2<double> A;
	sf::Vector2<double> B;
	sf::Vector2<double> C;
	sf::Vector2<double> GetPosition(double t);
};

struct SpaceMover
{
	enum SpaceMovementState
	{
		S_WAIT,
		S_HOVER,
		S_STRAIGHTMOVE,
		S_HOVERMOVE,
		S_CURVEDMOVE,
	};

	//CubicCurve curve;
	QuadraticCurve curve;

	SpaceMovementState state;

	SpaceMover();
	void Reset();
	void SetHover(double dipPixels, int loopFrames);
	void SetMove(V2d &target, double maxSpeed,
		double accel, int decelFrames, double decelRad);
	void SetCurvedMoveContinue(V2d target, 
		V2d targetVel);
	void SetCurvedMove( V2d startVelocity, V2d target,
		V2d targetVel);
	void StateEnded();
	bool Update();
	void SetState(SpaceMovementState newState);
	void ApplyHover();
	void ApplyLinearMove();
	void DebugDraw(sf::RenderTarget *target);
	bool IsIdle();

	V2d position;
	V2d targetPos;
	V2d startPos;

	V2d velocity;
	V2d targetVelocity;

	double accel;
	double maxVel;
	double decelRadius;
	int decelFrames;
	double currentHoverOffset;

	double hoverDipPixels;

	double currentT;
	double curvedSpeed;

	int frame;
	int currStateLength;

	bool stateOver;
};

#endif