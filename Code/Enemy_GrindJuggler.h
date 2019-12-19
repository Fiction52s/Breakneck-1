#ifndef __ENEMY_GRINDJUGGLER_H__
#define __ENEMY_GRINDJUGGLER_H__

#include "Enemy.h"

struct ComboObject;
struct Rail;

struct GrindJuggler : Enemy, SurfaceRailMoverHandler
{
	enum Action
	{
		S_FLOAT,
		S_FLY,
		S_RAILGRIND,
		S_GRIND,
		S_RETURN,
		S_Count
	};

	GrindJuggler(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos,
		int p_level,
		int juggleReps,
		bool cw);
	~GrindJuggler();
	void HitTerrainAerial(Edge * edge, double quant);
	//void HitTerrainAerial(Edge *, double);

	void HandleEntrant(QuadTreeEntrant *qte);
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void ComboKill(Enemy *e);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void Move();
	void Return();
	void Pop();
	void PopThrow();
	void ExtraQueries(sf::Rect<double> &r);

	void LeaveRail();
	void BoardRail();
	bool CanLeaveRail();

	Rail *currRail;
	Edge *railEdge;
	double railQuant;
	double railSpeed;

	void Throw(double a, double strength);
	void Throw(V2d vel);


	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];

	V2d origPos;

	V2d velocity;

	int hitLimit;
	int currHits;

	sf::Sprite sprite;
	Tileset *ts;

	int juggleReps;
	int currJuggle;

	double flySpeed;
	int waitFrame;
	int maxWaitFrames;

	SurfaceRailMover *mover;
	//SurfaceMover *mover;

	bool clockwise;
};

#endif