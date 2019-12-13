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

	Action action;
	int actionLength[A_Count];
	int animFactor[A_Count];
	//MovementSequence testSeq;
	SwingLauncher(GameSession *owner,
		sf::Vector2i &pos, sf::Vector2i &other, int speed,
		bool cw);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	sf::SoundBuffer *launchSoundBuf;

	void UpdateSprite();
	void DirectKill();

	bool clockwiseLaunch;

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

	double swingRadius;

	int animationFactor;

	sf::Vector2<double> dir;
	int speed;
	int stunFrames;

	sf::Vertex debugLine[2];
	sf::Text debugSpeed;

	V2d anchor;
};

#endif