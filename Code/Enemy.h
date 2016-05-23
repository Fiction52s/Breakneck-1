#ifndef __ENEMY_H__
#define __ENEMY_H__


#include "Actor.h"
#include <list>
#include "Mover.h"
#include "Movement.h"


struct Zone;
struct Monitor;
//projectile shotting process

struct BasicBullet;
struct LauncherEnemy
{
	virtual void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos ) = 0;
	virtual void BulletHitPlayer( BasicBullet *b ) = 0;
};
//a step is the amount of time in a substep
//which is a tenth of a step right now i think
struct Launcher;
struct BasicBullet : QuadTreeCollider
{
	BasicBullet( int indexVA, Launcher *launcher );
	BasicBullet *prev;
	BasicBullet *next;
	sf::Vector2<double> position;
	//CollisionBox hurtBody;
	CollisionBox physBody;
	CollisionBox hitBody;
	virtual void HandleEntrant( QuadTreeEntrant *qte );
	virtual void UpdatePrePhysics();
	void Reset(
		sf::Vector2<double> &pos,
		sf::Vector2<double> &vel );
	virtual void UpdatePhysics();
	virtual void UpdateSprite();
	virtual void UpdatePostPhysics();
	virtual void ResetSprite();
	bool ResolvePhysics( 
		sf::Vector2<double> vel );
	virtual bool HitTerrain();
	void HitPlayer();
	//CollisionBox physBody;
	sf::Vector2<double> velocity;
	int slowCounter;
	int slowMultiple;
	//int maxFramesToLive;
	int framesToLive;
	//sf::VertexArray *va;
	sf::Transform transform;
	Tileset *ts;
	int index;

	int bounceCount;
	//sf::Vector2<double> tempadd;

	bool col;
	Contact minContact;
	sf::Vector2<double> gravity;
	//bool gravTowardsPlayer;
	sf::Vector2<double> tempVel;
	Launcher *launcher;
};

struct SinBullet : BasicBullet
{
	SinBullet( int indexVA, Launcher *launcher );
	void UpdatePrePhysics();
	void UpdatePhysics();

	SinBullet *prev;
	SinBullet *next;
	int slowCounter;
	CollisionBox hurtBody;
	sf::Vector2<double> tempadd;
};

//struct SwarmBullet : BasicBullet
//{
//	SwarmBullet( int indexVA,
//		Launcher *launcher );
//	void UpdatePrePhysics();
//	void UpdatePhysics();
//
//	double gravStrength;
//
//	//SwarmBullet *prev;
//	//SwarmBullet *next;
//};


//struct CurveBullet : BasicBullet
//{
//};

struct Launcher
{
	Launcher( LauncherEnemy *handler, 
		GameSession *owner,
		int numTotalBullets,
		int bulletsPerShot,
		sf::Vector2<double> position,
		sf::Vector2<double> direction,
		double angleSpread,
		int maxFramesToLive,
		bool hitTerrain = true,
		int wavelength =0,
		double amplitude =0.0 );
	Launcher( LauncherEnemy *handler,
		GameSession *owner,
		int maxFramesToLive );

	//Launcher( LauncherEnemy *handler,
	bool interactWithTerrain;
	void CapBulletVel( double speed );
	
	void Reset();
	BasicBullet *inactiveBullets;
	BasicBullet *activeBullets;
	void DeactivateBullet( BasicBullet *b );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void UpdateSprites();
	
	BasicBullet * ActivateBullet();
	int GetActiveCount();
	void Fire();
	void Fire( double gravStrength );
	virtual BasicBullet * RanOutOfBullets();
	void AddToList( BasicBullet *b,
		BasicBullet *&list );
	void SetGravity( sf::Vector2<double> &grav );
	void SetBulletSpeed( double speed );
	LauncherEnemy *handler;
	
	GameSession *owner;
	int totalBullets;
	int perShot;
	HitboxInfo *hitboxInfo;
	double bulletSpeed;
	sf::Vector2<double> position;
	double angleSpread;
	sf::Vector2<double> facingDir;
	double amplitude;
	int wavelength;
	int maxFramesToLive;
	//Launcher *next;
};





