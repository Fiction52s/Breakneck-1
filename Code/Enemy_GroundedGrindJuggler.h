#ifndef __ENEMY_GROUNDGRINDJUGGLER_H__
#define __ENEMY_GROUNDGRINDJUGGLER_H__

#include "Enemy.h"

struct ComboObject;

struct GroundedGrindJuggler : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		S_IDLE,
		S_GRIND,
		S_SLOW,
		S_STOPPED,
		S_RETURN,
		S_Count
	};

	V2d velocity;

	int hitLimit;
	int currHits;

	Tileset *ts;

	int juggleReps;
	int currJuggle;

	int waitFrame;
	int maxWaitFrames;

	double friction;
	double pushStart;

	bool clockwise;

	GroundedGrindJuggler(ActorParams *ap);

	~GroundedGrindJuggler();
	void UpdateParamsSettings();
	void SetLevel(int lev);
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
	void Move();
	void Return();
	void Push(double strength);


};

#endif