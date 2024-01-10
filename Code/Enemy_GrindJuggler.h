#ifndef __ENEMY_GRINDJUGGLER_H__
#define __ENEMY_GRINDJUGGLER_H__

#include "Enemy.h"

struct ComboObject;
struct Rail;

struct GrindJuggler : Enemy, SurfaceRailMoverHandler
{
	enum Action
	{
		S_FLOAT,
		S_FLY,
		S_RAILGRIND,
		S_GRIND,
		S_RETURN,
		//S_FLY_FROM_GRIND,
		//S_HITFLY_FROM_GRIND,
		S_Count
	};


	struct MyData : StoredEnemyData
	{
		int currHits;
		V2d velocity;
		float currAngle;
		int moveFrame;
	};

	MyData data;

	SoundInfo *myHitSoundCW;
	SoundInfo *myHitSoundCCW;

	float idleTurnDegrees;
	float moveTurnDegrees;

	
	Edge *railEdge;
	double railQuant;
	double railSpeed;

	int hitLimit;

	Tileset *ts;

	double flySpeed;

	int maxMoveFrames;
	
	//int maxWaitFrames;

	bool clockwise;

	bool CountsForEnemyGate() { return false; }
	GrindJuggler(ActorParams *ap);
	~GrindJuggler();
	void SetLevel(int lev);
	void UpdateParamsSettings();
	void HitTerrainAerial(Edge * edge, double quant);
	void HandleEntrant(QuadTreeEntrant *qte);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void ComboKill(Enemy *e);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void Move();
	void Return();
	void Pop();
	void PopThrow();
	void ExtraQueries(sf::Rect<double> &r);

	void LeaveRail();
	void BoardRail();
	bool CanLeaveRail();

	void Throw(double a, double strength);
	void Throw(V2d vel);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif