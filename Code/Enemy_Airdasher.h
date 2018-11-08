#ifndef __ENEMY_AIRDASHER_H__
#define __ENEMY_AIRDASHER_H__

#include "Enemy.h"

struct Airdasher : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_DASH,
		S_RETURN,
		S_Count
	};

	Airdasher(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos );
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();

	V2d velocity;

	CubicBezier dashBez;
	CubicBezier returnBez;

	double dashRadius;
	V2d playerDir;
	int dashFrames;
	int returnFrames;

	Action action;
	int physStepIndex;
	V2d origPos;
	int actionLength[S_Count];
	int animFactor[S_Count];
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
	bool facingRight;
};

#endif