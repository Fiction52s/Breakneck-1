#ifndef __ENEMY_WIREJUGGLER_H__
#define __ENEMY_WIREJUGGLER_H__

#include "Enemy.h"

struct ComboObject;

struct WireJuggler : Enemy
{
	enum JuggleType
	{
		T_WHITE,
		T_BLUE,
		T_RED,
		T_MAGENTA
	};

	enum Action
	{
		S_FLOAT,
		S_POP,
		S_JUGGLE,
		S_RETURN,
		S_HELD,
		S_Count
	};

	JuggleType jType;
	sf::Text numJugglesText;

	V2d velocity;
	bool limitedJuggles;

	bool reversedGrav;

	double gravFactor;
	V2d gDir;
	double maxFallSpeed;

	int hitLimit;
	int currHits;

	Tileset *ts;

	bool reversed;

	int juggleReps;
	int currJuggle;

	int waitFrame;
	int maxWaitFrames;

	WireJuggler( ActorParams *ap );
	~WireJuggler();
	void SetLevel(int lev);
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
	void HandleWireHit(Wire *w);
	void HandleWireAnchored(Wire *w);
	void HandleWireUnanchored(Wire *w);
	bool CanBeAnchoredByWire(bool red);
	void UpdateParamsSettings();
	void UpdateJuggleRepsText(int reps);

	void Throw(double a, double strength);
	void Throw(V2d vel);
	bool CanBeHitByWireTip(bool red);
	bool CanBeHitByComboer();

	
};

#endif