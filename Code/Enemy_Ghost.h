#ifndef __ENEMY_GHOST_H__
#define __ENEMY_GHOST_H__

#include "Enemy.h"

struct Ghost : Enemy
{
	enum Action
	{
		WAKEUP,
		APPROACH,
		BITE,
		EXPLODE,
		Count
	};


	double detectionRadius;
	Action action;
	int actionLength[Count];
	int animFactor[Count];

	double latchStartAngle;
	MovementSequence testSeq;
	Ghost(GameSession *owner, bool hasMonitor,
		sf::Vector2i &pos, int p_level );
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void Bite();

	bool origFacingRight;
	int awakeFrames;
	int awakeCap;

	bool latchedOn;
	V2d basePos;

	double acceleration;
	double speed;

	bool facingRight;

	int approachFrames;
	int totalFrame;
	sf::Vector2<double> origOffset;

	sf::Sprite sprite;
	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	CubicBezier approachAccelBez;

	V2d offsetPlayer;
	V2d origPosition;
	
};

#endif