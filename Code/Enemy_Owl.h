#ifndef __ENEMY_OWL_H__
#define __ENEMY_OWL_H__

#include "Enemy.h"

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

	Owl(GameSession *owner,
		bool hasMonitor,
		sf::Vector2i &pos,
		int bulletSpeed,
		int framesBetween,
		bool facingRight);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
	void ActionEnded();
	
	void ProcessState();
	void UpdateEnemyPhysics();
	
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	
	double ang;
	sf::Vector2<double> fireDir;


	Tileset *ts_bulletExplode;
	Action action;
	int actionLength[A_Count];
	int animFactor[A_Count];

	int bulletSpeed;
	int movementRadius;
	int retreatRadius;
	int shotRadius;
	int chaseRadius;
	int framesBetween;

	CubicBezier flyingBez;

	sf::Vector2i originalPos;

	sf::Vector2<double> velocity;
	double flySpeed;

	Tileset *ts_death;
	Tileset *ts_flap;
	Tileset *ts_spin;
	Tileset *ts_throw;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;

	bool facingRight;

	Shield *shield;
};

#endif