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

	enum AimLauncherType
	{
		TYPE_BOUNCE,
		TYPE_GRIND,
		TYPE_HOMING,
	};

	AimLauncherType aimLauncherType;

	void UpdateParamsSettings();
	void AddToWorldTrees();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);
	void SetCurrDir(V2d &newDir);

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

	V2d origDir;
	V2d currDir;

	int speed;
	int stunFrames;
	double dist;

	sf::Vertex bounceQuads[3 * 4];



	sf::Vertex debugLine[2];
	sf::Text debugSpeed;

	V2d dest;
};

#endif