#ifndef __ENEMY_BOUNCEFLOATER_H__
#define __ENEMY_BOUNCEFLOATER_H__

#include "Enemy.h"

struct ComboObject;

struct BounceFloater : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_BOUNCE,
		S_RECOVER,
		S_Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts;

	bool CountsForEnemyGate() { return false; }
	BounceFloater(ActorParams *ap);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void SetLevel(int lev);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif