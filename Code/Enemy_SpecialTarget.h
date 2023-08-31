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
		TARGET_GLIDE,
		TARGET_BOUNCE,
		TARGET_SCORPION,
		TARGET_GRIND,
		TARGET_HOMING,
		TARGET_SWING,
		TARGET_FREEFLIGHT,
	};

	struct MyData : StoredEnemyData
	{
		int keyFrame;
	};
	MyData data;

	int targetType;
	Tileset *ts;
	bool regenOn;

	sf::Sprite keyObjectSprite;
	int keyIdleLength;
	int keyAnimFactor;

	bool CountsForEnemyGate() { return false; }
	SpecialTarget(ActorParams *ap);
	bool IsInteractible();
	bool IsValidTrackEnemy();
	bool IsHomingTarget();
	void FrameIncrement();
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