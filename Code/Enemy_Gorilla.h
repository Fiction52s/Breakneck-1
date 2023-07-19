#ifndef __ENEMY_GORILLA_H__
#define __ENEMY_GORILLA_H__

#include "Enemy.h"

struct Gorilla : Enemy
{
	enum Action
	{
		IDLE,
		FOLLOW,
		ATTACK,
		RECOVER,
		//NEUTRAL,
		//WAKEUP,
		//ALIGN,
		//FOLLOW,
		//ATTACK,
		//RECOVER,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int currWallHitboxesBodyID;	
		CollisionBody wallHitBody;
		V2d velocity;
	};
	MyData data;


	CollisionBody *currWallHitboxes;

	
	int createWallFrame;
	
	int followFrames;

	

	HitboxInfo *wallHitboxInfo;

	double wallWidth;

	double acceleration;
	double speed;

	int approachFrames;
	

	Tileset *ts;

	Tileset *ts_wall;
	sf::Sprite wallSprite;

	int wallHitboxWidth;
	int wallHitboxHeight;
	double idealRadius;
	double wallAmountCloser;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	CubicBezier approachAccelBez;

	Gorilla(ActorParams *ap);
	~Gorilla();

	void SetLevel(int lev);
	//void UpdateHitboxes();
	void ProcessState();
	void UpdateEnemyPhysics();

	void ActionEnded();

	void EnemyDraw(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	
	void UpdateSprite();
	void ResetEnemy();
	bool CheckHitPlayer(int index = 0);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif