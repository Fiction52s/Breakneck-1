#ifndef __ENEMY_BALL_H__
#define __ENEMY_BALL_H__

#include "Enemy.h"

struct ComboObject;

struct Ball : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		S_FLOAT,
		S_FLY,
		S_BOUNCE,
		S_RETURN,
		S_Count
	};

	Ball(ActorParams *ap);
	~Ball();
	void SetLevel(int lev);
	void HitTerrainAerial(Edge * edge, double quant);
	void UpdateParamsSettings();
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void ComboKill(Enemy *e);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void Return();
	void Pop();
	void PopThrow();
	V2d GetBounceVel(V2d &normal);

	void Throw(double a, double strength);
	void Throw(V2d vel);

	double flySpeed;

	int hitLimit;
	int currHits;

	Tileset *ts;

	int juggleReps;
	int currJuggle;

	V2d *guidedDir;

	int waitFrame;
	int maxWaitFrames;
};

#endif