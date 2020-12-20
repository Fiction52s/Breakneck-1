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

	bool limitedKills;
	sf::Text numKilledText;

	Rail *currRail;
	Edge *railEdge;
	double railQuant;
	double railSpeed;

	V2d velocity;

	int hitLimit;
	int currHits;

	Tileset *ts;

	int maxKilled;
	int numKilled;

	double flySpeed;
	int waitFrame;
	int maxWaitFrames;

	bool clockwise;

	GrindJuggler(ActorParams *ap);
	~GrindJuggler();
	void SetLevel(int lev);
	void UpdateParamsSettings();
	void HitTerrainAerial(Edge * edge, double quant);
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
	void UpdateKilledNumberText(int reps);

	void LeaveRail();
	void BoardRail();
	bool CanLeaveRail();

	void Throw(double a, double strength);
	void Throw(V2d vel);
};

#endif