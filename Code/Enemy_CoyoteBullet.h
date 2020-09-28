#ifndef __ENEMY_COYOTEBULLET_H__
#define __ENEMY_COYOTEBULLET_H__

#include "Enemy.h"
#include <vector>

struct CoyoteBullet;

struct CoyoteBulletPool
{
	CoyoteBulletPool();
	~CoyoteBulletPool();
	void Reset();
	CoyoteBullet * Throw(V2d &pos, V2d &dir);
	void Draw(sf::RenderTarget *target);
	std::vector<CoyoteBullet*> bulletVec;
	sf::Vertex *verts;
	Tileset *ts;
	int numBullets;
};

struct CoyoteBullet : Enemy
{
	enum Action
	{
		DASHING,
		WAITING,
		A_Count
	};

	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	double dashSpeed;

	V2d velocity;

	sf::Vertex *quad;

	CoyoteBullet(sf::Vertex *quad, 
		CoyoteBulletPool *pool );
	void Kill();
	V2d GetThrowDir(V2d &dir);
	void UpdateEnemyPhysics();
	void Throw(V2d &pos, V2d &dir);
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();
};

#endif