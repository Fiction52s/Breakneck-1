#ifndef __ENEMY_TELEPORTER_H__
#define __ENEMY_TELEPORTER_H__

#include "Enemy.h"

struct Teleporter : Enemy
{
	enum Action
	{
		IDLE,
		TELEPORTING,
		RECOVERING,
		RECEIVING,
		RECEIVE_RECOVERING,
		A_Count
	};

	void AddToWorldTrees();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);
	void AddToGame();
	//void UpdateParamsSettings();

	Teleporter(ActorParams *ap);
	~Teleporter();

	void CreateSecondary(ActorParams *ap);
	void ReceivePlayer();
	bool TryTeleport();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	sf::SoundBuffer *launchSoundBuf;

	void UpdateSprite();
	void DirectKill();

	void ResetEnemy();
	void ActionEnded();
	Tileset *ts_idle;
	Tileset *ts_recover;
	Tileset *ts_springing;
	void DebugDraw(sf::RenderTarget *target);

	int animationFactor;

	V2d dir;
	int speed;
	int stunFrames;
	double dist;

	sf::Vertex debugLine[2];

	bool goesBothWays;
	bool secondary;
	bool teleportedPlayer;

	V2d dest;

	TeleporterParams *otherParams;
	Teleporter *otherTele;
};

#endif