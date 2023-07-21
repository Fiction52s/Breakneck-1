#ifndef __ENEMY_GORILLAWALL_H__
#define __ENEMY_GORILLAWALL_H__

#include "Enemy.h"
#include "Bullet.h"
#include <vector>

struct GorillaWall;

struct GorillaWallPool
{
	GorillaWallPool();
	~GorillaWallPool();
	void Reset();
	GorillaWall *Throw( V2d &pos, V2d &dir );
	void Draw(sf::RenderTarget *target);
	void SetEnemyIDsAndAddToGame();
	void DebugDraw(sf::RenderTarget *target);
	/*int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);*/
	std::vector<GorillaWall*> bulletVec;
	sf::Vertex *verts;
	Tileset *ts;
	int numBullets;
};

struct GorillaWall : Enemy
{
	enum Action
	{
		THROWN,
		THROWN_VULNERABLE,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		V2d velocity;
		int framesToLive;
	};
	MyData data;


	double throwSpeed;

	/*double throwSpeed;
	double acceleration;
	double maxSpeed;*/

	double wallLength;
	double wallThickness;

	Tileset *ts;

	sf::Vertex *quad;

	GorillaWallPool *pool;

	GorillaWall(GorillaWallPool *p_pool, sf::Vertex *quad);
	bool IsHitFacingRight();
	void Die();
	void UpdateHitboxes();
	void ProcessHit();
	bool IsSlowed(int index);
	void Throw(V2d &pos, V2d &dir);
	void SetLevel(int lev);
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif