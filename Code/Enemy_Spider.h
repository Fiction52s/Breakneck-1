#ifndef __ENEMY_SPIDER_H__
#define __ENEMY_SPIDER_H__

#include "Enemy.h"

struct Spider : Enemy, RayCastHandler
{
	enum Action
	{
		MOVE,
		JUMP,
		ATTACK,
		LAND
	};
	struct SurfaceInfo
	{
		SurfaceInfo() {}
		SurfaceInfo(Edge *p_e, double p_q,
			sf::Vector2<double> &pos)
			:e(p_e), q(p_q), position(pos)
		{}
		Edge *e;
		double q;
		sf::Vector2<double> position;
		bool clockwiseFromCurrent;
	};

	Spider(GameSession *owner, bool hasMonitor,
		Edge *ground,
		double quantity, int p_level);
	void ActionEnded();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	void UpdateSprite();
	void UpdateHitboxes();
	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void UpdatePostPhysics();

	void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();
	void CheckClosest(Edge * e,
		sf::Vector2<double> &playerPos,
		bool right,
		double cutoffQuant);
	void SetClosestLeft();
	void SetClosestRight();

	SurfaceInfo closestPos;
	sf::Sprite sprite;
	Tileset *ts;

	Action action;
	bool facingRight;

	Edge *rcEdge;
	double rcQuantity;
	sf::Vector2<double> rayStart;
	sf::Vector2<double> rayEnd;

	bool canSeePlayer;

	int framesLaseringPlayer;
	int laserLevel;

	double laserAngle;


	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	SurfaceMover *mover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();

	//CollisionBox physBody;
	HitboxInfo *laserInfo0;
	HitboxInfo *laserInfo1;
	HitboxInfo *laserInfo2;
	HitboxInfo *laserInfo3;
	int laserCounter;


	sf::Vector2<double> tempVel;
	sf::Vector2<double> gravity;

	int attackFrame;
	int attackMult;


	Edge *startGround;
	double startQuant;

	int crawlAnimationFactor;
	int rollAnimationFactor;

	double maxGroundSpeed;
	double maxFallSpeed;
};

#endif