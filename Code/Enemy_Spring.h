#ifndef __ENEMY_SPRING_H__
#define __ENEMY_SPRING_H__

#include "Enemy.h"

struct Spring : Enemy
{
	enum Action
	{
		IDLE,
		SPRINGING,
		RECOVERING,
		A_Count
	};

	enum SpringType
	{
		TYPE_REGULAR,
		TYPE_GLIDE,
		TYPE_BOUNCE,
		TYPE_AIRBOUNCE,
		TYPE_GRIND,
		//TYPE_HOMING,
		TYPE_ANNIHILATION_GLIDE,
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts;
	Tileset *ts_recover;
	Tileset *ts_particles;
	Tileset *ts_boost;

	V2d dir;
	int speed;
	int stunFrames;
	double dist;

	sf::Vertex debugLine[2];
	sf::Text debugSpeed;
	V2d dest;

	sf::Sprite particleSprite;
	sf::Sprite recoverSprite;
	sf::Sprite boostSprite;

	int tilesetChoice;
	int startFrame;
	int recoverTileseChoice;
	int recoverStartFrame;

	int numFrames;
	SpringType springType;

	Spring(ActorParams *ap);
	void UpdateParamsSettings();
	void AddToWorldTrees();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);
	bool CountsForEnemyGate() { return false; }
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	bool IsHomingTarget() { return false; }
	SoundInfo *launchSoundBuf;
	
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