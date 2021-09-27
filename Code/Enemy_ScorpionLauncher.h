#ifndef __ENEMY_SCORPIONLAUNCHER_H__
#define __ENEMY_SCORPIONLAUNCHER_H__

#include "Enemy.h"

struct ScorpionLauncher : Enemy
{
	enum Action
	{
		IDLE,
		SPRINGING,
		RECOVERING,
		A_Count
	};

	ScorpionLauncher(ActorParams *ap);

	void UpdateParamsSettings();
	void AddToWorldTrees();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);
	bool CountsForEnemyGate() { return false; }

	
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	sf::SoundBuffer *launchSoundBuf;

	void UpdateSprite();
	void DirectKill();

	void ResetEnemy();
	void ActionEnded();
	void Launch();
	Tileset *ts;


	//Tileset *ts_idle;
	//Tileset *ts_recover;
	//Tileset *ts_springing;
	void DebugDraw(sf::RenderTarget *target);

	V2d dir;
	int speed;

	sf::Vertex debugLine[2];
	sf::Text debugSpeed;

	int tilesetChoice;
	int startFrame;
	int recoverTileseChoice;
	int recoverStartFrame;

	int numFrames;
};

#endif