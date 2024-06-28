#ifndef _KIN_UPGRADES_H__
#define _KIN_UPGRADES_H__


const static int MAX_KIN_POWERS = 32; //won't get up to this number, but its nice for 4 byte spacing
enum KinUpgrades
{
	POWER_AIRDASH,
	POWER_GRAV,
	POWER_BOUNCE,
	POWER_GRIND,
	POWER_TIME,
	POWER_RWIRE,
	POWER_LWIRE,

	//W1
	UPGRADE_W1_DECREASE_ENEMY_DAMAGE = MAX_KIN_POWERS,
	UPGRADE_W1_INCREASE_ENEMY_REGEN,
	UPGRADE_W1_INCREASE_ENEMY_MOMENTUM,
	UPGRADE_W1_INCREASE_STARTING_MOMENTUM,

	UPGRADE_W1_DASH_BOOST,
	UPGRADE_W1_STEEP_CLIMB_1,
	UPGRADE_W1_STEEP_SLIDE_1,
	UPGRADE_W1_PASSIVE_GROUND_1,
	UPGRADE_W1_SPRINT_1,
	UPGRADE_W1_BASE_DASH_1,

	UPGRADE_W1_EMPTY_1,
	UPGRADE_W1_EMPTY_2,
	UPGRADE_W1_EMPTY_3,
	UPGRADE_W1_EMPTY_4,
	UPGRADE_W1_EMPTY_5,
	UPGRADE_W1_EMPTY_6,

	//W2
	UPGRADE_W2_DECREASE_ENEMY_DAMAGE = MAX_KIN_POWERS + 64 * 1,
	UPGRADE_W2_INCREASE_ENEMY_REGEN,
	UPGRADE_W2_INCREASE_ENEMY_MOMENTUM,
	UPGRADE_W2_INCREASE_STARTING_MOMENTUM,

	UPGRADE_W2_AIRDASH_BOOST,
	UPGRADE_W2_STEEP_CLIMB_2,
	UPGRADE_W2_STEEP_SLIDE_2,
	UPGRADE_W2_PASSIVE_GROUND_2,
	UPGRADE_W2_SPRINT_2,
	UPGRADE_W2_BASE_AIRDASH_1,

	UPGRADE_W2_EMPTY_1,
	UPGRADE_W2_EMPTY_2,
	UPGRADE_W2_EMPTY_3,
	UPGRADE_W2_EMPTY_4,
	UPGRADE_W2_EMPTY_5,
	UPGRADE_W2_EMPTY_6,

	//W3
	UPGRADE_W3_DECREASE_ENEMY_DAMAGE = MAX_KIN_POWERS + 64 * 2,
	UPGRADE_W3_INCREASE_ENEMY_REGEN,
	UPGRADE_W3_INCREASE_ENEMY_MOMENTUM,
	UPGRADE_W3_INCREASE_STARTING_MOMENTUM,

	UPGRADE_W3_CEILING_DROP_AERIAL,
	UPGRADE_W3_CEILING_STEEP_CLIMB_1,
	UPGRADE_W3_CEILING_STEEP_SLIDE_1,
	UPGRADE_W3_CEILING_PASSIVE_GROUND_1,
	UPGRADE_W3_CEILING_SPRINT_1,
	UPGRADE_W3_BASE_DASH_2,

	UPGRADE_W3_EMPTY_1,
	UPGRADE_W3_EMPTY_2,
	UPGRADE_W3_EMPTY_3,
	UPGRADE_W3_EMPTY_4,
	UPGRADE_W3_EMPTY_5,
	UPGRADE_W3_EMPTY_6,

	//W4
	UPGRADE_W4_DECREASE_ENEMY_DAMAGE = MAX_KIN_POWERS + 64 * 3,
	UPGRADE_W4_INCREASE_ENEMY_REGEN,
	UPGRADE_W4_INCREASE_ENEMY_MOMENTUM,
	UPGRADE_W4_INCREASE_STARTING_MOMENTUM,

	UPGRADE_W4_SCORPION_ATTACK,
	UPGRADE_W4_SCORPION_JUMP,
	UPGRADE_W4_SCORPION_DOUBLE_JUMP,
	UPGRADE_W4_SCORPION_BOUNCE,
	UPGRADE_W4_CEILING_STEEP_CLIMB_2,
	UPGRADE_W4_CEILING_STEEP_SLIDE_2,
	UPGRADE_W4_CEILING_PASSIVE_GROUND_2,
	UPGRADE_W4_CEILING_SPRINT_2,

	UPGRADE_W4_EMPTY_1,
	UPGRADE_W4_EMPTY_2,
	UPGRADE_W4_EMPTY_3,
	UPGRADE_W4_EMPTY_4,

	//W5
	UPGRADE_W5_DECREASE_ENEMY_DAMAGE = MAX_KIN_POWERS + 64 * 4,
	UPGRADE_W5_INCREASE_ENEMY_REGEN,
	UPGRADE_W5_INCREASE_ENEMY_MOMENTUM,
	UPGRADE_W5_INCREASE_STARTING_MOMENTUM,

	UPGRADE_W5_GRIND_LUNGE,
	UPGRADE_W5_SLOW_RESISTANCE,
	UPGRADE_W5_CEILING_STEEP_CLIMB_3,
	UPGRADE_W5_CEILING_STEEP_SLIDE_3,
	UPGRADE_W5_CEILING_PASSIVE_GROUND_3,
	UPGRADE_W5_CEILING_SPRINT_3,
	UPGRADE_W5_BASE_AIRDASH_2,

	UPGRADE_W5_EMPTY_1,
	UPGRADE_W5_EMPTY_2,
	UPGRADE_W5_EMPTY_3,
	UPGRADE_W5_EMPTY_4,
	UPGRADE_W5_EMPTY_5,

	//W6
	UPGRADE_W6_DECREASE_ENEMY_DAMAGE = MAX_KIN_POWERS + 64 * 5,
	UPGRADE_W6_INCREASE_ENEMY_REGEN,
	UPGRADE_W6_INCREASE_ENEMY_MOMENTUM,
	UPGRADE_W6_INCREASE_STARTING_MOMENTUM,

	UPGRADE_W6_BUBBLE_AIRDASH,
	UPGRADE_W6_BUBBLE_SIZE,
	UPGRADE_W6_EXTRA_BUBBLES_1,
	UPGRADE_W6_EXTRA_BUBBLES_2,
	UPGRADE_W6_BASE_DASH_3,
	UPGRADE_W6_BASE_AIRDASH_3,
	UPGRADE_W6_STEEP_CLIMB_3,
	UPGRADE_W6_STEEP_SLIDE_3,
	UPGRADE_W6_PASSIVE_GROUND_3,
	UPGRADE_W6_SPRINT_3,

	UPGRADE_W6_EMPTY_1,
	UPGRADE_W6_EMPTY_2,


	UPGRADE_Count,
};


#endif