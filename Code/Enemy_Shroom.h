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
	};

	Action action;
	int actionLength[HITTING + 1];
	int animFactor[HITTING + 1];

	Shroom(ActorParams * ap);/*bool hasMonitor,
		Edge *ground, double quantity, int level);*/
	~Shroom();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void HandleNoHealth();
	void DirectKill();
	void ResetEnemy();
	void ProcessState();

	void CheckedMiniDraw(sf::RenderTarget *target,
		sf::FloatRect &rect);
	void SetZoneSpritePosition();

	sf::SoundBuffer *hitSound;

	sf::Sprite sprite;
	sf::Sprite auraSprite;
	Tileset *ts;
	Tileset *ts_aura;

	Edge *ground;
	double edgeQuantity;

	double angle;

	sf::Vector2<double> gn;

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

	Action action;
	int actionLength[A_Count];
	int animFactor[A_Count];

	ShroomJelly(ActorParams *ap, V2d &pos,
		int level );
	~ShroomJelly();
	void EnemyDraw(sf::RenderTarget *target);
	
	void UpdateSprite();
	void ResetEnemy();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ProcessHit();
	void ComboHit();

	sf::Sprite sprite;
	sf::Sprite auraSprite;
	Tileset *ts;
	Tileset *ts_aura;
	
	sf::SoundBuffer *floatSound;

	V2d orig;

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
};

#endif