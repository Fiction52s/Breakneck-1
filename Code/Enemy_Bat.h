#ifndef __ENEMY_BAT_H__
#define __ENEMY_BAT_H__

#include "Enemy.h"

struct Bat : Enemy, LauncherEnemy
{
	enum Action
	{
		FLY,
		RETREAT,
		RETURN,
	};


	Action action;
	MovementSequence testSeq;
	MovementSequence retreatSeq;
	MovementSequence returnSeq;
	LineMovement *retreatMove;
	LineMovement *returnMove;
	WaitMovement *retreatWait;
	int framesSinceBothered;
	Bat(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, std::list<sf::Vector2i> &path,
		int bulletSpeed,
		//int nodeDistance,
		int framesBetween,
		bool loop);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
	void ProcessState();
	bool physicsOver;
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	
	//void DebugDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	int bulletSpeed;
	//int nodeDistance;
	int framesBetween;

	Tileset *ts_bulletExplode;

	sf::Vector2i *path; //global
	int pathLength;
	bool loop;
	int frame;
	V2d startPos;
	V2d currBasePos;
	V2d retreatPos;
	V2d startRetreatPos;

	int fireCounter;

	double acceleration;
	double speed;
	int nodeWaitFrames;
	sf::Sprite sprite;
	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;
	bool facingRight;
};

#endif