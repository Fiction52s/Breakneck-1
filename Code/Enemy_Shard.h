#ifndef __ENEMY_SHARD_H__
#define __ENEMY_SHARD_H__

#include "Enemy.h"
#include <map>

#include "MovingGeo.h"

struct TutorialBox;
struct EffectPool;
struct TilesetManager;
struct Session;

struct ShardPopup
{
	enum State
	{
		SHOW,
		Count
	};


	ShardPopup();
	~ShardPopup();
	void SetTopLeft(sf::Vector2f &pos);
	void SetCenter(sf::Vector2f &pos);
	void Reset();
	void Update();
	void SetShard(int w, int li);
	void SetDescription(const std::string &desc);
	void Draw(sf::RenderTarget *target);

	TutorialBox *tutBox;
	sf::Sprite shardSpr;
	
	float borderHeight;
	float shardBorder;
	float width;
	float height;
	float shardSize;
	float nameHeight;

	sf::Vertex bgQuad[4];
	sf::Vertex topBorderQuad[4];
	sf::Vertex shardBorderQuad[4];

	sf::Text nameText;

	sf::Vector2f topLeft;
	sf::Vector2f descBorder;
	sf::Vector2f shardRel;

	Session *sess;
	int w;
	int li;
	int frame;
	State state;

};

struct GetShardSequence;
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
	bool CountsForEnemyGate() { return false; }
	ShapeEmitter *testEmitter;
	MovingGeoGroup geoGroup;

	static Tileset *GetShardTileset(int w,
		TilesetManager *ttm);
	
	bool alreadyCollected;
	void Launch();
	//SpinningTri *triTest[5];
	//Laser *laser;
	//MovingRing *mRing;
	void DirectKill();

	static int GetShardTypeFromWorldAndIndex(int w, int li);
	static int GetNumShardsTotal();
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

	GetShardSequence *shardSeq;
	Tileset *ts;

	int shardType;

	bool caught;
	int totalFrame;

	int shardWorld;
	int localIndex;
};

#endif