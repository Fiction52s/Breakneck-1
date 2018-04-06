#ifndef __ENEMY_CRAWLER_H__
#define __ENEMY_CRAWLER_H__

#include "Enemy.h"

struct CrawlerQueen : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		UNBURROW,
		BURROW,
		STAND,
		SHOOT,
		BOOST,
		JUMP,
		BURROWEDTHROW,
		DYING
	};

	int actionLength[DYING + 1];
	CrawlerQueen(GameSession *owner, bool hasMonitor, Edge *ground, double quantity, bool clockwise );
	void ProcessState();
	void HandleNoHealth();
	void FrameIncrement();
	double dashAccel;
	void Accelerate(double amount);
	void SetForwardSpeed(double speed);
	void TransferEdge(Edge *);

	bool ShouldDash();
	bool PlayerInFront();
	void EnemyDraw(sf::RenderTarget *target);
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