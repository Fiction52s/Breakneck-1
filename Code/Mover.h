#ifndef __MOVER_H__
#define __MOVER_H__

#include "Physics.h"
#include "Movement.h"
#include "PositionInfo.h"
//#include "GameSession.h"

struct Session;
struct Rail;

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
	virtual void ExtraQueries(sf::Rect<double> &r) {}
};


struct SurfaceRailMoverHandler : SurfaceMoverHandler
{
	virtual void HitTerrainAerial(Edge *, double) {};
	virtual void TransferEdge(Edge *) {};
	virtual void ExtraQueries(sf::Rect<double> &r) {}
	virtual void LeaveRail() {}
	virtual void BoardRail() {}
	virtual bool CanLeaveRail() {return false;}
};


//circle for now
struct SurfaceMover : QuadTreeCollider
{
	//some virtual functions

	SurfaceMover(Edge *startGround,
		double startQuantity,
		double radius);
	void Set(PositionInfo &pi);
	virtual void HandleEntrant( QuadTreeEntrant *qte );
	void AddAirForce(V2d &force);
	void ClearAirForces();
	void SetSpeed( double speed );
	virtual void Move( int slowMultiple, int numPhysSteps );
	bool RollClockwise( double &q,
		double &m );
	bool RollCounterClockwise( double &q,
		double &m );
	void UpdateGroundPos();

	virtual bool ResolvePhysics( 
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
	Session *sess;
	double edgeQuantity;
	bool roll;
	CollisionBox physBody;
	double groundSpeed;
	Contact minContact;
	bool col;
	V2d force;
	//std::string queryMode;
	sf::Vector2<double> tempVel;
	bool collisionOn;
	double steal;

	//sf::Vector2<double> gravity;
	//double maxSpeed;
	sf::Vector2<double> velocity;

	int framesInAir;

	void SetHandler(SurfaceMoverHandler *h);

	SurfaceMoverHandler *surfaceHandler;
	//move clockwise or counterclockwise
	//and receive callbacks for stuff happening
	//
};

struct SurfaceRailMover : SurfaceMover
{
	SurfaceRailMover(Edge *startGround,
		double startQuantity,
		double radius);
	bool ResolvePhysics(
		V2d &vel);
	void Move(int slowMultiple, int numPhysSteps);
	void HandleEntrant(QuadTreeEntrant *qte);
	void SetRailSpeed(double s);

	std::string queryMode;

	Rail *currRail;

	bool railCollisionOn;
	double tempQuant;

	void SetHandler(SurfaceRailMoverHandler *h);

	SurfaceRailMoverHandler *surfaceRailHandler;

};

struct GroundMover : SurfaceMover
{
	GroundMover(Edge *startGround,
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