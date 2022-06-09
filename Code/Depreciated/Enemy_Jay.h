#ifndef __ENEMY_JAY_H__
#define __ENEMY_JAY_H__

#include "Enemy.h"

struct Jay : Enemy
{
	enum Action
	{
		PROTECTED,
		WAITTOFIRE,
		FIRE,
		RECOVER,
		SHUTDOWN,
		Count
	};

	int redHealth;
	int blueHealth;

	bool dying;

	Jay(GameSession *owner, bool hasMonitor,
		sf::Vector2i &startPos,
		sf::Vector2i &endPos);
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
	//std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();


	bool triggered;

	CollisionBox triggerBox;

	MovementSequence seq;
	//MovementSequence seq1;
	float angle;

	sf::Vector2<double> redPos;
	sf::Vector2<double> bluePos;
	sf::Vector2<double> redNodePos;

	sf::Vector2<double> origStartPoint;
	sf::Vector2<double> origEndPoint;
	sf::Vector2<double> origDiff;
	double moveDistance;
	sf::Vector2<double> startCharge;
	sf::Vector2<double> moveDir;
	sf::Vector2<double> wallVel;

	std::pair<bool, bool> PlayerHitRed();
	std::pair<bool, bool> PlayerHitBlue();
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	void ActionEnded();


	void SaveEnemyState();
	void LoadEnemyState();

	Action action;
	int actionLength[Action::Count];
	int animFactor[Action::Count];
	int moveFrames;
	int currMoveFrame;

	//int redDeathFrame;
	//int blueDeathFrame;
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
	int shieldFrame;
	int wallFrame;
	int wallDuration;
	int wallAnimFactor;

	sf::VertexArray pathVA;
	void UpdatePath();
	void SetupWall();

	int numWallTiles;
	float remainder;
	double wallTileWidth;
	bool hitRed;

	//sf::Sprite sprite;

	sf::VertexArray jayVA;
	sf::VertexArray shieldVA;
	sf::VertexArray *wallVA;
	sf::Vector2f *localWallPoints;

	void UpdateJays();
	void SetupJays();

	void UpdateWall();

	bool PlayerSlowingWall();
	int slowCounterWall;
	int slowMultipleWall;


	Tileset *ts;
	Tileset *ts_shield;
	Tileset *ts_wall;
	CollisionBox redHurtBody;
	CollisionBox redHitBody;
	CollisionBox blueHurtBody;
	CollisionBox blueHitBody;
	HitboxInfo *hitboxInfo;
	HitboxInfo *wallHitboxInfo;
	HitboxInfo *shieldHitboxInfo;
	CollisionBox wallHitBody;
	CollisionBox wallNodeHitboxRed;
	CollisionBox wallNodeHitboxBlue;

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