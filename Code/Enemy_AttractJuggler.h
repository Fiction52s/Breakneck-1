#ifndef __ENEMY_ATTRACTJUGGLER_H__
#define __ENEMY_ATTRACTJUGGLER_H__

#include "Enemy.h"

struct ComboObject;

struct AttractJuggler : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_POP,
		S_JUGGLE,
		S_RETURN,
		S_Count
	};

	struct MyData : StoredEnemyData
	{
		V2d velocity;
		int currJuggle;
		int currHits;
		int waitFrame;
		int juggleTextNumber;
	};
	MyData data;

	

	sf::Text numJugglesText;
	bool limitedJuggles;

	double gravFactor;
	double maxFallSpeed;

	int hitLimit;
	
	Tileset *ts;

	bool reversed;

	int juggleReps;
	
	int maxWaitFrames;

	bool CountsForEnemyGate() { return false; }
	AttractJuggler(ActorParams *ap);
	~AttractJuggler();
	void UpdateParamsSettings();
	void ProcessState();
	void ProcessHit();
	void DirectKill();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void Move();
	void Return();
	void Pop();
	void PopThrow();
	void UpdateJuggleRepsText(int reps);
	

	void Throw(double a, double strength);
	void Throw(V2d vel);
	void SetLevel(int lev);
	void HandleWireHit(Wire *w);
	bool CanBeAnchoredByWire(bool red);
	bool CanBeHitByWireTip(bool red);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);


};

#endif