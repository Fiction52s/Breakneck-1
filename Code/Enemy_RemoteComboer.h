#ifndef __ENEMY_REMOTECOMBOER_H__
#define __ENEMY_REMOTECOMBOER_H__

#include "Enemy.h"

struct ComboObject;

struct RemoteComboer : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_SHOT,
		S_EXPLODE,
		S_RETURN,
		S_Count
	};

	V2d velocity;
	int shootFrames;
	int shootLimit;
	int hitLimit;
	int currHits;

	double acceleration;
	double speed;

	Tileset *ts;

	Actor *controlPlayer;

	void SetLevel(int lev);
	bool CountsForEnemyGate() { return false; }
	RemoteComboer(ActorParams * ap);
	~RemoteComboer();

	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
};

#endif