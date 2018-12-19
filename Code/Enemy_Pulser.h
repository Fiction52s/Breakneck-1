#ifndef __ENEMY_PULSER_H__
#define __ENEMY_PULSER_H__

#include "Enemy.h"

struct Pulser : Enemy
{

	enum Action
	{
		SHIELDOFF,
		SHIELDON,
		NOSHIELD,
		A_COUNT
	};

	Action action;
	int actionLength[A_COUNT];
	int animFactor[A_COUNT];

	Pulser(GameSession *owner,
		bool hasMonitor,
		sf::Vector2i &pos,
		std::list<sf::Vector2i> &path,
		int framesBetween,
		bool loop);
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	Shield *shield;

	int framesBetween;
	MovementSequence testSeq;

	int pulseWait;

	sf::Vector2i *path; //global
	int pathLength;
	bool loop;

	int nodeWaitFrames;
	sf::Sprite sprite;
	Tileset *ts;
	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	HitboxInfo *hitboxInfo;

	bool facingRight;
};

#endif