#ifndef __ENEMY_PATROLLER_H__
#define __ENEMY_PATROLLER_H__

#include "Enemy.h"

struct Patroller : Enemy, LauncherEnemy
{
	enum Action
	{
		FLAP,
		TRANSFORM,
		CHARGEDFLAP,
		WAITINGFORBULLETS,
		DEAD,
	};

	Patroller(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, std::list<sf::Vector2i> &path, bool loop, int speed);
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
	void ProcessState();
	void HandleHitAndSurvive();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void HandleNoHealth();
	void AdvanceTargetNode();

	Action action;
	int fireCounter;
	int actionLength[CHARGEDFLAP + 1];
	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;
	int targetNode;
	bool forward;
	double acceleration;
	double speed;
	int nodeWaitFrames;
	sf::Sprite sprite;
	Tileset *ts;
	HitboxInfo *hitboxInfo;
	int animationFactor;
	bool facingRight;
};

#endif