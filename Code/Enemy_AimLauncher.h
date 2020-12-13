#ifndef __ENEMY_AIMLAUCHER_H__
#define __ENEMY_AIMLAUCHER_H__

#include "Enemy.h"

struct AimLauncher : Enemy
{
	enum Action
	{
		IDLE,
		AIMING,
		LAUNCHING,
		RECOVERING,
		A_Count
	};

	void UpdateParamsSettings();
	void AddToWorldTrees();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);

	AimLauncher(ActorParams *ap);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	sf::SoundBuffer *launchSoundBuf;

	void StartAiming();
	void UpdateSprite();
	void DirectKill();

	void ResetEnemy();
	void ActionEnded();
	void Launch();
	Tileset *ts_idle;
	Tileset *ts_recover;
	Tileset *ts_springing;
	void DebugDraw(sf::RenderTarget *target);

	int animationFactor;

	sf::Vector2<double> dir;
	int speed;
	int stunFrames;
	double dist;

	sf::Vertex debugLine[2];
	sf::Text debugSpeed;

	V2d dest;
};

#endif