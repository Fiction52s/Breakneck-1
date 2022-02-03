#ifndef __ENEMY_SKELETONLASER_H__
#define __ENEMY_SKELETONLASER_H__

#include "Enemy.h"
#include <vector>

struct PoiInfo;
struct SkeletonLaser;

struct SkeletonLaserPool
{
	SkeletonLaserPool();
	~SkeletonLaserPool();
	void Reset();
	SkeletonLaser * Throw(int type, V2d &pos, V2d &dir);

	SkeletonLaser * ThrowAt(int type, V2d &pos, PoiInfo *pi);
	//void Draw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget * target);
	std::vector<SkeletonLaser*> laserVec;
	//sf::Vertex *verts;
	Tileset *ts;
	int numLasers;
};


struct SkeletonLaser : Enemy,
	SurfaceMoverHandler
{
	enum Action
	{
		THROWN,
		THROWN_AT,
		DISSIPATE,
		A_Count
	};

	enum LaserType
	{
		LT_REGULAR,
		LT_Count,
	};

	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	double flySpeed;

	double lengthLimit;

	V2d velocity;

	//sf::Vertex *quad;

	sf::Vertex *quads;

	LaserType laserType;

	PoiInfo *destPoi;
	int framesToArriveToDestPoi;

	int maxBounces;
	int currBounce;

	std::vector<V2d> anchorPositions;

	double laserWidth;


	SkeletonLaser(//sf::Vertex *quad,
		SkeletonLaserPool *pool);
	void ClearQuads();
	V2d GetThrowDir(V2d &dir);
	void Throw(int type, V2d &pos, V2d &dir);
	void ThrowAt(int type, V2d &pos, PoiInfo *pi);
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