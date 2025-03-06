#ifndef _KIN_UPGRADES_H__
#define _KIN_UPGRADES_H__


#include "BitField.h"

const static int MAX_KIN_POWERS = 32; //won't get up to this number, but its nice for 4 byte spacing
enum KinUpgrades
{
	POWER_AIRDASH,
	POWER_GRAV,
	POWER_BOUNCE,
	POWER_GRIND,
	POWER_TIME,
	POWER_DOUBLE_WIRES,

	ABILITY_DASH,
	//Skills + Upgrades
	//SKILL_DASH_BOOST_1,
	//SKILL_AIRDASH_BOOST_1,
	//SKILL_GRIND_LUNGE_1,
	//UPGRADE_W3_CEILING_DROP_AERIAL,
	//UPGRADE_W4_SCORPION_ATTACK,
	//UPGRADE_W4_SCORPION_JUMP,
	//UPGRADE_W4_SCORPION_DOUBLE_JUMP,
	//UPGRADE_W4_SCORPION_BOUNCE,
	//UPGRADE_W5_SLOW_RESISTANCE,
	//UPGRADE_W6_BUBBLE_AIRDASH,
	//UPGRADE_W6_BUBBLE_SIZE,
	//UPGRADE_W6_EXTRA_BUBBLES_1,

	//MOVEMENT
	UPGRADE_STEEP_CLIMB,
	UPGRADE_STEEP_SLIDE,
	UPGRADE_PASSIVE_GROUND,
	UPGRADE_SPRINT,

	//COMBAT
	UPGRADE_DECREASE_ENEMY_DAMAGE,
	UPGRADE_INCREASE_ENEMY_REGEN,
	UPGRADE_INCREASE_ENEMY_MOMENTUM,
	UPGRADE_INCREASE_STARTING_MOMENTUM,

	UPGRADE_Count,
};

struct UpgradeLevels
{
	//0 means you dont have it, higher levels is higher level upgrades

	unsigned char upgradeLevel[UPGRADE_Count];

	UpgradeLevels();
	void Clear();
	int GetUpgradeLevel(int up); //return the level of the upgrade
	bool HasUpgradeLevel(int up, int lvl); //returns true if you have that upgrade level or higher
	void SetUpgradeLevel(int up, int lvl);
	void Set(UpgradeLevels *ul);

	bool Load(std::istream &is);
	void Save(std::ofstream &of);
	bool LoadBinary(std::istream &is);
	void SaveBinary(std::ofstream &of);
};


#endif