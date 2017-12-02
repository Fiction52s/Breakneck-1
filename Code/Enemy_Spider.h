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
		double quantity, int speed);
	void ActionEnded();
	int NumTotalBullets();
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *target);
	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
	bool ResolvePhysics(sf::Vector2<double> vel);
	void ResetEnemy();

	void SaveEnemyState();
	void LoadEnemyState();

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
	Launcher *testLaunch;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;
	Tileset *ts;
	//Tileset *ts_walk;
	//Tileset *ts_roll;

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

	CollisionBox hurtBody;
	CollisionBox hitBody;
	//CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	HitboxInfo *laserInfo0;
	HitboxInfo *laserInfo1;
	HitboxInfo *laserInfo2;
	HitboxInfo *laserInfo3;
	int laserCounter;


	sf::Vector2<double> tempVel;
	sf::Vector2<double> gravity;

	int attackFrame;
	int attackMult;

	//double rollFactor;
	Contact minContact;
	bool col;
	std::string queryMode;

	int possibleEdgeCount;

	Edge *startGround;
	double startQuant;
	//sf::Vector2<double> offset;
	int frame;
	//bool roll;

	int deathFrame;
	int crawlAnimationFactor;
	int rollAnimationFactor;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;

	double maxGroundSpeed;
	double maxFallSpeed;
};

#endif