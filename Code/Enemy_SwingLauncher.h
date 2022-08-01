#ifndef __ENEMY_SWINGLAUNCHER_H__
#define __ENEMY_SWINGLAUNCHER_H__

#include "Enemy.h"

struct SwingLauncher : Enemy
{
	enum Action
	{
		IDLE,
		SWINGING,
		RECOVERING,
		A_Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	SoundInfo *launchSoundBuf;
	Tileset *ts_idle;
	Tileset *ts_recover;
	Tileset *ts_springing;

	double swingRadius;

	int animationFactor;

	V2d dir;
	int speed;
	int stunFrames;

	sf::Vertex debugLine[2];
	sf::Text debugSpeed;
	bool clockwiseLaunch;
	V2d anchor;

	SwingLauncher(ActorParams *ap );
	bool CountsForEnemyGate() { return false; }
	void ProcessState();
	void AddToWorldTrees();
	void UpdateParamsSettings();
	void UpdatePath();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void DirectKill();
	void ResetEnemy();
	void ActionEnded();
	void Launch();
	void DebugDraw(sf::RenderTarget *target);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif