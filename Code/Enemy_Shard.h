#ifndef __ENEMY_SHARD_H__
#define __ENEMY_SHARD_H__

#include "Enemy.h"

struct Shard : Enemy
{
	enum Action
	{
		FLOAT,
		DISSIPATE,
		Count
	};

	int actionLength[Count];
	int animFactor[Count];
	Action action;
	//MovementSequence testSeq;
	Shard(GameSession *owner, sf::Vector2i pos,
		int shardsLoaded, ShardType p_sType );
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void IHitPlayer(int index);
	void ProcessHit();
	void UpdateHitboxes();
	void ResetEnemy();
	void DissipateOnCapture();

	CollisionBody *hitBody;
	CollisionBody *hurtBody;

	int shardIndex;

	int radius;

	sf::Sprite sprite;
	Tileset *ts;

	ShardType shardType;

	bool caught;
};

#endif