#ifndef __ENEMY_Comboer_H__
#define __ENEMY_Comboer_H__

#include "Enemy.h"

struct ComboObject;

struct Comboer : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_SHOT,
		S_EXPLODE,
		S_RETURN,
		S_Count
	};
	bool CountsForEnemyGate() { return false; }
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);

	Comboer(ActorParams * ap);
	~Comboer();

	void Return();
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();

	V2d velocity;
	int shootFrames;
	int shootLimit;
	int hitLimit;
	int currHits;

	BasicPathFollower pathFollower;

	double acceleration;
	double speed;
	
	Tileset *ts;
};

#endif