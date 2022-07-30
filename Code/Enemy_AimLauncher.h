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
		TYPE_AIRBOUNCE,
		TYPE_GRIND,
		TYPE_HOMING,
	};

	struct MyData : StoredEnemyData
	{
		V2d currDir;
	};
	MyData data;

	AimLauncherType aimLauncherType;

	Tileset *ts;
	Tileset *ts_recover;
	Tileset *ts_particles;
	Tileset *ts_boost;

	sf::Sprite particleSprite;
	sf::Sprite recoverSprite;
	sf::Sprite boostSprite;

	int animationFactor;

	V2d origDir;
	

	int speed;
	int stunFrames;
	double dist;

	sf::Vertex bounceQuads[3 * 4];
	sf::Vertex debugLine[2];
	sf::Text debugSpeed;

	V2d dest;
	SoundInfo *launchSoundBuf;

	int tilesetChoice;
	int startFrame;
	int recoverTileseChoice;
	int recoverStartFrame;

	void UpdateParamsSettings();
	void AddToWorldTrees();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);
	void SetCurrDir(V2d &newDir);
	bool CountsForEnemyGate() { return false; }

	AimLauncher(ActorParams *ap);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	

	void StartAiming();
	void UpdateSprite();
	void DirectKill();

	void ResetEnemy();
	void ActionEnded();
	void Launch();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	

	void DebugDraw(sf::RenderTarget *target);
};

#endif