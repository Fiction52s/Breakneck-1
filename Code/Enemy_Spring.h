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
		BLUE,
		GREEN,
		BOUNCE,
		AIRBOUNCE,
		TELEPORT,
	};

	void AddToWorldTrees();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);

	SpringType springType;
	Spring(ActorParams *ap);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	sf::SoundBuffer *launchSoundBuf;
	
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

	sf::Vertex debugLine[2];
	sf::Text debugSpeed;

	V2d dest;
};

#endif