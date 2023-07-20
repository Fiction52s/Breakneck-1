#ifndef __ENEMY_SPECTERBULLET_H__
#define __ENEMY_SPECTERBULLET_H__

#include "Enemy.h"
#include "Bullet.h"
#include <vector>

struct SpecterBullet;

struct SpecterBulletPool
{
	SpecterBulletPool();
	~SpecterBulletPool();
	void Reset();
	SpecterBullet *Throw( V2d &pos, V2d &dir );
	void Draw(sf::RenderTarget *target);
	void SetEnemyIDsAndAddToGame();
	std::vector<SpecterBullet*> bulletVec;
	sf::Vertex *verts;
	Tileset *ts;
	int numBullets;
};

struct SpecterBullet : Enemy
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
	double acceleration;
	double maxSpeed;

	Tileset *ts;

	sf::Vertex *quad;

	SpecterBulletPool *pool;

	SpecterBullet(SpecterBulletPool *p_pool, sf::Vertex *quad);
	bool IsHitFacingRight();
	void Die();
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