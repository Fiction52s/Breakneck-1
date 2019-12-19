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
		S_Count
	};

	HungryComboer(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, int p_level,
		int juggleReps, bool returnsToPlayer );
	~HungryComboer();
	void HandleEntrant(QuadTreeEntrant *qte);
	bool IsValidTrackEnemy(Enemy *e);
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

	void Throw(double a, double strength);
	void Throw(V2d vel);

	Enemy *chaseTarget;
	int chaseIndex;

	V2d GetTrackPos();

	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];

	V2d origPos;

	V2d velocity;

	double gravFactor;
	double maxFallSpeed;

	int hitLimit;
	int currHits;

	sf::Sprite sprite;
	Tileset *ts;

	bool returnsToPlayer;

	int growthLevel;
	int numGrowthLevels;
	double origScale;
	double origSize;

	void UpdateScale();

	double GetFlySpeed();

	double flySpeed;

	int juggleReps;
	int currJuggle;

	int waitFrame;
	int maxWaitFrames;
};

#endif