struct Enemy : QuadTreeCollider, QuadTreeEntrant
{
	enum EnemyType
	{
		BASICEFFECT,
		PATROLLER,
		CRAWLER,
		BASICTURRET,
		FOOTTRAP,
		BAT,
		STAGBEETLE,
		POISONFROG,
		CURVETURRET,
		PULSER,
		BADGER,
		CACTUS,
		OWL,
		TURTLE,
		CHEETAH,
		CORALNANOBOTS,
		CORAL_BLOCK,
		SPIDER,
		GHOST,
		OVERGROWTH,
		OVERGROWTH_TREE,
		SHARK,
		SWARM,
		COPYCAT,
		GORILLA,
		FLYINGHEAD,
		SPECTRE,
		GOAL,
		KEY,
		BOSS_CRAWLER,
		BOSS_BIRD,
		BOSS_COYOTE,
		BOSS_TIGER,
		BOSS_ALLIGATOR,
		BOSS_SKELETON,
		BOSS_BEAR,
		GATEMONITOR,
		HEALTHFLY,
		Count
	};

	
	
	Enemy( GameSession *owner, EnemyType t );
	//virtual void HandleEdge( Edge *e ) = 0;
	virtual void HandleEntrant( QuadTreeEntrant *qte ) = 0;
	virtual void UpdatePrePhysics() = 0;
	virtual void UpdatePhysics() = 0;
	virtual void UpdatePostPhysics() = 0;
	virtual void Draw( sf::RenderTarget *target) = 0;
	virtual void DrawMinimap( sf::RenderTarget *target ){};
	virtual bool IHitPlayer() = 0;
	virtual void UpdateHitboxes() = 0;
	virtual std::pair<bool,bool> PlayerHitMe() = 0;
	virtual bool PlayerSlowingMe() = 0;
	virtual void DebugDraw(sf::RenderTarget *target) = 0;
	virtual int NumTotalBullets();
	void SaveState();
	void LoadState();
	void AttemptSpawnMonitor();
	virtual void SaveEnemyState() = 0;
	virtual void LoadEnemyState() = 0;
	void Reset();
	virtual void ResetEnemy() = 0;
	Enemy *prev;
	Enemy *next;
	GameSession *owner;
	bool spawned;
	sf::Rect<double> spawnRect;
	HitboxInfo *receivedHit;
	int slowMultiple;
	int slowCounter;
	EnemyType type;
	bool spawnedByClone;
	int initHealth;
	int health;
	sf::Vector2<double> position;
	Zone *zone;
	Monitor *monitor;
	bool dead;
	bool suppressMonitor;



	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );

	struct Stored
	{
		bool dead;
		Enemy *prev;
		Enemy *next;
		bool spawned;		
		HitboxInfo *receivedHit;
		int slowMultiple;
		int slowCounter;
	};
	Stored stored;
};

//set up the movers first

struct BasicEffect : Enemy
{
	BasicEffect ( GameSession *owner );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	void Init( Tileset *ts, 
		sf::Vector2<double> position, 
		double angle, int frameCount,
		int animationFactor, bool right );
	int frameCount;
	sf::Sprite s;
	Tileset *ts;
	int frame;
	bool pauseImmune;
	bool activated;
	int animationFactor;
	bool facingRight;

	int stored_frame;
	//sf::Vector2<double> position;
};

struct Patroller : Enemy
{
	Patroller( GameSession *owner, sf::Vector2i pos, std::list<sf::Vector2i> &path, bool loop, int speed );
	//void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;

	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void AdvanceTargetNode();

	void SaveEnemyState();
	void LoadEnemyState();

	
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	//Tileset * ts_death;
	//std::list<sf::Vector2i> path;
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;

	int targetNode;
	bool forward;
	//sf::Vector2<double>
	int frame;

	double acceleration;
	double speed;
	int nodeWaitFrames;
	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

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

struct Bat : Enemy, LauncherEnemy
{
	MovementSequence testSeq;
	Bat( GameSession *owner, sf::Vector2i pos, std::list<sf::Vector2i> &path,
		int bulletSpeed,
		int nodeDistance,
		int framesBetween,
		bool loop );
	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );
	//void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;

	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	//void AdvanceTargetNode();

	void SaveEnemyState();
	void LoadEnemyState();

	int bulletSpeed;
	int nodeDistance;
	int framesBetween;

	//sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	//Tileset * ts_death;
	//std::list<sf::Vector2i> path;
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;

	//int targetNode;
	//bool forward;
	//sf::Vector2<double>
	int frame;

	Launcher *launcher;

	int fireCounter;

	bool dying;

	double acceleration;
	double speed;
	int nodeWaitFrames;
	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

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

struct Pulser : Enemy
{
	Pulser( GameSession *owner, 
		sf::Vector2i &pos, 
		std::list<sf::Vector2i> &path,
		int framesBetween,
		bool loop );
	
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;

	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	//void AdvanceTargetNode();

	void SaveEnemyState();
	void LoadEnemyState();

	int framesBetween;
	MovementSequence testSeq;
	//sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	//Tileset * ts_death;
	//std::list<sf::Vector2i> path;
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;

	//int targetNode;
	//bool forward;
	//sf::Vector2<double>
	int frame;

	int nodeWaitFrames;
	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

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

struct Turtle : Enemy, LauncherEnemy
{
	enum Action
	{
		NEUTRAL,
		FIRE,
		INVISIBLE,
		FADEIN,
		FADEOUT
	};

	Turtle( GameSession *owner, sf::Vector2i pos );
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

struct Ghost : Enemy
{
	double latchStartAngle;
	MovementSequence testSeq;
	Ghost( GameSession *owner, sf::Vector2i pos, 
		float speed );

	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	//bool physicsOver;

	void UpdatePostPhysics();
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

	sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	
	int frame;

	double acceleration;
	double speed;

	int approachFrames;
	int totalFrame;
	sf::Vector2<double> origOffset;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
	bool facingRight;

	CubicBezier approachAccelBez;

	
	sf::Vector2<double> offsetPlayer;
	sf::Vector2<double> origPosition;
	//double offsetRadius;
	bool latchedOn;

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

struct CrawlerReverser : QuadTreeEntrant
{
	CrawlerReverser( GameSession* owner,
		Edge *edge, double quantity );
	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );
	void Draw( sf::RenderTarget *target );
	sf::Vector2<double> position;
	Edge *ground;
	double quantity;
	Tileset *ts;
	sf::Sprite sprite;
	CollisionBox hurtBody;
	CrawlerReverser *drawNext;
	
};

struct Crawler : Enemy
{
	Crawler( GameSession *owner, Edge *ground, double quantity, bool clockwise, double speed );
//	void HandleEdge( Edge *e );
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
	sf::Sprite sprite;
	Tileset *ts;
	//Tileset *ts_walk;
	//Tileset *ts_roll;

	bool clockwise;
	double groundSpeed;
	Edge *ground;
	//sf::Vector2<double> offset;
	double edgeQuantity;

	CrawlerReverser *lastReverser;

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

struct StagBeetle : Enemy, GroundMoverHandler
{
	enum Action
	{
		RUN,
		JUMP,
		ATTACK,
		LAND
	};

	StagBeetle( GameSession *owner, Edge *ground, 
		double quantity, 
		bool clockwise, double speed );
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

	void HitTerrain( double &q );
	bool StartRoll();
	void FinishedRoll();

	Launcher *testLaunch;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;
	Tileset *ts;
	//Tileset *ts_walk;
	//Tileset *ts_roll;

	Action action;
	bool facingRight;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	//CrawlerReverser *lastReverser;
	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	GroundMover *testMover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial();
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
};

struct Badger : Enemy, GroundMoverHandler
{
	enum Action
	{
		RUN,
		LEDGEJUMP,
		SHORTJUMP,
		TALLJUMP,
		ATTACK,
		LAND,
		Count
	};


	Badger( GameSession *owner, Edge *ground, 
		double quantity, 
		bool clockwise, int speed,
		int jumpStrength );
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
	//bool ResolvePhysics( sf::Vector2<double> vel );
	void ResetEnemy();
	void Jump( double strengthx, 
		double strengthy );
	void UpdateNextAction();
	
	void SaveEnemyState();
	void LoadEnemyState();

	void HitTerrain( double &q );
	bool StartRoll();
	void FinishedRoll();

	Launcher *testLaunch;
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

