#ifndef __ENEMY_COMBOERTARGET_H__
#define __ENEMY_COMBOERTARGET_H__

#include "Enemy.h"

struct ComboerTarget : Enemy
{
	enum Action
	{
		S_NEUTRAL,
		S_Count
	};

	enum TargetType
	{
		BLUE,
		GREEN,
		YELLOW,
	};

	ComboerTarget(ActorParams *ap);
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void AddToWorldTrees();
	void Collect();
	//bool CanBeHitByPlayer();
	//bool CanBeHitByComboer();
	HitboxInfo *IsHit(int pIndex);
	TargetType targetType;
	Tileset *ts;

};

#endif