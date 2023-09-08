#ifndef __ENEMY_GRAVITYFALLER_H__
#define __ENEMY_GRAVITYFALLER_H__

#include "Enemy.h"

struct GravityFaller : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		IDLE,
		DOWNCHARGE,
		FALLDOWN,
		UPCHARGE,
		FALLUP,
		REVERSEUPTODOWN,
		REVERSEDOWNTOUP,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int chargeFrame;
		int fallFrames;
	};
	MyData data;

	
	
	

	
	double gravity;
	double gravityFactor;
	int chargeLength;
	

	double maxFallSpeed;
	V2d startNormal;
	Tileset *ts;

	GravityFaller(ActorParams *ap);
	void SetLevel(int lev);
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ProcessState();
	void ActionEnded();
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();

	void HitTerrainAerial(Edge *e, double quant);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif