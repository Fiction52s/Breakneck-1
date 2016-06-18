#ifndef __BOSS_H__
#define __BOSS_H__

#include "Actor.h"
#include <list>
#include "Mover.h"
#include "Movement.h"
#include "Enemy.h"

struct Boss_Crawler : Enemy, LauncherEnemy,
	SurfaceMoverHandler
{
	enum Action
	{
		SHOOT,
		BOOST
	};
	int frameTest;

	Boss_Crawler( GameSession *owner, Edge *ground, 
		double quantity );
	void ActionEnded();
	int NumTotalBullets();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void ResetEnemy();
	
	void SaveEnemyState();
	void LoadEnemyState();

	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );

	void HitTerrainAerial(Edge *, double);
	void TransferEdge( Edge * );

	void HitTerrain( double &q );
	bool StartRoll();
	void FinishedRoll();

	bool GetClockwise( int index );
	double GetDistanceClockwise( int index );
	double GetDistanceCCW( int index );

	void SetDirs();

	double totalDistanceAround;
	bool leftFirstEdge;
	Edge *firstEdge;

	Launcher *launcher;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;
	Tileset *ts;

	sf::VertexArray markerVA;

	//std::list<sf::Drawable*> progressDraw;

	bool onTargetEdge;
	//Tileset *ts_walk;
	//Tileset *ts_roll;
	struct EdgeInfo
	{

		Edge *edge;
		double quantity;
		//dont worry about rolling etc
		//for the boss cuz of the special room
	};
	
	int shootIndex;
	int bulletDirIndex[5];
	EdgeInfo bulletHits[5];
	int bulletIndex;
	int travelIndex;
	int numBullets;

	Action action;
	bool facingRight;

	//CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	//CrawlerReverser *lastReverser;
	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	SurfaceMover *mover;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	//CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	sf::Vector2<double> tempVel;
	//sf::Vector2<double> gravity;
	
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

struct Boss_Bird : Enemy, LauncherEnemy
{
	enum Action
	{
		PLANMOVE,
		MOVE,
		SHOOT
	};

	int testFrame;

	Boss_Bird( GameSession *owner, sf::Vector2i pos );
	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void ActionEnded();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void SaveEnemyState();
	void LoadEnemyState();
	void CreatePath();
	bool DirIsValid( sf::Vector2i &testIndex,
		sf::Vector2i &testDir );

	sf::Vector2i moveIndex;
	const sf::Vector2i DOWN;
	const sf::Vector2i LEFT;
	const sf::Vector2i RIGHT;
	const sf::Vector2i UP;
	//sf::Vector2i moveDir;
	bool moveX;
	
	const static int MAX_PATH_SIZE = 10;
	sf::Vector2i path[MAX_PATH_SIZE];
	int pathSize;

	const static int GRID_SIZE = 6;
	double gridRatio;
	double gridSizeRatio;

	sf::Vector2<double> gridOriginPos;

	sf::VertexArray pathVA;

	CubicBezier moveBez;


	std::map<Action,int> actionLength;
	std::map<Action,int> animFactor;

	int bulletSpeed;

	Action action;
	//sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	
	sf::Vector2i originalPos;
	int frame;

	Launcher *launcher;

	int fireCounter;

	bool dying;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	//int animationFactor;

	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
	bool facingRight;

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

