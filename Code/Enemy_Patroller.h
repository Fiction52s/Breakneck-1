#ifndef __ENEMY_PATROLLER_H__
#define __ENEMY_PATROLLER_H__

#include "Enemy.h"
#include "Bullet.h"

struct PatrollerEye;
struct Patroller : Enemy, LauncherEnemy
{
	enum Action
	{
		S_FLAP,
		S_BEAKOPEN,
		S_BEAKHOLDOPEN,
		S_BEAKCLOSE,
		S_Count
	};

	PatrollerEye *eye;

	void UpdateOnPlacement(ActorParams *ap);
	//static Enemy *Create(ActorParams *ap);
	Patroller(ActorParams *ap);//bool hasMonitor,
		//sf::Vector2i pos, std::list<sf::Vector2i> &path, bool loop, int speed);
	~Patroller();
	void UpdatePath();
	void SetLevel(int lev);

	sf::FloatRect GetAABB();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(
		int playerIndex, 
		BasicBullet *b,
		int hitResult);
	void ProcessState();
	void HandleHitAndSurvive();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateHitboxes();
	void HandleNoHealth();

	int turnFrame;
	int turnAnimFactor;

	int aimingFrames;
	int maxAimingFrames;


	float targetAngle;
	V2d targetPos;
	float currentAngle;
	float beakTurnSpeed;
	sf::SoundBuffer *shootSound;

	int fireCounter;
	BasicPathFollower pathFollower;
	double acceleration;
	double speed;

	sf::Vertex bodyVA[4 * 2];
	//sf::Vertex bodyAuraVA[4 * 2];
	Tileset *ts;
	Tileset *ts_aura;
	bool facingRight;
};

#endif