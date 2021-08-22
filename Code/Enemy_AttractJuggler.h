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

	bool CountsForEnemyGate() { return false; }
	AttractJuggler(ActorParams *ap);
	~AttractJuggler();
	void UpdateParamsSettings();
	void ProcessState();
	void ProcessHit();
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

	V2d velocity;

	sf::Text numJugglesText;
	bool limitedJuggles;

	double gravFactor;
	double maxFallSpeed;

	int hitLimit;
	int currHits;

	Tileset *ts;

	bool reversed;

	int juggleReps;
	int currJuggle;

	int waitFrame;
	int maxWaitFrames;
};

#endif