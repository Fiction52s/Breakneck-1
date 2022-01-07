#ifndef __ENEMY_PULSEATTACK_H__
#define __ENEMY_PULSEATTACK_H__

#include "Enemy.h"
#include <vector>
#include "MovingGeo.h"

struct PulseAttack;

struct PulseAttackPool
{
	PulseAttackPool( int numPulses);
	~PulseAttackPool();
	void Reset();
	PulseAttack * Pulse(
		int pulseType,
		V2d &pos, float pulseWidth,
		float pulseStartRadius,
		float pulseEndRadius,
		float pulseFrames,
		sf::Color startColor,
		sf::Color endColor,
		sf::Shader *sh );
	bool CanPulse();
	std::vector<PulseAttack*> pulseVec;
	int numPulses;
};

struct PulseAttack : Enemy
{
	enum PulseType
	{
		PT_PULSE_HIT,
		PT_Count,
	};

	enum Action
	{
		A_PULSING,
		A_Count
	};

	int pulseType;

	//Tileset *ts;

	//double flySpeed;
	//double maxFlySpeed;
	//double accel;
	//int framesToLive;
	//int origFramesToLive;
	//V2d velocity;

	//for node grow
	double currRadius;
	double startRadius;
	double maxRadius;

	MovingGeoGroup geoGroup;
	MovingRing *myRing;

	int growthFrames;

	PulseAttack(
		PulseAttackPool *pool);
	void UpdateEnemyPhysics();
	void Die();
	void Pulse(
		int pulseType,
		V2d &pos, float pulseWidth,
		float pulseStartRadius,
		float pulseEndRadius,
		float pulseFrames, 
		sf::Color startColor,
		sf::Color endColor,
		sf::Shader *sh);
	void SetLevel(int lev);
	void ProcessState();
	//bool CheckHitPlayer(int index = 0);
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();
	void UpdateHitboxes();
};

#endif