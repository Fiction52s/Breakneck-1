#ifndef __ENEMY_AIRDASHJUGGLER_H__
#define __ENEMY_AIRDASHJUGGLER_H__

#include "Enemy.h"

struct ComboObject;

struct AirdashJuggler : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_DASH,
		S_RETURN,
		S_EXPLODE,
		S_Count
	};

	AirdashJuggler(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, int p_level );
	//Comboer(GameSession *owner, std::ifstream &is);

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

	ComboObject *comboObj;

	V2d origPos;

	V2d startDashPos;
	V2d endDashPos;
	CubicBezier dashBez;

	int dashFrames;

	V2d velocity;

	double dashDist;

	int hitLimit;
	int currHits;

	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];

	int dashStep;
	int maxDashSteps;

	double speed;
	sf::Sprite sprite;
	Tileset *ts;

	int dashReps;
	int currDash;
};

#endif