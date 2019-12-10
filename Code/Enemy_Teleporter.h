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
		RECEIVERECOVERING,
		A_Count
	};

	Action action;
	int actionLength[A_Count];
	int animFactor[A_Count];
	//MovementSequence testSeq;
	Teleporter(GameSession *owner,
		sf::Vector2i &pos, sf::Vector2i &other, bool bothWays,
		bool secondary = false);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	//sf::SoundBuffer *launchSoundBuf;

	void UpdateSprite();
	void DirectKill();
	Teleporter *CreateSecondary();

	void ResetEnemy();
	void ActionEnded();
	void Teleport();
	sf::Sprite sprite;
	Tileset *ts_idle;
	Tileset *ts_recover;
	Tileset *ts_springing;
	CollisionBody * hurtBody;
	CollisionBody* hitBody;

	int animationFactor;
	bool goesBothWays;

	sf::Vector2<double> dir;
	int speed;
	int stunFrames;

	bool secondary;
	Teleporter *otherTele;

	V2d dest;
};

#endif