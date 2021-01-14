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

	void SetLevel(int lev);

	SwordProjectile();
	~SwordProjectile();

	void Throw( int playerIndex, V2d &pos, V2d &dir);

	void ProcessState();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void ComboKill(Enemy *e);

	V2d velocity;
	int shootFrames;
	int shootLimit;
	int hitLimit;
	int currHits;
	bool IsActive();

	double speed;

	Tileset *ts;
};

#endif