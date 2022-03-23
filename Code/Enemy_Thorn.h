#ifndef __ENEMY_THORN_H__
#define __ENEMY_THORN_H__

#include "Enemy.h"
#include <vector>

struct PoiInfo;
struct Thorn;

struct ThornPool
{
	ThornPool();
	~ThornPool();
	void Reset();
	Thorn * Throw(int type, V2d &pos, V2d &dir);
	Thorn * ThrowAt(int type, V2d &pos, PoiInfo *pi);
	void DrawMinimap(sf::RenderTarget * target);
	std::vector<Thorn*> thornVec;
	Tileset *ts;
	int numThorns;
};


struct Thorn : Enemy
{
	enum Action
	{
		IDLE,
		GROW,
		HOLD,
		SHRINK,
		A_Count
	};

	enum ThornType
	{
		THORN_NORMAL,
		THORN_Count,
	};

	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	double flySpeed;

	double lengthLimit;

	V2d velocity;

	sf::Vertex *quads;

	ThornType thornType;

	PoiInfo *destPoi;
	int framesToArriveToDestPoi;

	//int timeToLive;
	//int origTimeToLive;

	int maxPastPositions;

	double accel;
	double maxSpeed;

	int numActivePositions;

	std::vector<V2d> pastPositions;
	std::vector<V2d> normals;

	double laserWidth;

	//sf::Color headColor;
	//sf::Color tailColor;

	//CollisionBody laserBody;

	Thorn(//sf::Vertex *quad,
		ThornPool *pool);
	~Thorn();
	void ClearQuads();
	void Throw(int type, V2d &pos, V2d &dir);
	void ThrowAt(int type, V2d &pos, PoiInfo *pi);
	void SetLevel(int lev);
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void FrameIncrement();
	
	bool CanBeHitByPlayer();
	bool CanBeHitByComboer();
};

#endif