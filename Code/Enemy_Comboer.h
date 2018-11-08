#ifndef __ENEMY_Comboer_H__
#define __ENEMY_Comboer_H__

#include "Enemy.h"

struct Comboer : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_SHOT,
		S_EXPLODE,
		S_Count
	};

	Comboer(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, std::list<sf::Vector2i> &path, bool loop, int speed);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void HandleNoHealth();
	void AdvanceTargetNode();

	V2d velocity;


	Action action;
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