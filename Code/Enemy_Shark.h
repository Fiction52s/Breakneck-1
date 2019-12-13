#ifndef __ENEMY_SHARK_H__
#define __ENEMY_SHARK_H__

#include "Enemy.h"

struct Shark : Enemy
{
	enum Action
	{
		WAKEUP,
		APPROACH,
		CIRCLE,
		FINALCIRCLE,
		RUSH,
		Count
	};
	Action action;

	int circleCounter;

	int wakeCounter;
	int wakeCap;

	int circleFrames;
	double attackAngle;

	int actionLength[Count];
	int animFactor[Count];

	double latchStartAngle;
	MovementSequence circleSeq;
	MovementSequence rushSeq;
	Shark(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, int p_level );

	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();

	sf::Vector2<double> basePos;

	V2d GetCircleOffset();

	sf::Color testColor; //for temp anim

	double acceleration;
	double speed;

	V2d origOffset;
	V2d attackOffset;

	sf::Sprite sprite;
	Tileset *ts_circle;
	Tileset *ts_bite;
	Tileset *ts_death;

	bool facingRight;

	CubicBezier approachAccelBez;

	V2d offsetPlayer;
	V2d origPosition;
	bool latchedOn;
};


#endif