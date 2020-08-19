#ifndef __ENEMY_TIGERGRINDBULLET_H__
#define __ENEMY_TIGERGRINDBULLET_H__

#include "Enemy.h"
#include <vector>

struct TigerGrindBullet;
struct GrindFire;

struct GrindFirePool
{
	GrindFirePool();
	~GrindFirePool();
	void Reset();
	GrindFire * Create(V2d &pos, 
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

	sf::Vertex *quad;

	GrindFire(sf::Vertex *quad,
		GrindFirePool *pool);
	V2d GetThrowDir(V2d &dir);
	void Create(V2d &pos, Edge *ground,
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
	TigerGrindBullet * Throw(V2d &pos, V2d &dir);
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
		GRIND,
		A_Count
	};

	GrindFirePool firePool;
	

	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	double flySpeed;

	V2d velocity;

	sf::Vertex *quad;

	TigerGrindBullet(sf::Vertex *quad,
		TigerGrindBulletPool *pool);
	V2d GetThrowDir(V2d &dir);
	void Throw(V2d &pos, V2d &dir);
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