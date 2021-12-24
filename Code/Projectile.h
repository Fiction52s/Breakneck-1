#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "Enemy.h"

struct Projectile;

struct ProjectileLauncher
{
	Tileset *ts;
	sf::Vertex *quads;
};

struct Projectile : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		THROWN,
		GRIND,
		A_Count
	};

	int hitlagFrames;
	int hitstunFrames;

	double flySpeed;

	V2d velocity;

	Projectile(ProjectileLauncher *pLauncher,
		int index );
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