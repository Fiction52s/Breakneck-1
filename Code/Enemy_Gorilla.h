#ifndef __ENEMY_GORILLA_H__
#define __ENEMY_GORILLA_H__

#include "Enemy.h"

struct Gorilla : Enemy
{
	enum Action
	{
		WAKEUP,
		ALIGN,
		FOLLOW,
		ATTACK,
		RECOVER,
		Count
	};

	Action action;
	int actionLength[Count];
	int animFactor[Count];

	
	double latchStartAngle;

	Gorilla(GameSession *owner, bool hasMonitor,
		sf::Vector2i &pos, int level);

	void ProcessState();
	void UpdateEnemyPhysics();

	void ActionEnded();

	void EnemyDraw(sf::RenderTarget *target);
	
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();

	bool origFacingRight;

	int awakeFrames;
	int awakeCap;

	V2d basePos;

	int alignMoveFrames;
	int createWallFrame;
	int alignFrames;
	int followFrames;
	int recoveryLoops;
	int recoveryCounter;

	double wallWidth;

	double acceleration;
	double speed;

	int approachFrames;
	int totalFrame;

	sf::Sprite sprite;
	Tileset *ts;

	Tileset *ts_wall;
	sf::Sprite wallSprite;

	CollisionBox wallHitbox;
	int wallHitboxWidth;
	int wallHitboxHeight;
	double idealRadius;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	bool facingRight;

	CubicBezier approachAccelBez;

	V2d offsetPlayer;
	V2d origPosition;
	V2d origOffset;
	bool latchedOn;
};

#endif