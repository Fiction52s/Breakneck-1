#ifndef __ENEMY_DIMENSION_EYE_H__
#define __ENEMY_DIMENSION_EYE_H__

#include "Enemy.h"
#include "Movement.h"

struct GameSession;

struct GreyEye : Enemy
{
	enum Action
	{
		BASE_WORLD_IDLE,
		DIMENSION_IDLE,
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

	GreyEye(int eyeType);
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