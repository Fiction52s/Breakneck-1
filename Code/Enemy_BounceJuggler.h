#ifndef __ENEMY_BOUNCEJUGGLER_H__
#define __ENEMY_BOUNCEJUGGLER_H__

#include "Enemy.h"

struct ComboObject;

struct BounceJuggler : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		S_FLOAT,
		S_FLY,
		S_BOUNCE,
		S_RETURN,
		S_Count
	};

	struct MyData : StoredEnemyData
	{
		int flyFrame;
		//int currHits;
		//int juggleTextNumber;
		//int currJuggle;
		//int waitFrame;
		//bool doneBeingHittable;
	};
	MyData data;

	double flySpeed;

	int hitLimit;

	Tileset *ts;

	int juggleReps;

	int maxFlyFrames;

	bool CountsForEnemyGate() { return false; }
	BounceJuggler(ActorParams *ap);
	~BounceJuggler();
	void SetLevel(int lev);
	void HitTerrainAerial(Edge * edge, double quant);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void ComboKill(Enemy *e);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void Return();
	void Pop();
	void PopThrow();
	void DirectKill();
	bool CanComboHit(Enemy *e);

	void Throw(double a, double strength);
	void Throw(V2d vel);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif