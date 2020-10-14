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

	double ang;
	V2d fireDir;

	Shield *shield;

	Tileset *ts_bulletExplode;

	int bulletSpeed;
	int movementRadius;
	int retreatRadius;
	int shotRadius;
	int chaseRadius;
	int framesBetween;

	CubicBezier flyingBez;

	V2d velocity;
	double flySpeed;

	Tileset *ts_death;
	Tileset *ts_flap;
	Tileset *ts_spin;
	Tileset *ts_throw;

	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	Owl(ActorParams *ap);
	void SetLevel(int lev);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult);
	void ActionEnded();
	void ShieldDestroyed(Shield *shield);
	void ProcessState();
	void UpdateEnemyPhysics();
	
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();

};

#endif