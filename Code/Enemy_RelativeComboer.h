#ifndef __ENEMY_RELATIVECOMBOER_H__
#define __ENEMY_RELATIVECOMBOER_H__

#include "Enemy.h"

struct ComboObject;

struct RelativeComboer : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_FLY,
		S_ATTACHEDWAIT,
		S_WAIT,
		S_RETURN,
		S_Count
	};


	int latchFrame;
	int maxLatchFrames;

	int specialPauseFrames;

	V2d velocity;

	int hitLimit;
	int currHits;

	Tileset *ts;

	bool detachOnKill;

	int juggleReps;
	int currJuggle;

	int waitFrame;
	int maxWaitFrames;

	double flySpeed;

	bool latchedOn;
	V2d basePos;

	V2d offsetPos;

	RelativeComboer(
		ActorParams *ap);/*GameSession *owner, bool hasMonitor,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &path, int p_level,
		int juggleReps, bool detachOnKill*/
	~RelativeComboer();
	void SetLevel(int lev);
	void UpdateParamsSettings();
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	bool CanBeHitByComboer();

	void ComboKill(Enemy *e);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void Move();
	void Return();
	void Pop();
	void PopThrow();

	void Throw(double a, double strength);
	void Throw(V2d vel);

};

#endif