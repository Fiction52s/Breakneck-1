#ifndef __ENEMY_POISONFROG_H__
#define __ENEMY_POISONFROG_H__

#include "Enemy.h"

struct PoisonFrog : Enemy, GroundMoverHandler
{
	enum Action
	{

		STAND,
		JUMPSQUAT,
		STEEPJUMP,
		JUMP,
		LAND,
		WALLCLING,
		//STUNNED,
		Count
	};

	//struct Bullet
	//{
	//	Bullet();
	//	//Bullet *prev;
	//	//Bullet *next;
	//	sf::Vector2<double> position;
	//	sf::Vector2<double> velocity;
	//	CollisionBox hurtBody;
	//	CollisionBox hitBody;
	//	CollisionBox physBody;
	//	bool active;
	//	int frame;
	//	int slowCounter;
	//	int slowMultiple;
	//	
	//	//int maxFramesToLive;
	//	//int framesToLive;
	//};

	PoisonFrog(GameSession *owner,
		bool hasMonitor,
		Edge *ground, double quantity,
		int gravFactor,
		sf::Vector2i &jumpStrength,
		int jumpFramesWait);

	int actionLength[Action::Count];
	int animFactor[Action::Count];
	//void DirectKill();


	Tileset *ts_test;
	//int queryIndex;

	//sf::VertexArray bulletVA;
	Action action;
	int frame;
	double gravity;
	bool facingRight;
	sf::Vector2<double> velocity;
	double angle;

	int hitsBeforeHurt;
	int hitsCounter;
	int invincibleFrames;

	//double jumpStrength;
	double xSpeed;
	int jumpFramesWait;
	double gravityFactor;
	bool steepJump;
	sf::Vector2<double> jumpStrength;

	GroundMover *mover;


	//int wallTouchCounter;
	//sf::Vector2<double> position;


	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void ActionEnded();
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

	void SaveEnemyState();
	void LoadEnemyState();
	void UpdatePhysics2();
	void UpdatePhysics3();



	void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();

	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();

	//void FireBullets();
	//void UpdateBulletSprites();
	//void UpdateBulletHitboxes();


	sf::Sprite sprite;
	Tileset *ts_walk;
	Tileset *ts_roll;


	//double groundSpeed;
	//Edge *ground;

	//double edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	//CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	sf::Vector2<double> tempVel;

	double maxFallSpeed;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	Edge *startGround;
	double startQuant;
	sf::Vector2<double> offset;




	bool dead;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
};

#endif