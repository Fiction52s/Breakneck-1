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


	struct MyData : StoredEnemyData
	{
		bool jellySpawnable;
	};
	MyData data;

	SoundInfo *hitSound;

	Tileset *ts;

	

	ShroomJelly *jelly;

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
	int GetNumEnergyAbsorbParticles();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
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

	struct MyData : StoredEnemyData
	{
		int currentCycle;
		int shootFrames;
		int currHits;
		V2d velocity;
	};
	MyData data;

	SoundInfo *floatSound;
	int cycleLimit;
	int shootLimit;
	int hitLimit;
	Tileset *ts;
	CubicBezier risingBez;
	CubicBezier fallingBez;

	Shroom *shroom;
	
	ShroomJelly(Shroom *shr);
	~ShroomJelly();
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void ProcessState();
	void UpdateEnemyPhysics();
	//void ProcessHit();
	void ComboHit();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif