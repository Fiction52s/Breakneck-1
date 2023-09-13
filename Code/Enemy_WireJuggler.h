#ifndef __ENEMY_WIREJUGGLER_H__
#define __ENEMY_WIREJUGGLER_H__

#include "Enemy.h"

struct ComboObject;

struct WireJuggler : Enemy
{
	enum JuggleType
	{
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


	struct MyData : StoredEnemyData
	{
		V2d velocity;
		int currHits;
		int waitFrame;
	};
	MyData data;

	JuggleType jType;

	double gravFactor;
	V2d gDir;
	double maxFallSpeed;
	
	Tileset *ts;
	
	int maxWaitFrames;

	bool CountsForEnemyGate() { return false; }
	WireJuggler( ActorParams *ap );
	~WireJuggler();
	void SetLevel(int lev);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	HitboxInfo * IsHit(int pIndex);
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

	void Throw(double a, double strength);
	void Throw(V2d vel);
	bool CanBeHitByWireTip(bool red);
	bool CanBeHitByComboer();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif