#ifndef __ENEMY_STAGBEETLE_H__
#define __ENEMY_STAGBEETLE_H__

#include "Enemy.h"

struct StagBeetle : Enemy, GroundMoverHandler
{
	enum Action
	{
		IDLE,
		RUN,
		JUMP,
		//ATTACK,
		LAND,
		Count
	};

	StagBeetle(GameSession *owner, bool hasMonitor,
		Edge *ground,
		double quantity,
		bool clockwise, double speed);
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
	bool ResolvePhysics(sf::Vector2<double> vel);
	void ResetEnemy();
	//void DirectKill();
	void SaveEnemyState();
	void LoadEnemyState();

	void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();

	Launcher *testLaunch;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;

	Tileset *ts;
	Tileset *ts_death;
	Tileset *ts_hop;
	Tileset *ts_idle;
	Tileset *ts_run;
	Tileset *ts_sweep;
	Tileset *ts_walk;

	//Tileset *ts_walk;
	//Tileset *ts_roll;

	Action action;
	bool facingRight;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;


	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	GroundMover *testMover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();

	CollisionBox hurtBody;
	CollisionBox hitBody;
	//CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	sf::Vector2<double> tempVel;
	sf::Vector2<double> gravity;


	int actionLength[Action::Count];
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
	double angle;
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