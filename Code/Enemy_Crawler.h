#ifndef __ENEMY_CRAWLER_H__
#define __ENEMY_CRAWLER_H__

#include "Enemy.h"

struct Crawler : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		LATENT,
		UNBURROW,
		CRAWL,
		STARTROLL,
		ROLL,
		ENDROLL,
		DASH,
		BURROW,
		DYING
	};

	int actionLength[DYING+1];
	Crawler(GameSession *owner, bool hasMonitor, Edge *ground, double quantity, bool clockwise, int speed, int dist);

	void TransferEdge(Edge *);
	//	void HandleEdge( Edge *e );
	void HandleEntrant(QuadTreeEntrant *qte);
	bool ShouldDash();
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
	sf::Sprite sprite;
	double totalDistBeforeBurrow;
	double currDistTravelled;
	Tileset *ts;
	//Tileset *ts_walk;
	//Tileset *ts_roll;

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