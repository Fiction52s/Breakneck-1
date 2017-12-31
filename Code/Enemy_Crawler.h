#ifndef __ENEMY_CRAWLER_H__
#define __ENEMY_CRAWLER_H__

#include "Enemy.h"

struct Crawler : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		UNBURROW,
		CRAWL,
		STARTROLL,
		ROLL,
		ENDROLL,
		DASH,
		BURROW,
		UNDERGROUND,
		DYING
	};

	int actionLength[DYING+1];
	Crawler(GameSession *owner, bool hasMonitor, Edge *ground, double quantity, bool clockwise, int speed, int framesUntilBurrow );
	void SetActionDash();
	void ProcessState();
	bool TryDash();
	void HandleNoHealth();
	void FrameIncrement();
	bool IsPlayerChasingMe();
	void AttemptRunAwayBoost();
	double dashAccel;
	void Accelerate(double amount);
	void SetForwardSpeed( double speed );
	void TransferEdge(Edge *);

	bool ShouldDash();
	bool PlayerInFront();
	void Draw(sf::RenderTarget *target);
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	sf::Sprite sprite;
	double totalDistBeforeBurrow;
	double currDistTravelled;
	Tileset *ts;
	bool origCW;

	int maxFramesUntilBurrow;
	int framesUntilBurrow;

	bool clockwise;
	double groundSpeed;
	Edge *ground;

	SurfaceMover *mover;
	double edgeQuantity;

	Action action;

	CollisionBody *hurtBody;
	CollisionBody *hitBody;

	HitboxInfo *hitboxInfo;
	sf::Vector2<double> tempVel;

	int attackFrame;
	int attackMult;

	double rollFactor;
	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	Edge *startGround;
	double startQuant;
	//sf::Vector2<double> offset;
	bool roll;

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
};

#endif