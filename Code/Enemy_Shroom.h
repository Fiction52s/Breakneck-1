#ifndef _ENEMY_SHROOM__H_
#define _ENEMY_SHROOM__H_

#include "Enemy.h"

struct ShroomJelly;

struct Shroom : Enemy
{
	enum Action
	{
		LATENT,
		HITTING,
		A_Count
	};

	Shroom(ActorParams * ap);
	~Shroom();
	
	
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void HandleNoHealth();
	void DirectKill();
	void ResetEnemy();
	void IHitPlayer(int index = 0);
	void ProcessState();

	void CheckedMiniDraw(sf::RenderTarget *target,
		sf::FloatRect &rect);
	void SetZoneSpritePosition();

	sf::SoundBuffer *hitSound;

	Tileset *ts;

	bool jellySpawnable;

	ShroomJelly *jelly;
};

struct ShroomJelly : Enemy
{
	enum Action
	{
		WAIT,
		APPEARING,
		RISING,
		DROOPING,
		DISSIPATING,
		SHOT,
		EXPLODING,
		A_Count
	};

	void SetLevel(int lev);
	ShroomJelly(Shroom *shr);
	~ShroomJelly();
	void EnemyDraw(sf::RenderTarget *target);
	
	void UpdateSprite();
	void ResetEnemy();
	void ProcessState();
	void UpdateEnemyPhysics();
	//void ProcessHit();
	void ComboHit();

	Tileset *ts;
	
	sf::SoundBuffer *floatSound;

	int cycleLimit;
	int currentCycle;

	int shootFrames;
	int shootLimit;
	int hitLimit;
	int currHits;

	V2d velocity;

	CubicBezier risingBez;
	CubicBezier fallingBez;

	double angle;

	Shroom *shroom;
};

#endif