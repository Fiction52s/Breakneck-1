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
		S_WAIT,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int numEatenTextNumber;
		int currHits;
		V2d velocity;
		int growthLevel;
		Enemy *chaseTarget;
		int chaseIndex;
		int waitFrame;
		int numEaten;
	};
	MyData data;

	bool limitedEating;
	sf::Text numEatenText;

	int hitLimit;
	

	Tileset *ts;

	
	int numGrowthLevels;
	double origScale;
	double origSize;

	double flySpeed;

	int maxEdible;
	

	
	int maxWaitFrames;

	
	bool CountsForEnemyGate() { return false; }
	HungryComboer(ActorParams *ap);
	~HungryComboer();
	void SetLevel(int lev);
	//void HandleEntrant(QuadTreeEntrant *qte);
	bool IsValidTrackEnemy(Enemy *e);
	void UpdateParamsSettings();
	void ProcessState();
	void ProcessHit();
	void Eat();
	void IHitPlayer(int index);
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void ComboKill( Enemy *e );
	void EnemyDraw(sf::RenderTarget *target);
	void DirectKill();
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void Move();
	void Return();
	void Pop();
	void PopThrow();
	void UpdateScale();
	void UpdateEatenNumberText(int reps);

	double GetFlySpeed();
	V2d GetTrackPos();

	void Throw(double a, double strength);
	void Throw(V2d vel);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif