#ifndef __ENEMY_DIMENSION_EYE_H__
#define __ENEMY_DIMENSION_EYE_H__

#include "Enemy.h"
#include "Movement.h"

struct GameSession;
struct GreySkeleton;

struct GreyEye : Enemy
{
	enum Action
	{
		BASE_WORLD_IDLE,
		DIMENSION_WORLD_IDLE,
		IDLE,
		DIE,
		A_Count
	};

	enum EyeType
	{
		EYE_RED,
		EYE_Count,
	};

	EyeType eyeType;
	Tileset *ts;
	GameSession *myBonus;
	bool isBonusEye;
	GreySkeleton *greySkel;

	GreyEye(int eyeType,
		GreySkeleton *gs );
	~GreyEye();
	void Setup();
	bool CountsForEnemyGate() { return false; }
	void ProcessState();
	void HandleHitAndSurvive();
	void EnemyDraw(sf::RenderTarget *target);
	void ProcessHit();
	void UpdateSprite();
	void ResetEnemy();
	void Appear( V2d &pos );
	void UpdateEnemyPhysics();
};

#endif