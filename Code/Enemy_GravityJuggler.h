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
		S_FINAL_POP,
		S_JUGGLE,
		S_RETURN,
		S_Count
	};

	struct MyData : StoredEnemyData
	{
		V2d velocity;
		int hitLimit;
		int currHits;
		int currJuggle;
		int juggleTextNumber;
		int waitFrame;
		bool doneBeingHittable;
	};
	MyData data;

	sf::Text numJugglesText;
	bool reversedGrav;
	bool limitedJuggles;
	double gravFactor;
	V2d gDir;
	double maxFallSpeed;
	Tileset *ts;
	bool reversed;
	int juggleReps;
	
	int maxWaitFrames;

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

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif