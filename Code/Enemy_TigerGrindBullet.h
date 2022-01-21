#ifndef __ENEMY_TIGERGRINDBULLET_H__
#define __ENEMY_TIGERGRINDBULLET_H__

#include "Enemy.h"
#include <vector>

struct TigerGrindBullet;
struct GrindFire;
struct PoiInfo;

struct GrindFirePool
{
	GrindFirePool();
	~GrindFirePool();
	void Reset();
	GrindFire * Create( int type, V2d &pos, 
		Edge *ground,
		double quant );
	void Draw(sf::RenderTarget *target);
	std::vector<GrindFire*> fireVec;
	sf::Vertex *verts;
	Tileset *ts;
	int numFires;
};


struct GrindFire : Enemy
{
	enum Action
	{
		BURN,
		A_Count
	};

	Tileset *ts;

	int gbType;

	sf::Vertex *quad;

	GrindFire(sf::Vertex *quad,
		GrindFirePool *pool);
	V2d GetThrowDir(V2d &dir);
	void Create( int type, V2d &pos, Edge *ground,
		double quant );
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();
};




struct TigerGrindBulletPool
{
	TigerGrindBulletPool();
	~TigerGrindBulletPool();
	void Reset();
	TigerGrindBullet * Throw( int type, V2d &pos, V2d &dir);

	TigerGrindBullet * ThrowAt(int type, V2d &pos, PoiInfo *pi );
	void Draw(sf::RenderTarget *target);
	std::vector<TigerGrindBullet*> bulletVec;
	sf::Vertex *verts;
	Tileset *ts;
	int numBullets;
};

struct TigerGrindBullet : Enemy,
	SurfaceMoverHandler
{
	enum Action
	{
		THROWN,
		THROWN_AT,
		GRIND,
		A_Count
	};

	enum GrindBulletType
	{
		GB_REGULAR_CW,
		GB_LARGE_CW,
		GB_FAST_CW,
		GB_REGULAR_CCW,
		GB_LARGE_CCW,
		GB_FAST_CCW,
	};

	GrindFirePool firePool;
	

	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	double flySpeed;

	V2d velocity;

	sf::Vertex *quad;

	GrindBulletType gbType;

	PoiInfo *destPoi;
	int framesToArriveToDestPoi;

	TigerGrindBullet(sf::Vertex *quad,
		TigerGrindBulletPool *pool);
	V2d GetThrowDir(V2d &dir);
	void Throw( int type, V2d &pos, V2d &dir);
	void ThrowAt(int type, V2d &pos, PoiInfo *pi);
	void StartGrind();
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();

	void HitTerrainAerial(Edge *, double);
};



#endif