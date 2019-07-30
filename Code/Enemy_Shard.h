#ifndef __ENEMY_SHARD_H__
#define __ENEMY_SHARD_H__

#include "Enemy.h"
#include <map>
#include "ShardTypes.h"
#include "MovingGeo.h"

struct EffectPool;
struct TilesetManager;
struct Shard : Enemy
{
	enum Action
	{
		FLOAT,
		DISSIPATE,
		Count
	};

	static Tileset *GetShardTileset(int w,
		TilesetManager *ttm);
	MovingGeoGroup geoGroup;
	//SpinningTri *triTest[5];
	//Laser *laser;
	//MovingRing *mRing;
	static ShardType GetShardType(const std::string &str);
	static ShardType GetShardType(int w, int li);
	static std::string GetShardString(ShardType st);
	static std::string GetShardString(int w,
		int li );
	static void SetupShardMaps();
	int actionLength[Count];
	int animFactor[Count];
	void DirectKill();

	Action action;
	//MovementSequence testSeq;
	Shard(GameSession *owner, sf::Vector2i pos, 
		int w, int li);
	~Shard();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void IHitPlayer(int index);
	void ProcessHit();
	void UpdateHitboxes();
	void ResetEnemy();
	void DissipateOnTouch();
	void Capture();
	void FrameIncrement();
	Tileset *ts_sparkle;
	EffectPool *sparklePool;
	Tileset *ts_explodeCreate;
	V2d startPos;
	int radius;

	sf::Sprite sprite;
	Tileset *ts;

	ShardType shardType;

	bool caught;
	int totalFrame;

	int world;
	int localIndex;

private:
	static std::map<std::string, ShardType> shardTypeMap;
	static std::map<ShardType, std::string> shardStrMap;
};

#endif