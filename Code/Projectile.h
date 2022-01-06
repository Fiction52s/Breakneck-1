#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "Enemy.h"
#include "EnemyMover.h"

struct Projectile;

struct ProjectileLauncher
{
	enum ProjectileType
	{
		TEST,
		P_Count
	};

	ProjectileLauncher( int projType );
	void SetTileset(Tileset *p_ts);
	Tileset *ts;
	sf::Vertex *quads;
	std::vector<Projectile*> projectileVec;
};

struct ProjectileParams
{
	int projType;
	V2d position;
	V2d velocity;
};

struct Projectile : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		IDLE,
		LINEAR_MOVE,
		A_Count
	};

	V2d velocity;
	ProjectileLauncher *launcher;
	sf::Vertex *quad;
	int index;

	Projectile(ProjectileLauncher *pLauncher,
		int index );
	void Launch(ProjectileParams &params);
	void Throw(V2d &pos, V2d &dir);
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();
	bool IsEnemyMoverAction(int a);

	void HitTerrainAerial(Edge *, double);
};

#endif