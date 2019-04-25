#ifndef __ENEMY_PATROLLER_H__
#define __ENEMY_PATROLLER_H__

#include "Enemy.h"

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

	Patroller(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, std::list<sf::Vector2i> &path, bool loop, int speed);
	~Patroller();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
	void ProcessState();
	void HandleHitAndSurvive();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void HandleNoHealth();
	void AdvanceTargetNode();

	int turnFrame;
	int turnAnimFactor;
	bool currFacingRight;

	int aimingFrames;
	int maxAimingFrames;


	float targetAngle;
	V2d targetPos;
	float currentAngle;
	float beakTurnSpeed;
	sf::SoundBuffer *shootSound;

	Action action;
	int fireCounter;
	int actionLength[S_Count];
	int animFactor[S_Count];
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;
	int targetNode;
	bool forward;
	double acceleration;
	double speed;
	int nodeWaitFrames;
	sf::Sprite sprite;

	sf::Vertex bodyVA[4 * 2];
	//sf::Vertex bodyAuraVA[4 * 2];
	Tileset *ts;
	Tileset *ts_aura;
	bool facingRight;
};

#endif