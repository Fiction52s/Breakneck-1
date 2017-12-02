#ifndef __ENEMY_BADGER_H__
#define __ENEMY_BADGER_H__

#include "Enemy.h"

struct Badger : Enemy, GroundMoverHandler
{
	enum Action
	{
		RUN,
		LEDGEJUMP,
		SHORTJUMP,
		SHORTJUMPSQUAT,
		TALLJUMP,
		TALLJUMPSQUAT,
		ATTACK,
		LAND,
		Count
	};

	Badger(GameSession *owner,
		bool hasMonitor,
		Edge *ground,
		double quantity,
		bool clockwise, int speed,
		int jumpStrength);
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
	//bool ResolvePhysics( sf::Vector2<double> vel );
	void ResetEnemy();
	void Jump(double strengthx,
		double strengthy);
	void UpdateNextAction();

	void SaveEnemyState();
	void LoadEnemyState();

	void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();

	Launcher *testLaunch;
	Action landedAction;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;
	Tileset *ts;
	//Tileset *ts_walk;
	//Tileset *ts_roll;

	Action action;
	bool facingRight;
	double angle;

	Action nextAction;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	int actionLength[Action::Count];
	int animFactor[Action::Count];


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
	bool originalFacingRight;
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

	double jumpStrength;
};


#endif