	//CrawlerReverser *lastReverser;
	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	GroundMover *testMover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial();
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

struct Cheetah : Enemy, GroundMoverHandler
{
	enum Action
	{
		NEUTRAL,
		CHARGE,
		ARRIVE,
		TURNAROUND,
		JUMP,
		ATTACK,
		LAND,
		Count
	};

	Cheetah( GameSession *owner, Edge *ground, 
		double quantity, 
		bool clockwise );
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
	//bool ResolvePhysics( sf::Vector2<double> vel );
	void ResetEnemy();
	
	void SaveEnemyState();
	void LoadEnemyState();

	void HitTerrain( double &q );
	bool StartRoll();
	void FinishedRoll();

	Launcher *testLaunch;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;
	Tileset *ts;
	//Tileset *ts_walk;
	//Tileset *ts_roll;

	Action action;
	bool facingRight;
	bool origFacingRight;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	//CrawlerReverser *lastReverser;
	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	GroundMover *testMover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial();
	void Land();

	int actionLength[Action::Count];
	int animFactor[Action::Count];

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
};

struct Owl : Enemy, LauncherEnemy
{
	enum Action
	{
		NEUTRAL,
		CHASE,
		RETREAT,
		REST,
		FIRE
	};

	Owl( GameSession *owner, sf::Vector2i &pos,
		int bulletSpeed,
		int framesBetween,
		bool facingRight );
	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );
	void ActionEnded();
	//void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
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

	Action action;
	std::map<Action,int> actionLength;
	std::map<Action,int> animFactor;

	int bulletSpeed;
	int movementRadius;
	int retreatRadius;
	int shotRadius;
	int chaseRadius;
	int framesBetween;

	CubicBezier flyingBez;

	sf::Vector2i originalPos;

	sf::Vector2<double> velocity;
	double flySpeed;
	//sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;

	//int targetNode;
	//bool forward;
	//sf::Vector2<double>
	int frame;

	Launcher *launcher;

	bool dying;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;

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

struct BasicTurret : Enemy
{
	BasicTurret( GameSession *owner, Edge *ground, double quantity, 
		double bulletSpeed,
		int framesWait );
//	void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	bool IHitPlayerWithBullets();
	std::pair<bool,bool> PlayerHitMe();
	std::pair<bool, bool> PlayerHitMyBullets();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void UpdateBulletHitboxes();


	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	sf::Sprite sprite;
	Tileset *ts;
	
	const static int maxBullets = 16;
	sf::Vector2<double> bulletPositions[maxBullets];
	sf::Vector2<double> tempVel;
	



	sf::VertexArray bulletVA;
	CollisionBox bulletHurtBody[maxBullets];
	CollisionBox bulletHitBody[maxBullets];
	struct Bullet
	{
		Bullet();
		Bullet *prev;
		Bullet *next;
		sf::Vector2<double> position;
		CollisionBox hurtBody;
		CollisionBox hitBody;
		CollisionBox physBody;
		int frame;
		int slowCounter;
		int slowMultiple;
		int maxFramesToLive;
		int framesToLive;
	};
	Bullet *queryBullet;
	bool ResolvePhysics( Bullet *b, sf::Vector2<double> vel );

	void AddBullet();
	void DeactivateBullet( Bullet *bullet );
	Bullet * ActivateBullet();
	Tileset * ts_bullet;

	Bullet *activeBullets;
	Bullet *inactiveBullets;
	HitboxInfo *bulletHitboxInfo;

	

	int framesWait;
	int firingCounter;
	Edge *ground;
	double edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	
	double angle;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	int frame;
	int deathFrame;
	int animationFactor;
	sf::Vector2<double> gn;
	double bulletSpeed;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
};

struct Cactus : Enemy, LauncherEnemy
{
	Cactus( GameSession *owner, Edge *ground, double quantity, 
		double bulletSpeed,
		int framesWait,
		sf::Vector2i &gravFactor,
		bool relativeGrav );
//	void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	bool IHitPlayerWithBullets();
	std::pair<bool,bool> PlayerHitMe();
	std::pair<bool, bool> PlayerHitMyBullets();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	//void UpdateBulletHitboxes();
	void BulletHitTerrain(BasicBullet *b, 
		Edge *edge, 
		sf::Vector2<double> &pos);
	void BulletHitPlayer( BasicBullet *b );

	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	Launcher *testLauncher;

	sf::Sprite sprite;
	Tileset *ts;
	
	const static int maxBullets = 16;
	sf::Vector2<double> tempVel;

	int framesWait;
	int firingCounter;
	Edge *ground;
	double edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	
	double angle;

	sf::Vector2<double> gravity;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	int frame;
	int deathFrame;
	int animationFactor;
	sf::Vector2<double> gn;
	double bulletSpeed;

	bool dying;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
};

struct CurveTurret : Enemy, LauncherEnemy
{
	CurveTurret( GameSession *owner, Edge *ground, double quantity, 
		double bulletSpeed,
		int framesWait,
		sf::Vector2i &gravFactor,
		bool relativeGrav );
//	void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	bool IHitPlayerWithBullets();
	std::pair<bool,bool> PlayerHitMe();
	std::pair<bool, bool> PlayerHitMyBullets();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	//void UpdateBulletHitboxes();
	void BulletHitTerrain(BasicBullet *b, 
		Edge *edge, 
		sf::Vector2<double> &pos);
	void BulletHitPlayer( BasicBullet *b );

	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	Launcher *testLauncher;

	sf::Sprite sprite;
	Tileset *ts;
	
	const static int maxBullets = 16;
	sf::Vector2<double> tempVel;

	int framesWait;
	int firingCounter;
	Edge *ground;
	double edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	
	double angle;

	sf::Vector2<double> gravity;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	int frame;
	int deathFrame;
	int animationFactor;
	sf::Vector2<double> gn;
	double bulletSpeed;

	bool dying;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
};

struct Overgrowth;
struct Tree : Enemy, LauncherEnemy
{
	Tree(Overgrowth *parent, sf::VertexArray &va,
		Tileset *ts, int index );
	void SetParams( Edge *ground, 
		double edgeQuantity );
	void ClearSprite();
	void UpdatePostPhysics();
	void UpdateSprite();


	void BulletHitTerrain(BasicBullet *b, 
		Edge *edge, 
		sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b );

	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	//void UpdateBulletHitboxes();
	//int NumTotalBullets();
	Overgrowth *parent;
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	int vaIndex;
	int frame;
	int animFactor;
	Edge *ground;
	
	bool active;
	sf::VertexArray &va;
	double edgeQuantity;
	Launcher *launcher;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	
	double angle;

	//Contact minContact;
	//bool col;
	//std::string queryMode;
	//int possibleEdgeCount;
	Tileset *ts;
	//int frame;
	int deathFrame;
	int animationFactor;
	sf::Vector2<double> gn;
	double bulletSpeed;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;

	int framesToLive;
	int maxFramesToLive;
	
	//sf::VertexArray &treeVA;
		
	//sf::Transform trans;
};

struct Overgrowth : Enemy
{
	

	Overgrowth( GameSession *owner, Edge *ground, double quantity, 
		double bulletSpeed, int lifeCycleFrames );
//	void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	//bool IHitPlayer();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void InitTrees();
	void AddTree( Tree *tree );
	int NumTotalBullets();
	void DeactivateTree( Tree *tree );
	Tree * ActivateTree( Edge *g, double q );

	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();

	int animationFactor;
	void AddToList( Tree *tree,
		Tree *&list );

	Edge *origGround;
	double origQuantity;

	Tileset *ts;
	Tree *activeTrees;
	Tree *inactiveTrees;
	//void UpdateBulletHitboxes();
	


	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	const static int MAX_TREES = 16;
	//const static int MAX_TREES = 16;
	sf::VertexArray treeVA;
};

struct CoralNanobots;
struct CoralBlock : Enemy
{
	CoralBlock(CoralNanobots *parent, 
		sf::VertexArray &va,
		Tileset *ts, int index );
	void SetParams( sf::Vector2<double> &pos,
		sf::Vector2<double> &dir,
		int iteration );
	void ClearSprite();
	void UpdatePostPhysics();
	void UpdateSprite();


	bool ResolvePhysics( sf::Vector2<double> &vel );

	void BlockHitTerrain(BasicBullet *b, 
		Edge *edge, 
		sf::Vector2<double> &pos);


	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	//void UpdateBulletHitboxes();
	//int NumTotalBullets();
	CoralNanobots *parent;
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	int vaIndex;
	int frame;
	int animFactor;
	//Edge *ground;
	
	bool active;

	bool topOpen;
	bool leftOpen;
	bool rightOpen;
	bool botOpen;

	int iteration;

	CubicBezier bez;
	sf::Vector2<double> startPos;
	sf::Vector2<double> direction;
	sf::Vector2<double> oldPos;
	MovementSequence move;
	sf::VertexArray &va;
	//double edgeQuantity;
	//Launcher *launcher;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	
	double angle;
	sf::Vector2<double> tempVel;
	sf::Vector2<double> dir;

	//Contact minContact;
	//bool col;
	//std::string queryMode;
	//int possibleEdgeCount;
	Tileset *ts;
	//int frame;
	int deathFrame;
	int animationFactor;
	sf::Vector2<double> gn;
	double bulletSpeed;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;

	bool lockedIn;

	//int framesToLive;
	//int maxFramesToLive;
	
	//sf::VertexArray &blockVA;

	Contact minContact;
	bool col;
	//sf::Transform trans;
};

struct CoralNanobots : Enemy//, LauncherEnemy
{
	CoralNanobots( GameSession *owner, 
		sf::Vector2i &pos, double speed );
	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );
//	void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	//bool IHitPlayer();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void InitBlocks();
	void AddBlock( CoralBlock *block );
	//int NumTotalBullets();
	void DeactivateBlock( CoralBlock *block );
	CoralBlock * ActivateBlock( 
		sf::Vector2<double> &pos,
		sf::Vector2<double> &dir,
		int iteration );

	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();

	int animationFactor;
	void AddToList( CoralBlock *block,
		CoralBlock *&list );

	//Edge *origGround;
	sf::Vector2<double> origPosition;
	//double origQuantity;
	


	Tileset *ts;
	CoralBlock *activeBlocks;
	CoralBlock *inactiveBlocks;

	
	//void UpdateBulletHitboxes();
	

	int blockSizeX;
	int blockSizeY;
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	const static int MAX_BLOCKS = 25;
	Launcher *launcher;
	//const static int MAX_TREES = 16;
	sf::VertexArray blockVA;
};

struct Swarm;
struct SwarmMember : Enemy
{
	SwarmMember(Swarm *parent, 
		sf::VertexArray &va, int index,
		sf::Vector2<double> &targetOffset );
	void ClearSprite();
	void UpdatePostPhysics();
	void UpdateSprite();
	void UpdatePrePhysics();
	void UpdatePhysics();
	void HandleEntrant( QuadTreeEntrant *qte );
	void PhysicsResponse();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	Swarm *parent;

	int vaIndex;
	int frame;
	int animFactor;
	sf::Vector2<double> targetOffset;

	sf::Vector2<double> velocity;
	
	
	bool active;
	//Edge *ground;
	
	//bool active;


	
	sf::VertexArray &va;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	//CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	
	//double angle;
	//sf::Vector2<double> tempVel;
	//sf::Vector2<double> dir;

	//Tileset *ts;
	//int frame;
	int deathFrame;
	int animationFactor;
	sf::Vector2<double> gn;
	double bulletSpeed;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
};

struct Swarm : Enemy
{
	Swarm( GameSession *owner, 
		sf::Vector2i &pos );	
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	//bool IHitPlayer();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();

	int animationFactor;
	int frame;

	sf::Vector2<double> origPosition;

	Tileset *ts;

	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	const static int NUM_SWARM = 5;
	sf::VertexArray swarmVA;
	sf::Sprite nestSprite;
	SwarmMember *members[NUM_SWARM];

	sf::Vector2f spriteSize;
	double maxSpeed;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;

	int deathFrame;
};

struct FootTrap : Enemy
{
	FootTrap( GameSession *owner, Edge *ground, double quantity );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();
	

	sf::Sprite sprite;
	Tileset *ts;

	Edge *ground;
	double edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	
	double angle;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	int frame;
	int deathFrame;
	int animationFactor;
	sf::Vector2<double> gn;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;

	struct Stored
	{
		bool dead;
		int deathFrame;
		int frame;
		int hitlagFrames;
		int hitstunFrames;
	};
	Stored stored;
};

struct Goal : Enemy
{
	Goal( GameSession *owner, Edge *ground, double quantity );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
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
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();
	

	sf::Sprite sprite;
	sf::Sprite miniSprite;
	Tileset *ts;
	Tileset *ts_mini;

	Edge *ground;
	double edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	
	double angle;

	//Contact minContact;
	//bool col;
	//std::string queryMode;
	//int possibleEdgeCount;

	int frame;
	int deathFrame;
	int animationFactor;
	bool dead;
	sf::Vector2<double> gn;
};

struct Key : Enemy
{
	enum KeyType
	{
		RED,
		GREEN,
		BLUE
	};

	Key( GameSession *owner, KeyType keyType, sf::Vector2i pos, std::list<sf::Vector2i> &path, bool loop, float speed, int stayFrames, bool teleport );
	//void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void AdvanceTargetNode();

	void SaveEnemyState();
	void LoadEnemyState();

	bool teleport;
	int stayFrames;

	bool dead;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	//std::list<sf::Vector2i> path;
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;

	int targetNode;
	bool forward;
	//sf::Vector2<double>
	int frame;

	double acceleration;
	double speed;
	int nodeWaitFrames;
	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
	KeyType keyType;

	struct Stored
	{
		bool dead;
		int deathFrame;
		int targetNode;
		bool forward;
		int frame;
		sf::Vector2<double> position;

		int hitlagFrames;
		int hitstunFrames;
	};
	Stored stored;
};

struct PoisonFrog : Enemy, GroundMoverHandler
{
	enum Action
	{

		STAND,
		JUMPSQUAT,
		STEEPJUMP,
		JUMP,
		LAND,
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
	
	PoisonFrog( GameSession *owner, 
		Edge *ground, double quantity,
		int gravFactor,
		sf::Vector2i &jumpStrength,
		int jumpFramesWait );

	int actionLength[Action::Count];
	int animFactor[Action::Count];

	

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

	//sf::Vector2<double> position;

	
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void ActionEnded();
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
	void UpdatePhysics2();
	void UpdatePhysics3();



	void HitTerrain( double &q );
	bool StartRoll();
	void FinishedRoll();

	void HitOther();
	void ReachCliff();
	void HitOtherAerial();
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

struct BossCrawler : Enemy
{
	enum Action
	{
		STAND,
		SHOOT,
		LUNGE,
		LUNGELAND,
		LUNGEAIR,
		RUN,
		ROLL,
		STUNNED,
		Count
	};
	
	struct Bullet
	{
		Bullet();
		//Bullet *prev;
		//Bullet *next;
		sf::Vector2<double> position;
		sf::Vector2<double> velocity;
		CollisionBox hurtBody;
		CollisionBox hitBody;
		CollisionBox physBody;
		bool active;
		int frame;
		int slowCounter;
		int slowMultiple;
		
		//int maxFramesToLive;
		//int framesToLive;
	};
	
	Tileset *ts_test;
	Tileset *ts_bullet;

	double bulletGrav;
	int bulletRadius;
	int numBullets;
	Bullet *bullets;
	int queryIndex;

	sf::VertexArray bulletVA;
	Action action;
	int frame;
	double gravity;
	bool facingRight;
	sf::Vector2<double> velocity;
	double angle;

	int hitsBeforeHurt;
	int hitsCounter;
	int invincibleFrames;
	//sf::Vector2<double> position;

	BossCrawler( GameSession *owner, Edge *ground, double quantity );
	void HandleEntrant( QuadTreeEntrant *qte );
	bool IHitPlayerWithBullets();
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
	bool ResolveBulletPhysics( int i,
		sf::Vector2<double> vel );
	void ResetEnemy();
	
	void SaveEnemyState();
	void LoadEnemyState();
	void UpdatePhysics2();
	void UpdatePhysics3();

	void FireBullets();
	void UpdateBulletSprites();
	void UpdateBulletHitboxes();


	sf::Sprite sprite;
	Tileset *ts_walk;
	Tileset *ts_roll;
	

	double groundSpeed;
	Edge *ground;
	
	double edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	HitboxInfo *bulletHitboxInfo;
	sf::Vector2<double> tempVel;
	

	double rollFactor;
	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	Edge *startGround;
	double startQuant;
	sf::Vector2<double> offset;
	
	bool roll;
	bool dead;
	int deathFrame;
	int crawlAnimationFactor;
	int rollAnimationFactor;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
	
	int maxHitsPerFrame;
	int hitsThisFrame;
	
};

struct Monitor : Enemy
{
	enum MonitorType
	{
		NONE,
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA,
		WHITE,
		Count
	};

	Monitor( GameSession *owner,
		MonitorType mType,
		Enemy *e_host );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw( sf::RenderTarget *target);
	void DrawMinimap( sf::RenderTarget *target );
	bool IHitPlayer();
	void UpdateHitboxes();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();
	void DebugDraw(sf::RenderTarget *target);
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	CollisionBox hurtBody;
	CollisionBox hitBody;
	int animationFactor;
	sf::Sprite sprite;
	sf::Sprite miniSprite;
	Tileset *ts_mini;
	Tileset *ts;
	int frame;
	Enemy *host;
	MonitorType monitorType;
	bool respawnSpecial;
};

struct HealthFly : Enemy
{
	enum FlyType
	{
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA,
		WHITE,
		Count
	};

	HealthFly( GameSession *owner,
		sf::Vector2i &pos,
		FlyType fType );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw( sf::RenderTarget *target);
	void DrawMinimap( sf::RenderTarget *target );
	bool IHitPlayer();
	void UpdateHitboxes();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();
	void DebugDraw(sf::RenderTarget *target);
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	CollisionBox hurtBody;
	CollisionBox hitBody;
	Enemy *host;
	FlyType flyType;
	sf::Sprite sprite;
	Tileset *ts;
	int frame;
	int animationFactor;
	bool caught;
};

struct Spider : Enemy
{
	enum Action
	{
		MOVE,
		JUMP,
		ATTACK,
		LAND
	};
	struct SurfaceInfo
	{
		SurfaceInfo(){}
		SurfaceInfo( Edge *p_e, double p_q, 
			sf::Vector2<double> &pos )
			:e( p_e ), q( p_q ), position( pos )
		{}
		Edge *e;
		double q;
		sf::Vector2<double> position;
	};

	Spider( GameSession *owner, Edge *ground, 
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

	void HitTerrain( double &q );
	bool StartRoll();
	void FinishedRoll();
	void CheckClosest( Edge * e,
		sf::Vector2<double> &playerPos,
		bool right,
		double cutoffQuant );
	void SetClosestLeft();
	void SetClosestRight();

	SurfaceInfo closestPos;
	Launcher *testLaunch;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;
	Tileset *ts;
	//Tileset *ts_walk;
	//Tileset *ts_roll;

	Action action;
	bool facingRight;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	//CrawlerReverser *lastReverser;
	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	SurfaceMover *mover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial();
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
};

struct EnemyParentNode;



struct EnemyQNode
{
	EnemyQNode():parent(NULL),debug(NULL){}
	sf::Vector2<double> pos;
	double rw;
	double rh;
	sf::RenderWindow *debug;
	EnemyParentNode *parent;
	bool leaf;
};



struct EnemyParentNode : EnemyQNode
{
	EnemyParentNode( const sf::Vector2<double> &pos, double rw, double rh );
	EnemyQNode *children[4];
	// 0    |     1
	//--------------
	// 2    |     3
	
};

struct EnemyLeafNode : EnemyQNode
{
	int objCount;
	EnemyLeafNode( const sf::Vector2<double> &pos, double rw, double rh );
	Enemy *enemies[4];
};

EnemyQNode *Insert( EnemyQNode *node, Enemy* e );
//void Query( EnemyQNode *node, void (*f)( Enemy *e ) );

void DebugDrawQuadTree( sf::RenderWindow *rw, EnemyQNode *node );

struct EnemyQuadTreeCollider
{
	virtual void HandleEnemy( Enemy *e ) = 0;
};

void Query( EnemyQuadTreeCollider *qtc, EnemyQNode *node, const sf::Rect<double> &r );

bool IsEnemyTouchingBox( Enemy *e, const sf::Rect<double> & ir );




#endif