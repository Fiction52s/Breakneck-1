#ifndef __MOVER_H__
#define __MOVER_H__

#include "Physics.h"
#include "Movement.h"
#include "PositionInfo.h"
//#include "GameSession.h"

struct Session;
struct TerrainRail;

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
	struct SurfaceMoverData
	{
		EdgeInfo groundInfo;
		double edgeQuantity;
		CollisionBox physBody;
		double groundSpeed;
		V2d force;
		int framesInAir;
		V2d velocity;
		bool collisionOn;
		bool roll;
	};

	SurfaceMoverData surfaceMoverData;

	Session *sess;
	Edge *ground;
	Contact minContact;
	bool col;
	V2d tempVel;
	double steal;
	SurfaceMoverHandler *surfaceHandler;

	//some virtual functions
	SurfaceMover();
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
	double GetAngleRadians();
	double GetAngleDegrees();
	V2d GetGroundPoint();
	const V2d &GetPosition();
	sf::Vector2f GetGroundPointF();
	void DebugDraw(sf::RenderTarget *target);
	virtual bool ResolvePhysics( 
		V2d &vel );
	void SetVelocity(const V2d &vel);
	bool MoveAlongEdge( double &movement, 
		double &groundLength, double &q,
		double &m
		);
	virtual void HitTerrain( double &q );
	virtual void HitTerrainAerial();
	virtual bool StartRoll();
	virtual void FinishedRoll();
	void Jump(V2d &vel );
	void SetHandler(SurfaceMoverHandler *h);
	void PopulateFromData(const SurfaceMoverData &sfm);
	void PopulateData(SurfaceMoverData &sfm);
	const V2d &GetVel();
	void SetVelX(double x);
	void SetVelY (double y);
	double GetGroundSpeed();
	double GetEdgeQuantity();
	void SetCollisionOn(bool on);

	virtual int GetNumStoredBytes();
	virtual void StoreBytes(unsigned char *bytes);
	virtual void SetFromBytes(unsigned char *bytes);
};

struct SurfaceRailMover : SurfaceMover
{
	struct RailMoverData
	{
		int currRailID;
		bool railCollisionOn;
	};

	RailMoverData railMoverData;

	std::string queryMode;
	TerrainRail *currRail;
	bool railCollisionOn;
	double tempQuant;
	SurfaceRailMoverHandler *surfaceRailHandler;

	SurfaceRailMover(Edge *startGround,
		double startQuantity,
		double radius);
	bool ResolvePhysics(
		V2d &vel);
	void Move(int slowMultiple, int numPhysSteps);
	void HandleEntrant(QuadTreeEntrant *qte);
	void SetRailSpeed(double s);
	void SetHandler(SurfaceRailMoverHandler *h);
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

struct GroundMover : SurfaceMover
{
	struct GroundMoverData
	{
		bool reverse;
		bool steeps;
	};

	GroundMoverData groundMoverData;
	GroundMoverHandler *handler;

	GroundMover();
	GroundMover(Edge *startGround,
		double startQuantity,
		double radius,
		bool steeps,
		GroundMoverHandler *handler);
	
	void HitTerrain( double &q );
	bool IsEdgeViableGround( V2d &en );
	bool StartRoll();
	void FinishedRoll();
	void HitTerrainAerial();
	void SetReverse(bool r);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	void PopulateFromData(const SurfaceMoverData &sfm, const GroundMoverData &gmd);
	void PopulateData(SurfaceMoverData &sfm, GroundMoverData &gmd );
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