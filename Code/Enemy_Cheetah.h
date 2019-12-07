#ifndef __ENEMY_CHEETAH_H__
#define __ENEMY_CHEETAH_H__

#include "Enemy.h"

struct Cheetah : Enemy, GroundMoverHandler
{
	enum Action
	{
		IDLE,
		CHARGE,
		BOOST,
		Count
	};

	Cheetah(GameSession *owner,
		bool hasMonitor,
		Edge *ground,
		double quantity,
		int level);
	void ActionEnded();
	void ProcessState();
	void UpdateEnemyPhysics();

	void HandleNoHealth();

	void EnemyDraw(sf::RenderTarget *target);

	void UpdateSprite();

	void UpdateHitboxes();

	void ResetEnemy();
	void Jump(double strengthx,
		double strengthy);

	//void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();

	Action landedAction;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;
	Tileset *ts;
	//Tileset *ts_walk;
	//Tileset *ts_roll;

	Action action;
	bool facingRight;

	Action nextAction;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	int actionLength[Action::Count];
	int animFactor[Action::Count];


	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	GroundMover *mover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();

	//CollisionBox physBody;
	sf::Vector2<double> tempVel;
	sf::Vector2<double> gravity;

	int attackFrame;
	int attackMult;




	Edge *startGround;
	double startQuant;
	//sf::Vector2<double> offset;
	bool originalFacingRight;
	//bool roll;

	int crawlAnimationFactor;
	int rollAnimationFactor;

	double maxGroundSpeed;
	double maxFallSpeed;

	double jumpStrength;

	Tileset *ts_aura;
	sf::Sprite auraSprite;
};


#endif