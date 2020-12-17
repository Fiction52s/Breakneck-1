#ifndef __ENEMY_GRAVITYJUGGLER_H__
#define __ENEMY_GRAVITYJUGGLER_H__

#include "Enemy.h"

struct ComboObject;

struct GravityJuggler : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_POP,
		S_JUGGLE,
		S_RETURN,
		S_Count
	};

	GravityJuggler(ActorParams *ap );
	~GravityJuggler();
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
	void UpdateJuggleRepsText( int reps );

	void Throw(double a, double strength);
	void Throw(V2d vel);
	void SetLevel(int lev);

	V2d velocity;

	sf::Text numJugglesText;
	bool reversedGrav;
	bool limitedJuggles;

	double gravFactor;
	V2d gDir;
	double maxFallSpeed;

	int hitLimit;
	int currHits;

	Tileset *ts;

	bool reversed;

	int juggleReps;
	int currJuggle;

	V2d *guidedDir;

	int waitFrame;
	int maxWaitFrames;
};

#endif