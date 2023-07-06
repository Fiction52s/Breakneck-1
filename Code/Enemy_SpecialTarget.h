#ifndef __ENEMY_SPECIALTARGET_H__
#define __ENEMY_SPECIALTARGET_H__

#include "Enemy.h"

struct SpecialTarget : Enemy
{
	enum Action
	{
		A_IDLE,
		A_DYING,
		A_WAIT_BEFORE_REGEN,
		A_REGENERATING,
		A_Count
	};

	enum TargetType
	{
		TARGET_BLUE,
		TARGET_GLIDE,
		TARGET_SCORPION,
		TARGET_FREEFLIGHT,
		TARGET_BOUNCE,
		TARGET_GRIND,
		TARGET_COMBOER_BLUE,
		TARGET_COMBOER_GREEN,
		TARGET_COMBOER_YELLOW,
		TARGET_COMBOER_ORANGE,
		TARGET_COMBOER_RED,
		TARGET_COMBOER_MAGENTA,
		TARGET_COMBOER_GREY,
		//TARGET_ATTACK_BLUE,
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	int targetType;
	Tileset *ts;
	bool regenOn;

	bool CountsForEnemyGate() { return false; }
	SpecialTarget(ActorParams *ap);
	bool IsInteractible();
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void AddToWorldTrees();
	void Collect();
	void ProcessHit();
	HitboxInfo * IsHit(int pIndex);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif