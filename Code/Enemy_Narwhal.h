#ifndef __ENEMY_NARWHAL_H__
#define __ENEMY_NARWHAL_H__

#include "Enemy.h"

struct Narwhal : Enemy
{
	enum Action
	{
		WAITING,
		CHARGE_START,
		CHARGE_REPEAT,
		TURNING,
		Count
	};

	Narwhal(GameSession *owner, bool hasMonitor,
		sf::Vector2i &startPos,
		sf::Vector2i &endPos, int moveFrames);
	//void HandleEdge( Edge *e );
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	bool triggered;
	bool start0;
	CollisionBox triggerBox;
	MovementSequence seq;
	//MovementSequence seq1;
	float angle;

	void SetupWaiting();

	sf::Vector2<double> origStartPoint;
	sf::Vector2<double> origEndPoint;
	double moveDistance;
	sf::Vector2<double> startCharge;
	sf::Vector2<double> moveDir;


	void ActionEnded();


	void SaveEnemyState();
	void LoadEnemyState();

	Action action;
	int actionLength[Action::Count];
	int animFactor[Action::Count];
	int moveFrames;
	int currMoveFrame;

	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	//Tileset * ts_death;
	//std::list<sf::Vector2i> path;

	//sf::Vector2<double> point1;
	//sf::Vector2<double> point0;

	int frame;

	sf::VertexArray pathVA;
	void UpdatePath();


	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;

	bool facingRight;
	int animationFactor;

	//bool facingRight;

	struct Stored
	{
		bool dead;
		int deathFrame;
		//sf::Vector2<double> deathVector;
		//double deathPartingSpeed;
		int targetNode;
		bool forward;
		int frame;
		sf::Vector2<double> position;

		int hitlagFrames;
		int hitstunFrames;
	};
	Stored stored;
};

#endif