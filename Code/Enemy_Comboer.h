#ifndef __ENEMY_Comboer_H__
#define __ENEMY_Comboer_H__

#include "Enemy.h"

struct ComboObject;

struct Comboer : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_SHOT,
		S_EXPLODE,
		S_RETURN,
		S_Count
	};

	struct MyData : StoredEnemyData
	{
		int currHits;
		int shootFrames;
	};

	MyData data;


	V2d velocity;
	
	int shootLimit;
	int hitLimit;
	

	BasicPathFollower pathFollower;

	double acceleration;
	double speed;

	Tileset *ts;

	Comboer(ActorParams * ap);
	~Comboer();

	bool CountsForEnemyGate() { return false; }
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);
	void Return();
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif