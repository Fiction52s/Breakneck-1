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
		//TYPE_HOMING,
		TYPE_ANNIHILATION_GLIDE,
	};

	void UpdateParamsSettings();
	void AddToWorldTrees();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);
	bool CountsForEnemyGate() { return false; }

	SpringType springType;
	Spring(ActorParams *ap);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	SoundInfo *launchSoundBuf;
	
	void UpdateSprite();
	void DirectKill();
	
	void ResetEnemy();
	void ActionEnded();
	void Launch();
	Tileset *ts;
	Tileset *ts_recover;
	Tileset *ts_particles;
	Tileset *ts_boost;

	
	//Tileset *ts_idle;
	//Tileset *ts_recover;
	//Tileset *ts_springing;
	void DebugDraw(sf::RenderTarget *target);

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
};

#endif