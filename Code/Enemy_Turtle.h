#ifndef __ENEMY_TURTLE_H__
#define __ENEMY_TURTLE_H__

#include "Enemy.h"
#include "Bullet.h"

struct Turtle : Enemy, LauncherEnemy
{
	enum Action
	{
		NEUTRAL,
		FIRE,
		INVISIBLE,
		FADEIN,
		FADEOUT,
		A_Count
	};

	Turtle(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, int p_level);
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void DirectKill();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	
	
	void UpdateSprite();
	void ResetEnemy();

	int actionLength[A_Count];
	int animFactor[A_Count];

	Tileset *ts_bulletExplode;
	int bulletSpeed;

	Action action;

	sf::Vector2i originalPos;

	int fireCounter;

	sf::Sprite sprite;
	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	V2d playerTrackPos;

	bool facingRight;

};

#endif