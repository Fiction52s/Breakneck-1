#ifndef __ENEMY_AIRDASHER_H__
#define __ENEMY_AIRDASHER_H__

#include "Enemy.h"

struct Airdasher : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_CHARGE,
		S_DASH,
		S_OUT,
		S_RETURN,
		S_COMBO,
		S_Count
	};

	Airdasher(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, int level );
	~Airdasher();
	void ProcessState();

	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	double SetFacingPlayerAngle();
	void SetFacingSide( V2d pDir );
	void ComboHit();
	void IHitPlayer(int index = 0);

	int hitLimit;
	int currHits;

	V2d playerDir;
	V2d velocity;

	ComboObject *comboObj;

	int hitFrame;


	int chargeFrames;
	int maxCharge;

	CubicBezier dashBez;
	CubicBezier returnBez;

	double dashRadius;
	int dashFrames;
	int returnFrames;

	Action action;
	int physStepIndex;
	V2d origPos;
	V2d currOrig;
	int actionLength[S_Count];
	int animFactor[S_Count];
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;
	int targetNode;
	bool forward;
	double acceleration;
	double speed;
	int nodeWaitFrames;
	sf::Sprite sprite;
	Tileset *ts;
	bool facingRight;
};

#endif