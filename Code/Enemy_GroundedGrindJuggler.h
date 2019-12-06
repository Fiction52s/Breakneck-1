#ifndef __ENEMY_GROUNDGRINDJUGGLER_H__
#define __ENEMY_GROUNDGRINDJUGGLER_H__

#include "Enemy.h"

struct ComboObject;

struct GroundedGrindJuggler : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		S_IDLE,
		S_GRIND,
		S_SLOW,
		S_STOPPED,
		S_RETURN,
		S_Count
	};

	GroundedGrindJuggler(GameSession *owner, bool hasMonitor,
		Edge *ground,
		double quantity,
		int p_level,
		int juggleReps,
		bool cw);

	~GroundedGrindJuggler();

	void HandleEntrant(QuadTreeEntrant *qte);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void ComboKill(Enemy *e);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void HandleNoHealth();
	CollisionBox &GetEnemyHitbox();
	void Move();
	void Return();

	void Push(double strength);
	//void Pop();
	//void PopThrow();

	//void Throw(double a, double strength);
	//void Throw(V2d vel);


	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];

	ComboObject *comboObj;

	//V2d origPos;
	Edge *startGround;
	double startQuant;

	V2d velocity;

	int hitLimit;
	int currHits;

	sf::Sprite sprite;
	Tileset *ts;

	int juggleReps;
	int currJuggle;

	int waitFrame;
	int maxWaitFrames;

	double friction;
	double pushStart;

	SurfaceMover *mover;

	bool clockwise;
};

#endif