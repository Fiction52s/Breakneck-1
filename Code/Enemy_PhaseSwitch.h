#ifndef __ENEMY_PHASE_SWITCH_H__
#define __ENEMY_PHASE_SWITCH_H__

#include "Enemy.h"

struct PhaseSwitch : Enemy
{
	enum Action
	{
		A_PHASE_OFF,
		A_SWITCHING_ON,
		A_PHASE_ON,
		A_SWITCHING_OFF,
		A_Count
	};

	enum TargetType
	{
		TARGET_BLUE,
		TARGET_GLIDE,
		TARGET_SCORPION,
		TARGET_FREEFLIGHT,
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

	
	Tileset *ts;	

	bool CountsForEnemyGate() { return false; }
	PhaseSwitch(ActorParams *ap);
	void SetLevel(int lev);
	void ProcessState();
	void UpdateSprite();
	void ResetEnemy();
	void AddToWorldTrees();
	void ProcessHit();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif