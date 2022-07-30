#ifndef __ENEMY_BAT_H__
#define __ENEMY_BAT_H__

#include "Enemy.h"
#include "Bullet.h"

struct Bat : Enemy, LauncherEnemy
{
	enum Action : int32
	{
		FLY,
		RETREAT,
		RETURN,
		A_Count
	};

	enum Visual : int32
	{
		FLAP,
		KICK,
		V_Count
	};

	struct MyData : StoredEnemyData
	{
		Visual currVisual;
		int visFrame;
		int framesSinceBothered;
		int fireCounter;
	};
	MyData data;

	BasicPathFollower pathFollower; //not used rn

	Tileset *ts_bulletExplode;

	V2d currBasePos;
	V2d retreatPos;
	V2d startRetreatPos;

	

	double acceleration;
	double speed;
	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	
	int visualLength[V_Count];
	int visualMult[V_Count];
	

	int bulletSpeed;
	int framesBetween;

	MovementSequence testSeq;
	MovementSequence retreatSeq;
	MovementSequence returnSeq;
	LineMovement *retreatMove;
	LineMovement *returnMove;
	WaitMovement *retreatWait;
	
	Bat(ActorParams *ap );

	void SetActionEditLoop();
	void SetLevel(int lev);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(
		int playerIndex, 
		BasicBullet *b,
		int hitResult);
	void ProcessState();
	bool physicsOver;
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	//void DebugDraw(sf::RenderTarget *target);
	void UpdateSprite();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif