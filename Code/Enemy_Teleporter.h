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

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

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

	Tileset *ts;
	Tileset *ts_recover;
	Tileset *ts_particles;
	Tileset *ts_boost;

	sf::Sprite particleSprite;
	sf::Sprite recoverSprite;
	sf::Sprite boostSprite;

	int tilesetChoice;
	int startFrame;
	int recoverTileseChoice;
	int recoverStartFrame;

	Teleporter(ActorParams *ap);
	~Teleporter();

	bool CountsForEnemyGate() { return false; }
	void AddToWorldTrees();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);
	void AddToGame();
	//void UpdateParamsSettings();

	

	void CreateSecondary(ActorParams *ap);
	void ReceivePlayer();
	bool TryTeleport();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	SoundInfo *launchSoundBuf;

	void UpdateSprite();
	void DirectKill();

	void ResetEnemy();
	void ActionEnded();
	void DebugDraw(sf::RenderTarget *target);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	
};

#endif