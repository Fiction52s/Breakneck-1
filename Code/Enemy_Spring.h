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

	Action action;
	int actionLength[A_Count];
	int animFactor[A_Count];
	//MovementSequence testSeq;
	Spring(GameSession *owner,
		sf::Vector2i &pos, sf::Vector2i &other, int moveFrames);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	sf::SoundBuffer *launchSoundBuf;
	
	void UpdateSprite();
	
	void ResetEnemy();
	void ActionEnded();
	void Launch();
	sf::Sprite sprite;
	Tileset *ts_idle;
	Tileset *ts_recover;
	Tileset *ts_springing;
	CollisionBody * hurtBody;
	CollisionBody* hitBody;
	void DebugDraw(sf::RenderTarget *target);
	//HitboxInfo *hitboxInfo;

	int animationFactor;

	sf::Vector2<double> dir;
	int speed;
	int stunFrames;

	sf::Vertex debugLine[2];
	sf::Text debugSpeed;
};

#endif