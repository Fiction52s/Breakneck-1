#ifndef __ENEMY_COMBOER_TARGET_H__
#define __ENEMY_COMBOER_TARGET_H__

#include "Enemy.h"

struct ComboerTarget : Enemy
{
	enum Action
	{
		A_IDLE,
		A_DYING,
		A_Count
	};

	enum TargetType
	{
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
		int keyFrame;
	};
	MyData data;

	int targetType;
	Tileset *ts;
	sf::Sprite keyObjectSprite;
	int keyIdleLength;
	
	int keyAnimFactor;

	bool CountsForEnemyGate() { return false; }
	ComboerTarget(ActorParams *ap);
	bool IsInteractible();
	bool IsValidTrackEnemy();
	bool IsHomingTarget();
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void AddToWorldTrees();
	void Collect();
	void ProcessHit();
	HitboxInfo * IsHit(int pIndex);
	void FrameIncrement();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif