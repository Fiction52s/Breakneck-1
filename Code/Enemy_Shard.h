#ifndef __ENEMY_SHARD_H__
#define __ENEMY_SHARD_H__

#include "Enemy.h"
#include <map>
#include "ShardTypes.h"

#include "MovingGeo.h"


struct EffectPool;
struct TilesetManager;
struct GameSession;

struct ShardPopup
{
	enum State
	{
		SHOW,
		Count
	};


	ShardPopup( GameSession *p_owner );
	void SetTopLeft(sf::Vector2f &pos);
	void SetCenter(sf::Vector2f &pos);
	void Reset();
	void Update();
	void SetShard(int w, int li);
	void SetDescription(const std::string &desc);
	void Draw(sf::RenderTarget *target);
	sf::Text desc;
	sf::Sprite shardSpr;
	sf::Sprite effectSpr;
	sf::Sprite bgSpr;

	sf::Vector2f topLeft;
	sf::Vector2f descRel;
	sf::Vector2f effectRel;
	sf::Vector2f shardRel;

	GameSession *owner;
	int w;
	int li;
	int frame;
	State state;

};


struct ShapeEmitter;
struct Shard : Enemy
{
	enum Action
	{
		FLOAT,
		DISSIPATE,
		LAUNCH,
		Count
	};

	Shard(ActorParams *ap);//sf::Vector2i pos,
		//int w, int li);
	~Shard();
	void UpdateParamsSettings();
	void Setup();

	ShapeEmitter *testEmitter;
	MovingGeoGroup geoGroup;

	static Tileset *GetShardTileset(int w,
		TilesetManager *ttm);
	
	bool alreadyCollected;
	void Launch();
	//SpinningTri *triTest[5];
	//Laser *laser;
	//MovingRing *mRing;
	static int GetShardType(const std::string &str);
	static int GetShardType(int w, int li);
	static std::string GetShardString(ShardType st);
	static std::string GetShardString(int w,
		int li );
	static void SetupShardMaps();
	int actionLength[Count];
	int animFactor[Count];
	void DirectKill();

	//MovementSequence testSeq;
	
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void IHitPlayer(int index);
	void ProcessHit();
	void ResetEnemy();
	void DissipateOnTouch();
	void Capture();
	void FrameIncrement();
	Tileset *ts_sparkle;
	EffectPool *sparklePool;
	Tileset *ts_explodeCreate;
	V2d rootPos;
	int radius;

	Tileset *ts;

	int shardType;

	bool caught;
	int totalFrame;

	GameSession *game;
	int shardWorld;
	int localIndex;

private:
	static std::map<std::string, ShardType> shardTypeMap;
	static std::map<ShardType, std::string> shardStrMap;
};

#endif