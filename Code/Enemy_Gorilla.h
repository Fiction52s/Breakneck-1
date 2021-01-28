#ifndef __ENEMY_GORILLA_H__
#define __ENEMY_GORILLA_H__

#include "Enemy.h"

struct Gorilla : Enemy
{
	enum Action
	{
		NEUTRAL,
		WAKEUP,
		ALIGN,
		FOLLOW,
		ATTACK,
		RECOVER,
		A_Count
	};

	
	double latchStartAngle;
	bool origFacingRight;

	V2d basePos;

	int alignMoveFrames;
	int createWallFrame;
	int alignFrames;
	int followFrames;

	int physStepIndex;

	CollisionBody wallHitBody;
	CollisionBody *currWallHitboxes;
	HitboxInfo *wallHitboxInfo;

	double wallWidth;

	double acceleration;
	double speed;

	int approachFrames;
	int totalFrame;

	Tileset *ts;

	Tileset *ts_wall;
	sf::Sprite wallSprite;

	//CollisionBox wallHitbox;
	int wallHitboxWidth;
	int wallHitboxHeight;
	double idealRadius;
	double wallAmountCloser;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	bool facingRight;

	CubicBezier approachAccelBez;

	V2d offsetPlayer;
	V2d origOffset;
	bool latchedOn;

	Gorilla(ActorParams *ap);
	~Gorilla();

	void SetLevel(int lev);
	//void UpdateHitboxes();
	void ProcessState();
	void UpdateEnemyPhysics();

	void ActionEnded();

	void EnemyDraw(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	
	void UpdateSprite();
	void ResetEnemy();
	bool CheckHitPlayer(int index = 0);

	
};

#endif