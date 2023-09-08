#ifndef __ENEMY_SWORDPROJECTILE_H__
#define __ENEMY_SWORDPROJECTILE_H__

#include "Enemy.h"

struct ComboObject;

struct SwordProjectile : Enemy
{
	enum Action
	{
		S_SHOT,
		S_EXPLODE,
		S_Count
	};

	struct MyData : StoredEnemyData
	{
		V2d velocity;
		int shootFrames;
		int currHits;
	};
	MyData data;

	double speed;
	

	int shootLimit;
	int hitLimit;
	
	Tileset *ts;

	void SetLevel(int lev);

	SwordProjectile();
	~SwordProjectile();

	void Throw( int playerIndex, V2d &pos, V2d &dir);

	void ProcessState();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void ComboKill(Enemy *e);
	bool IsActive();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif