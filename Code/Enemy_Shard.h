#ifndef __ENEMY_SHARD_H__
#define __ENEMY_SHARD_H__

#include "Enemy.h"
#include <map>

struct Shard : Enemy
{
	enum Action
	{
		FLOAT,
		DISSIPATE,
		Count
	};

	static ShardType GetShardType(const std::string &str);
	static std::string GetShardString(ShardType st);
	static void SetupShardMaps();
	int actionLength[Count];
	int animFactor[Count];
	Action action;
	//MovementSequence testSeq;
	Shard(GameSession *owner, sf::Vector2i pos, ShardType p_sType );
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

	int radius;

	sf::Sprite sprite;
	Tileset *ts;

	ShardType shardType;

	bool caught;

private:
	static std::map<std::string, ShardType> shardTypeMap;
	static std::map<ShardType, std::string> shardStrMap;
};

#endif