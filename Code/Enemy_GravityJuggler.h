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

	GravityJuggler(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, 
		std::list<sf::Vector2i> &path, int p_level,
		int juggleReps, bool reverse );
	~GravityJuggler();
	void HandleEntrant(QuadTreeEntrant *qte);
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

	void Throw(double a, double strength);
	void Throw(V2d vel);


	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];

	V2d origPos;

	V2d velocity;

	bool reversedGrav;

	double gravFactor;
	V2d gDir;
	double maxFallSpeed;

	int hitLimit;
	int currHits;

	sf::Sprite sprite;
	Tileset *ts;

	bool reversed;

	int juggleReps;
	int currJuggle;

	V2d *guidedDir;

	int waitFrame;
	int maxWaitFrames;
};

#endif