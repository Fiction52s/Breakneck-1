#ifndef __ENEMY_BOUNCEJUGGLER_H__
#define __ENEMY_BOUNCEJUGGLER_H__

#include "Enemy.h"

struct ComboObject;

struct BounceJuggler : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		S_FLOAT,
		S_FLY,
		S_BOUNCE,
		S_RETURN,
		S_Count
	};

	BounceJuggler(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos,
		std::list<sf::Vector2i> &path, int p_level,
		int juggleReps );
	~BounceJuggler();
	void HitTerrainAerial(Edge * edge, double quant);
	//void HitTerrainAerial(Edge *, double);

	void HandleEntrant(QuadTreeEntrant *qte);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void HandleNoHealth();
	CollisionBox &GetEnemyHitbox();
	void Move();
	void Return();
	void Pop();
	void PopThrow();

	void Throw(double a, double strength);
	void Throw(V2d vel);


	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];

	ComboObject *comboObj;

	V2d origPos;

	V2d velocity;

	int hitLimit;
	int currHits;

	sf::Sprite sprite;
	Tileset *ts;

	int juggleReps;
	int currJuggle;

	V2d *guidedDir;

	int waitFrame;
	int maxWaitFrames;

	SurfaceMover *mover;
};

#endif