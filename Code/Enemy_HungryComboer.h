#ifndef __ENEMY_HUNGRYCOMBOER_H__
#define __ENEMY_HUNGRYCOMBOER_H__

#include "EnemyTracker.h"
#include "Enemy.h"

struct ComboObject;

struct HungryComboer : Enemy, EnemyTracker
{
	enum Action
	{
		S_FLOAT,
		S_FLY,
		S_TRACKPLAYER,
		S_TRACKENEMY,
		S_RETURN,
		A_Count
	};

	Enemy *chaseTarget;
	int chaseIndex;

	V2d velocity;

	double gravFactor;
	double maxFallSpeed;

	int hitLimit;
	int currHits;

	Tileset *ts;

	bool returnsToPlayer;

	int growthLevel;
	int numGrowthLevels;
	double origScale;
	double origSize;

	double flySpeed;

	int juggleReps;
	int currJuggle;

	int waitFrame;
	int maxWaitFrames;

	HungryComboer(ActorParams *ap);
	~HungryComboer();
	void SetLevel(int lev);
	//void HandleEntrant(QuadTreeEntrant *qte);
	bool IsValidTrackEnemy(Enemy *e);
	void UpdateParamsSettings();
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void ComboKill( Enemy *e );
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void Move();
	void Return();
	void Pop();
	void PopThrow();
	void UpdateScale();

	double GetFlySpeed();
	V2d GetTrackPos();

	void Throw(double a, double strength);
	void Throw(V2d vel);

	
};

#endif