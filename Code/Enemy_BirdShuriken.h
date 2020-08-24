#ifndef __ENEMY_BIRDSHURIKEN_H__
#define __ENEMY_BIRDSHURIKEN_H__

#include "Enemy.h"
#include "Bullet.h"
#include <vector>

struct BirdShuriken;

struct BirdShurikenPool
{
	BirdShurikenPool();
	~BirdShurikenPool();
	void Reset();
	BirdShuriken *Throw(V2d &pos, V2d &dir, 
		int sType );
	void Draw(sf::RenderTarget *target);
	std::vector<BirdShuriken*> shurVec;
	void RethrowAll();
	sf::Vertex *verts;
	Tileset *ts;
	int numShurs;
};

struct BirdShuriken : Enemy, LauncherEnemy,
	SurfaceMoverHandler
{
	enum Action
	{
		THROWN,
		STICK,
		RETHROW,
		TURRET,
		A_Count
	};

	enum ShurikenType
	{
		UNBLOCKABLE,
		UNDODGEABLE,
		MACHINEGUNTURRET,
	};

	int bulletSpeed;
	int framesBetween;

	Tileset *ts_bulletExplode;

	V2d currBasePos;
	V2d retreatPos;
	V2d startRetreatPos;

	int fireCounter;

	double acceleration;
	double speed;
	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	double thrownSpeed;
	double accel;

	double startUnDodgeSpeed;
	double unDodgeAccel;
	double unDodgeMaxSpeed;
	double unDodgeSpeed;

	sf::Vertex *quad;

	int shurType;


	BirdShuriken(sf::Vertex *quad );
	void Throw( V2d &pos, V2d &dir,
		int shurType );
	void Rethrow();
	void SetLevel(int lev);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(
		int playerIndex,
		BasicBullet *b,
		int hitResult);
	void ProcessState();
	bool physicsOver;
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void FrameIncrement();
	void HitTerrainAerial(Edge *, double);
};

#endif