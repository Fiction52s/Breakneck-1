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
		S_TRACKENEMY,
		S_RETURN,
		S_WAIT,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		V2d velocity;
		Enemy *chaseTarget;
		int chaseIndex;
		int waitFrame;
		int numEaten;
	};
	MyData data;

	Tileset *ts;
	
	double origScale;
	double origSize;

	double flySpeed;

	int maxEdible;
	
	int maxWaitFrames;

	double homingRadius;

	//bool IsHomingTarget() { return false; }
	bool CountsForEnemyGate() { return false; }
	HungryComboer(ActorParams *ap);
	~HungryComboer();
	
	void SetLevel(int lev);
	//void HandleEntrant(QuadTreeEntrant *qte);
	bool CheckIfEnemyIsTrackable(Enemy *e);
	void ProcessState();
	void ProcessHit();
	void Eat();
	void IHitPlayer(int index);
	void UpdateEnemyPhysics();
	void FrameIncrement();
	bool CanComboHit(Enemy *e);
	void ComboHit();
	void ComboKill( Enemy *e );
	void EnemyDraw(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	void DirectKill();
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

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif