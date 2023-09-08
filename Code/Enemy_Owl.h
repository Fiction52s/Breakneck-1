#ifndef __ENEMY_OWL_H__
#define __ENEMY_OWL_H__

#include "Enemy.h"
#include "Bullet.h"

struct Owl : Enemy, LauncherEnemy
{
	enum Action
	{
		REST,
		GUARD,
		SPIN,
		FIRE,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		//V2d velocity;
	};
	MyData data;

	double ang;
	V2d fireDir;

	Shield *shield;

	int bulletSpeed;
	int movementRadius;
	int retreatRadius;
	int shotRadius;
	int chaseRadius;
	int framesBetween;

	CubicBezier flyingBez;

	
	double flySpeed;

	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	Owl(ActorParams *ap);
	~Owl();
	void SetLevel(int lev);
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void ActionEnded();
	void ShieldDestroyed(Shield *shield);
	void ProcessState();
	void UpdateEnemyPhysics();
	
	void UpdateSprite();
	void ResetEnemy();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

};

#endif