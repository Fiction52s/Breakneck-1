#ifndef __ENEMY_BEAMBOMB_H__
#define __ENEMY_BEAMBOMB_H__

#include "Enemy.h"
#include <vector>
#include "CircleGroup.h"

struct PoiInfo;
struct BeamBomb;

struct BeamBombPool
{
	BeamBombPool();
	~BeamBombPool();
	void Reset();
	BeamBomb * Throw(int type, V2d &pos, V2d &dir);
	//BeamBomb * ThrowAt(int type, V2d &pos, PoiInfo *pi);
	void DrawMinimap(sf::RenderTarget * target);
	std::vector<BeamBomb*> bombVec;
	Tileset *ts;
	int numBombs;
};


struct BeamBomb : Enemy, SurfaceMoverHandler,
	RayCastHandler
{
	enum Action
	{
		IDLE,
		BLAST_TEST,
		A_Count
	};

	enum BeamBombType
	{
		BOMB_NORMAL,
		BOMB_SINGLE,
		BOMB_Count,
	};

	Tileset *ts;

	int maxNumRays;

	double currRotVel;

	double rayLengthLimit;
	int numRays;
	double rayWidth;

	sf::Vertex *quads;

	BeamBombType bombType;

	PoiInfo *destPoi;
	int framesToArriveToDestPoi;

	double accel;
	double maxSpeed;

	double currRotation;

	sf::Color thornColor;

	std::vector<V2d> rayHitPoints;

	//CircleGroup cg;
	//sf::CircleShape pointTest;
	//sf::Color headColor;
	//sf::Color tailColor;

	//CollisionBody laserBody;

	BeamBomb(//sf::Vertex *quad,
		BeamBombPool *pool);
	~BeamBomb();
	void ClearQuads();
	void Throw(int type, V2d &pos, V2d &dir);
	//void ThrowAt(int type, V2d &pos, PoiInfo *pi);
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
	void SetBombTypeParams();
	void HitTerrainAerial(Edge * edge, double quant);
	bool CanBeHitByPlayer();
	bool CanBeHitByComboer();
	bool CheckHitPlayer( int index);
	//void HandleRayCollision(Edge *edge, double edgeQuantity,
	//	double rayPortion);
};

#endif