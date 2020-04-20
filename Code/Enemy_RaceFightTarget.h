#ifndef __ENEMY_RACEFIGHTTARGET_H__
#define __ENEMY_RACEFIGHTTARGET_H__

#include "Enemy.h"

struct RaceFightTarget : Enemy
{
	enum Action
	{
		NEUTRAL,
		PLAYER1,
		PLAYER2,
		PLAYER3,
		PLAYER4,
		Count
	};

	
	
	RaceFightTarget(ActorParams *ap);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void HandleHitAndSurvive();

	void UpdateSprite();
	
	void ResetEnemy();


	RaceFightTarget *pPrev;
	RaceFightTarget *pNext;
	RaceFightTarget *p2Prev;
	RaceFightTarget *p2Next;

	int gameTimeP1Hit;
	int gameTimeP2Hit;

	Action action;

	sf::Sprite sprite;
	Tileset *ts;

	//CollisionBox hurtBody;
	//CollisionBox hitBody;
	//HitboxInfo *hitboxInfo;

	//int hitlagFrames;
	//int hitstunFrames;
	int animationFactor;
};

#endif