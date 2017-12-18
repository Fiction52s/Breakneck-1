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

	HitboxInfo * IsHit(Actor *player);
	int actionLength[DYING+1];
	Crawler(GameSession *owner, bool hasMonitor, Edge *ground, double quantity, bool clockwise, int speed, int framesUntilBurrow );
	void SetActionDash();
	void ProcessState();
	bool TryDash();
	void HandleNoHealth();
	void IncrementFrame();
	bool IsPlayerChasingMe();
	void AttemptRunAwayBoost();
	double dashAccel;
	void Accelerate(double amount);
	void SetForwardSpeed( double speed );
	void TransferEdge(Edge *);
	//	void HandleEdge( Edge *e );
	void HandleEntrant(QuadTreeEntrant *qte);
	bool ShouldDash();
	bool PlayerInFront();
	void UpdatePhysics();
	void DrawMinimap(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *target);
	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	bool IsSlowed();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void ResetEnemy();
	//void DirectKill();
	void SaveEnemyState();
	void LoadEnemyState();
	sf::Sprite sprite;
	double totalDistBeforeBurrow;
	double currDistTravelled;
	Tileset *ts;
	//Tileset *ts_walk;
	//Tileset *ts_roll;

	int maxFramesUntilBurrow;
	int framesUntilBurrow;

	bool clockwise;
	double groundSpeed;
	Edge *ground;

	SurfaceMover *mover;
	//sf::Vector2<double> offset;
	double edgeQuantity;

	Action action;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	CollisionBox physBody;
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
	sf::Vector2<double> offset;
	int frame;
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