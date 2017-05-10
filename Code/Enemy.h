#ifndef __ENEMY_H__
#define __ENEMY_H__


#include "Actor.h"
#include <list>
#include "Mover.h"
#include "Movement.h"


struct Zone;
struct Monitor;
//projectile shotting process

//struct Bullet;
struct BasicBullet;
struct LauncherEnemy
{
	virtual void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos ){};
	virtual void BulletHitPlayer( BasicBullet *b ){};
	virtual void BulletHitTarget( BasicBullet *b ){};
	virtual int GetAttackIndex(){return -1;};
};
//a step is the amount of time in a substep
//which is a tenth of a step right now i think
struct Launcher;
struct BasicBullet : QuadTreeCollider
{
	enum BType
	{
		BASIC_TURRET,
		BAT,
		CURVE_TURRET,
		BOSS_CRAWLER,
		BOSS_BIRD,
		CACTUS_TURRET,
		OWL,
		BOSS_COYOTE,
		TURTLE,
		BOSS_TIGER,
		GROWING_TREE,
		COPYCAT,
		Count
	};

	BasicBullet( int indexVA, BType bType, Launcher *launcher );
	BType bulletType;
	BasicBullet *prev;
	BasicBullet *next;
	sf::Vector2<double> position;
	//CollisionBox hurtBody;
	CollisionBox physBody;
	CollisionBox hitBody;
	virtual void HandleEntrant( QuadTreeEntrant *qte );
	virtual void UpdatePrePhysics();
	virtual void Reset(
		sf::Vector2<double> &pos,
		sf::Vector2<double> &vel );
	virtual void UpdatePhysics();
	virtual void UpdateSprite();
	virtual void UpdatePostPhysics();
	virtual void ResetSprite();
	bool ResolvePhysics( 
		sf::Vector2<double> vel );
	bool PlayerSlowingMe();
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
	int frame;
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
	void Reset(
		sf::Vector2<double> &pos,
		sf::Vector2<double> &vel );

	SinBullet *prev;
	SinBullet *next;
	//int slowCounter;
	CollisionBox hurtBody;
	sf::Vector2<double> tempadd;
};

struct CopycatBullet : BasicBullet
{
	CopycatBullet( int indexVA, Launcher *launcher );
	void UpdatePrePhysics();
	void UpdatePhysics();

	//modified reset
	void Reset(
		sf::Vector2<double> &pos0,
		sf::Vector2<double> &pos1 );

	CopycatBullet *prev;
	CopycatBullet *next;
	sf::Vector2<double> destination;
	sf::Vector2<double> trueVel;
	double speed;
	int attackIndex;
	//CollisionBox hurtBody;
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
	
	int bulletTilesetIndex;
	BasicBullet::BType bulletType;
	Launcher( LauncherEnemy *handler, 
		BasicBullet::BType bulletType,
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
	double maxBulletSpeed;
	//Launcher *next;
};

struct Bullet : QuadTreeCollider
{
	Bullet( int indexVA, Launcher *launcher,
		double radius );
	
	virtual void HandleEntrant( QuadTreeEntrant *qte );
	virtual void UpdatePrePhysics();
	virtual void Reset(
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
	
	
	CollisionBox physBody;
	CollisionBox hitBody;
	CollisionBox hurtBody;
	Bullet *prev;
	Bullet *next;
	sf::Vector2<double> position;
	sf::Vector2<double> velocity;
	int slowCounter;
	int slowMultiple;
	int framesToLive;
	sf::Transform transform;
	Tileset *ts;
	int index;
	bool col;
	Contact minContact;
	sf::Vector2<double> tempVel;
	Launcher *launcher;
};

namespace BulletType
{
	enum Type
	{
		NORMAL,
		SIN,
		BIRDBOSS,
		Count
	};
}
Bullet* CreateBullet( BulletType::Type type, int vaIndex, Launcher *launcher );

struct Movable : QuadTreeCollider
{
	Movable();
	virtual void HandleEntrant( QuadTreeEntrant *qte );
	virtual void UpdatePrePhysics();
	virtual void Reset(
		sf::Vector2<double> &pos );
	virtual void UpdatePhysics();
	virtual void UpdatePostPhysics();
	bool ResolvePhysics( 
		sf::Vector2<double> vel );
	virtual void HitTerrain();
	virtual void HitPlayer();
	virtual void IncrementFrame();
	
	CollisionBox physBody;
	CollisionBox hitBody;
	CollisionBox hurtBody;

	bool collideWithTerrain;
	bool collideWithPlayer;

	sf::Vector2<double> position;
	sf::Vector2<double> velocity;
	int slowCounter;
	int slowMultiple;

	bool col;
	Contact minContact;
	sf::Vector2<double> tempVel;
	static GameSession *owner;
	//Launcher *launcher;
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
		SPECTER,
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
		NEXUS,
		SHIPPICKUP,
		SHARD,
		MINE,
		RACEFIGHTTARGET,
		Count
	};

	Enemy( GameSession *owner, EnemyType t,
		bool hasMonitor, int world );
	virtual void Init(){};

	void Record( int enemyIndex );
	virtual void RecordEnemy();
	virtual void DirectKill();
	//virtual void HandleEdge( Edge *e ) = 0;
	virtual void HandleEntrant( QuadTreeEntrant *qte ) = 0;
	virtual void UpdatePrePhysics() = 0;
	virtual void UpdatePhysics() = 0;
	virtual void UpdatePostPhysics() = 0;
	virtual void Draw( sf::RenderTarget *target) = 0;
	virtual void DrawMinimap( sf::RenderTarget *target ){};
	virtual bool IHitPlayer(int index = 0) = 0;
	virtual void UpdateHitboxes() = 0;
	virtual std::pair<bool,bool> PlayerHitMe(int index = 0) = 0;
	bool RightWireHitMe( CollisionBox hurtBox );
	bool LeftWireHitMe( CollisionBox hurtBox );
	virtual bool PlayerSlowingMe() = 0;
	virtual void DebugDraw(sf::RenderTarget *target) = 0;
	virtual int NumTotalBullets();
	void SaveState();
	void LoadState();
	virtual void SaveEnemyState() = 0;
	virtual void LoadEnemyState() = 0;
	void Reset();
	virtual void ResetEnemy() = 0;
	Enemy *prev;
	Enemy *next;
	GameSession *owner;
	bool spawned;
	sf::Color auraColor;
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
	bool hasMonitor;
	//Monitor *monitor;
	bool dead;
	bool suppressMonitor;
	bool specterProtected;
	Tileset *ts_hitSpack;
	sf::Shader *keyShader;
	sf::Shader *hurtShader;

	bool affectCameraZoom;

	Tileset *ts_blood;

	sf::Sprite *keySprite;
	//sf::Sprite bloodSprite;
	Tileset *ts_key;
	//int keyFrame;
	sf::Color keyColor;
	int world;

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

struct EnterNexus1Seq;
struct Nexus : Enemy
{
	enum Action
	{
		NORMAL,
		ENTERED,
		EXPLODED,
		Count
	};

	Action action;
	Nexus( GameSession *owner,
		Edge *ground, double quantity,
		int nexusIndex );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();
	
	EnterNexus1Seq * enterNexus1Seq;
	
	sf::Sprite sprite;
	Tileset *ts;
	sf::Vector2<double> entrancePos;
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

struct ShipPickup : Enemy
{
	ShipPickup( GameSession *owner,
		Edge *ground, double quantity,
		bool facingRight );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();
	//void DirectKill();
	bool facingRight;

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
//set up the movers first

enum EffectLayer
{
	BEHIND_TERRAIN,
	BEHIND_ENEMIES,
	BETWEEN_PLAYER_AND_ENEMIES,
	IN_FRONT,
	IN_FRONT_1,
	IN_FRONT_OF_UI,
	Count
};

struct BasicEffect : Enemy
{
	BasicEffect ( GameSession *owner );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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
	EffectLayer layer;
	int stored_frame;
	//sf::Vector2<double> position;
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();
	
	bool exploding;
	bool kinKilling;
	bool destroyed;
	//int kinKillFrame;
	sf::Sprite sprite;
	sf::Sprite miniSprite;
	Tileset *ts;
	Tileset *ts_mini;
	Tileset *ts_explosion;
	float goalKillStartZoom;
	sf::Vector2f goalKillStartPos;

	Edge *ground;
	double edgeQuantity;

	CollisionBox hurtBody;
	//CollisionBox hitBody;
	//HitboxInfo *hitboxInfo;
	
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
	bool IHitPlayer( int index = 0 );
	void UpdateHitboxes();
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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

struct Shard : Enemy
{
	
	//MovementSequence testSeq;
	Shard( GameSession *owner, sf::Vector2i pos,
		int shardsLoaded );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;

	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void SaveEnemyState();
	void LoadEnemyState();

	int shardIndex;

	int deathFrame;

	int radius;
	
	//sf::Vector2<double> deathVector;
	//double deathPartingSpeed;
	//sf::Sprite botDeathSprite;
	//sf::Sprite topDeathSprite;
	//Tileset * ts_death;
	//std::list<sf::Vector2i> path;
	
	//int targetNode;
	//bool forward;
	//sf::Vector2<double>
	int frame;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	bool caught;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	//Tileset *ts_testBlood;
	//sf::Sprite bloodSprite;
	//int bloodFrame;
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

struct Mine : Enemy
{
	enum Action
	{
		NEUTRAL,
		MALFUNCTION,
		Count
	};
	Action action;
	//MovementSequence testSeq;
	Mine( GameSession *owner,
		 bool hasMonitor,
		 sf::Vector2i pos );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;

	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void SaveEnemyState();
	void LoadEnemyState();

	int frame;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	//Tileset *ts_testBlood;
	//sf::Sprite bloodSprite;
	//int bloodFrame;
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

struct RaceFightTarget : Enemy
{
	enum Action
	{
		NEUTRAL,
		PLAYER1,
		PLAYER2,
		PLAYER3,
		PLAYER4,
		Count
	};

	RaceFightTarget *pPrev;
	RaceFightTarget *pNext;
	RaceFightTarget *p2Prev;
	RaceFightTarget *p2Next;

	int gameTimeP1Hit;
	int gameTimeP2Hit;

	Action action;
	//MovementSequence testSeq;
	RaceFightTarget( GameSession *owner,
		 sf::Vector2i &pos );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;

	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void SaveEnemyState();
	void LoadEnemyState();

	int frame;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	//CollisionBox hitBody;
	//HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;
};

//w1
struct Patroller : Enemy
{
	Patroller( GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, std::list<sf::Vector2i> &path, bool loop, int speed );
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void SaveEnemyState();
	void LoadEnemyState();

	void AdvanceTargetNode();
	
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
	Crawler( GameSession *owner, bool hasMonitor, Edge *ground, double quantity, bool clockwise, double speed );
//	void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void ResetEnemy();
	//void DirectKill();
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

struct BasicTurret : Enemy, LauncherEnemy
{
	BasicTurret( GameSession *owner, bool hasMonitor, Edge *ground, double quantity, 
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
	bool IHitPlayer( int index = 0 );
	bool IHitPlayerWithBullets();
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	std::pair<bool, bool> PlayerHitMyBullets();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void UpdateBulletHitboxes();
	Tileset *ts_bulletExplode;

	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );

	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	sf::Sprite sprite;
	Tileset *ts;
	
	const static int maxBullets = 16;
	sf::Vector2<double> bulletPositions[maxBullets];
	sf::Vector2<double> tempVel;
	
	Launcher *launcher;
	void DirectKill();


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

	bool dying;
	

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

struct FootTrap : Enemy
{
	FootTrap( GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();
	//void DirectKill();
	

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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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

//w2
struct Bat : Enemy, LauncherEnemy
{
	MovementSequence testSeq;
	Bat( GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, std::list<sf::Vector2i> &path,
		int bulletSpeed,
		//int nodeDistance,
		int framesBetween,
		bool loop );
	void DirectKill();
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	//void AdvanceTargetNode();

	void SaveEnemyState();
	void LoadEnemyState();

	int bulletSpeed;
	//int nodeDistance;
	int framesBetween;

	Tileset *ts_bulletExplode;

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

struct StagBeetle : Enemy, GroundMoverHandler
{
	enum Action
	{
		IDLE,
		RUN,
		JUMP,
		//ATTACK,
		LAND,
		Count
	};

	StagBeetle( GameSession *owner, bool hasMonitor,
		Edge *ground, 
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void ResetEnemy();
	//void DirectKill();
	void SaveEnemyState();
	void LoadEnemyState();

	void HitTerrain( double &q );
	bool StartRoll();
	void FinishedRoll();

	Launcher *testLaunch;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;

	Tileset *ts;
	Tileset *ts_death;
	Tileset *ts_hop;
	Tileset *ts_idle;
	Tileset *ts_run;
	Tileset *ts_sweep;
	Tileset *ts_walk;

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
	void HitOtherAerial( Edge *e );
	void Land();

	CollisionBox hurtBody;
	CollisionBox hitBody;
	//CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	sf::Vector2<double> tempVel;
	sf::Vector2<double> gravity;
	

	int actionLength[Action::Count];
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
	double angle;
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

struct CurveTurret : Enemy, LauncherEnemy
{
	CurveTurret( GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity, 
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
	bool IHitPlayer( int index = 0 );
	bool IHitPlayerWithBullets();
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	std::pair<bool, bool> PlayerHitMyBullets();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void DirectKill();
	//void UpdateBulletHitboxes();
	void BulletHitTerrain(BasicBullet *b, 
		Edge *edge, 
		sf::Vector2<double> &pos);
	void BulletHitPlayer( BasicBullet *b );
	//void DirectKill();
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	Launcher *testLauncher;

	sf::Sprite sprite;
	Tileset *ts;
	Tileset *ts_bulletExplode;
	const static int maxBullets = 16;
	sf::Vector2<double> tempVel;

	int framesWait;
	int firingCounter;
	int realWait;
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
	
	PoisonFrog( GameSession *owner, 
		bool hasMonitor,
		Edge *ground, double quantity,
		int gravFactor,
		sf::Vector2i &jumpStrength,
		int jumpFramesWait );

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

	
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void ActionEnded();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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
	void HitOtherAerial( Edge *e );
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


//w3
struct Pulser : Enemy
{
	Pulser( GameSession *owner, 
		bool hasMonitor,
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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

	Badger( GameSession *owner, 
		bool hasMonitor,
		Edge *ground, 
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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

	//CrawlerReverser *lastReverser;
	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	GroundMover *testMover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial( Edge *e );
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

struct Owl : Enemy, LauncherEnemy
{
	enum Action
	{
		REST,
		GUARD,
		SPIN,
		FIRE
	};

	sf::CircleShape guardCircle;
	bool hasGuard;
	Owl( GameSession *owner,
		bool hasMonitor,
		sf::Vector2i &pos,
		int bulletSpeed,
		int framesBetween,
		bool facingRight );
	void DirectKill();
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void SaveEnemyState();
	void LoadEnemyState();
	double ang;
	sf::Vector2<double> fireDir;


	Tileset *ts_bulletExplode;
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

	Tileset *ts_death;
	Tileset *ts_flap;
	Tileset *ts_spin;
	Tileset *ts_throw;

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

	//Tileset *ts_testBlood;
	//Tileset *ts_blood;
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

struct Cactus : Enemy, LauncherEnemy
{
	Cactus( GameSession *owner, bool hasMonitor,
		 Edge *ground, double quantity, 
		 int p_bulletSpeed, int p_rhythm, 
		 int p_amplitude );
//	void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer( int index = 0 );
	bool IHitPlayerWithBullets();
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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
	void DirectKill();
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	Tileset *ts_bulletExplode;
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

//w4
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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
	std::string queryMode;
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
		 bool hasMonitor,
		sf::Vector2i &pos, int moveFrames );
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
	//bool IHitPlayer( int index = 0 );
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

	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	bool PlayerSlowingMe();

	int animationFactor;
	void AddToList( CoralBlock *block,
		CoralBlock *&list );

	//Edge *origGround;
	sf::Vector2<double> origPosition;
	//double origQuantity;
	
	int moveFrames;

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

struct SecurityWeb : Enemy, RayCastHandler
{
	SecurityWeb( GameSession *owner,
		 bool hasMonitor,
		sf::Vector2i &pos, int numProtrusions,
		float angleOffset, 
		double bulletSpeed
		);
	virtual ~SecurityWeb();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void ActionEnded();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void SaveEnemyState();
	void LoadEnemyState();
	sf::VertexArray *edges;
	sf::VertexArray *nodes;
	void HandleRayCollision( Edge *edge, 
		double edgeQuantity, double rayPortion );
	void DirectKill();
	void ResetNodes();
	
	sf::Vector2<double> *origins;
	Tileset *ts_bulletExplode;
	sf::VertexArray *armVA;
	int *armLength;


	struct NodeProjectile : Movable
	{
		
		NodeProjectile( SecurityWeb *parent,
			int vaIndex );
		void Reset(
			sf::Vector2<double> &pos );
		void UpdatePrePhysics();
		void SetNode( int subIndex );
		void UpdatePostPhysics();
		void HitPlayer();
		void IncrementFrame();
		bool PlayerSlowingMe();
		//bool activated;
		int vaIndex;
		//void Fire( sf::Vector2<double> vel );
		//void Draw( sf::RenderTarget *target );
		//Tileset *ts;
		int frame;
		int framesToLive;
		bool active;
		NodeProjectile *nextProj;
		double startAngle;
		double moveAngle;
		
		//NodeProjectile *revNode;
		SecurityWeb *parent;
	};

	bool dynamicMode;
	int dynamicFrame;
	CollisionBox *edgeHitboxes;
	NodeProjectile *activeNodes;
	NodeProjectile **allNodes;

	//NodeProjectile *centerNode;

	//CollisionBox *centerHitbox;


	double bulletSpeed;
	float angleOffset;
	int numProtrusions;
	int maxProtLength;

	int deathFrame;

	Edge *rcEdge;
	double rcQuantity;
	sf::Vector2<double> rayStart;
	sf::Vector2<double> rayEnd;

	int targetFrames;

	bool dead;
	bool dying;
	int frame;

	int slowCounter;
	int slowMultiple;

	int nodeRadius;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
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

	Turtle( GameSession *owner, bool hasMonitor,
		sf::Vector2i pos );
	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DirectKill();
	void ActionEnded();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void SaveEnemyState();
	void LoadEnemyState();

	std::map<Action,int> actionLength;
	std::map<Action,int> animFactor;

	Tileset *ts_bulletExplode;
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

struct Cheetah : Enemy, GroundMoverHandler
{
	enum Action
	{
		NEUTRAL,
		CHARGEUP,
		BURST,
		ARRIVE,
		TURNAROUND,
		JUMP,
		ATTACK,
		LAND,
		Count
	};

	Cheetah( GameSession *owner, bool hasMonitor,
		Edge *ground, 
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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
	GroundMover *trueMover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial( Edge *e );
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

struct Spider : Enemy, RayCastHandler
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
		bool clockwiseFromCurrent;
	};

	Spider( GameSession *owner, bool hasMonitor,
		Edge *ground, 
		double quantity, int speed );
	void ActionEnded();
	int NumTotalBullets();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion );
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

	Edge *rcEdge;
	double rcQuantity;
	sf::Vector2<double> rayStart;
	sf::Vector2<double> rayEnd;

	bool canSeePlayer;

	int framesLaseringPlayer;
	int laserLevel;

	double laserAngle;


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
	void HitOtherAerial( Edge *e );
	void Land();

	CollisionBox hurtBody;
	CollisionBox hitBody;
	//CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	HitboxInfo *laserInfo0;
	HitboxInfo *laserInfo1;
	HitboxInfo *laserInfo2;
	HitboxInfo *laserInfo3;
	int laserCounter;


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

//w5
struct Ghost : Enemy
{
	enum Action
	{
		WAKEUP,
		APPROACH,
		BITE,
		EXPLODE,
		Count
	};

	sf::Rect<double> detectionRect;
	Action action;
	std::map<Action,int> actionLength;
	std::map<Action,int> animFactor;

	double latchStartAngle;
	MovementSequence testSeq;
	Ghost( GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, 
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void SaveEnemyState();
	void LoadEnemyState();

	bool origFacingRight;
	int awakeFrames;
	int awakeCap;
	bool awake;

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

struct GrowingTree : Enemy, LauncherEnemy
{
	enum Action
	{
		RECOVER0,
		RECOVER1,
		RECOVER2,
		LEVEL0,
		LEVEL0TO1,
		LEVEL1,
		LEVEL1TO2,
		LEVEL2,
		EXPLODE,
		Count
	};
	Action action;

	GrowingTree( GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity, 
		int numBullets, int startLevel,
		int pulseRadius );

	void ActionEnded();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	bool PlayerSlowingMe();
	void UpdateSprite();
	void Fire();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();
	void BulletHitTerrain(BasicBullet *b, 
		Edge *edge, 
		sf::Vector2<double> &pos);
	void DirectKill();
	void InitRangeMarkerVA();
	void BulletHitPlayer( BasicBullet *b );
	//void DirectKill();
	double pulseRadius;
	int pulseFrame;
	Tileset *ts_bulletExplode;
	int powerLevel;
	int totalBullets;
	Launcher *launcher;
	sf::Sprite sprite;
	Tileset *ts;
	int startPowerLevel;
	Edge *ground;
	sf::VertexArray rangeMarkerVA;
	double edgeQuantity;

	int actionLength[Action::Count];
	int animFactor[Action::Count];

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	
	bool dying;
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

struct Shark : Enemy
{
	enum Action
	{
		WAKEUP,
		APPROACH,
		CIRCLE,
		FINALCIRCLE,
		RUSH,
	};
	Action action; 

	int circleCounter;

	int wakeCounter;
	//int wakeCap;
	int wakeCap;

	int circleFrames;
	double attackAngle;
	//int attackCounter;

	std::map<Action,int> actionLength;
	std::map<Action,int> animFactor;

	double latchStartAngle;
	MovementSequence circleSeq;
	MovementSequence rushSeq;
	Shark( GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, 
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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
	
	sf::Color testColor; //for temp anim

	int frame;

	double acceleration;
	double speed;

	//int approachFrames;
	//int totalFrame;
	sf::Vector2<double> origOffset;
	sf::Vector2<double> attackOffset;

	sf::Sprite sprite;
	Tileset *ts_circle;
	Tileset *ts_bite;
	Tileset *ts_death;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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
	

	Overgrowth( GameSession *owner,
		 bool hasMonitor,
		 Edge *ground, double quantity, 
		double bulletSpeed, int lifeCycleFrames );
//	void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	//bool IHitPlayer( int index = 0 );
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void InitTrees();
	void AddTree( Tree *tree );
	int NumTotalBullets();
	void DeactivateTree( Tree *tree );
	Tree * ActivateTree( Edge *g, double q );

	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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

struct Swarm;
struct SwarmMember : Enemy
{
	SwarmMember(Swarm *parent, 
		sf::VertexArray &va, int index,
		sf::Vector2<double> &targetOffset,
		double p_maxSpeed );
	void ClearSprite();
	void UpdatePostPhysics();
	void UpdateSprite();
	void UpdatePrePhysics();
	void UpdatePhysics();
	void HandleEntrant( QuadTreeEntrant *qte );
	void PhysicsResponse();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	bool PlayerSlowingMe();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	int framesToLive;

	Swarm *parent;
	double maxSpeed;
	int vaIndex;
	int frame;
	int animFactor;
	sf::Vector2<double> targetOffset;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	sf::Vector2<double> velocity;
	
	
	bool active;
	//Edge *ground;
	
	//bool active;


	
	sf::VertexArray &va;

	//CollisionBox hurtBody;
	//CollisionBox hitBody;
	//CollisionBox physBody;
	//HitboxInfo *hitboxInfo;
	
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
	//Tileset *ts_testBlood;
	//sf::Sprite bloodSprite;
	//int bloodFrame;
};

struct Swarm : Enemy
{
	enum Action
	{
		NEUTRAL,
		FIRE,
		USED,
		REFILL,
		Count
	};
	
	Swarm( GameSession *owner, 
		sf::Vector2i &pos,
		int liveFrames );
	void ActionEnded();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	//bool IHitPlayer( int index = 0 );
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	bool PlayerSlowingMe();
	void Launch();

	int animationFactor;
	int actionLength[Action::Count];
	int animFactor[Action::Count];
	int frame;
	Action action;
	int liveFrames;

	sf::Vector2<double> origPosition;
	bool dying;
	Tileset *ts;
	Tileset *ts_swarm;

	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	const static int NUM_SWARM = 5;
	sf::VertexArray swarmVA;
	sf::Sprite nestSprite;
	SwarmMember *members[NUM_SWARM];

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	Tileset *ts_swarmExplode;

	sf::Vector2f spriteSize;
	double maxSpeed;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	//Tileset *ts_testBlood;
	//sf::Sprite bloodSprite;
	//int bloodFrame;

	int deathFrame;
};

//w6
struct Specter;
struct SpecterArea : QuadTreeEntrant
{
	SpecterArea( Specter *sp, sf::Vector2i &pos, int rad );
	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );
	int radius;
	sf::Rect<double> testRect;
	CollisionBox barrier;
	Specter *specter;
};

struct Specter : Enemy
{
	
	//MovementSequence testSeq;
	Specter( GameSession *owner,
		 bool hasMonitor,
		 sf::Vector2i pos );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;

	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void SaveEnemyState();
	void LoadEnemyState();

	int deathFrame;

	int radius;
	
	
	SpecterArea myArea;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	//Tileset * ts_death;
	//std::list<sf::Vector2i> path;
	
	//int targetNode;
	//bool forward;
	//sf::Vector2<double>
	int frame;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	//Tileset *ts_testBlood;
	//sf::Sprite bloodSprite;
	//int bloodFrame;
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

	Narwhal( GameSession *owner, bool hasMonitor,
		sf::Vector2i &startPos, 
		sf::Vector2i &endPos, int moveFrames );
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
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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

	//bool redDead;
	//bool blueDead;
	bool dying;

	//HitboxInfo *receivedHitRed;
	//HitboxInfo *receivedHitBlue;

	Jay( GameSession *owner, bool hasMonitor,
		sf::Vector2i &startPos, 
		sf::Vector2i &endPos );
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
	bool IHitPlayer( int index = 0 );
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
	
	std::pair<bool,bool> PlayerHitRed();
	std::pair<bool,bool> PlayerHitBlue();
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
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

struct Gorilla : Enemy
{
	enum Action
	{
		WAKEUP,
		ALIGN,
		FOLLOW,
		ATTACK,
		RECOVER,
		Count
	};

	sf::Rect<double> detectionRect;
	Action action;
	std::map<Action,int> actionLength;
	std::map<Action,int> animFactor;

	Tileset *ts_wall;
	sf::Sprite wallSprite;
	
	double latchStartAngle;
	
	Gorilla( GameSession *owner, bool hasMonitor,
		sf::Vector2i &pos, int wallWidth, int followFrames );
		//,int recoveryLoops );

	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	//bool physicsOver;

	void ActionEnded();

	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void SaveEnemyState();
	void LoadEnemyState();

	bool origFacingRight;

	int awakeFrames;
	int awakeCap;
	//bool awake;

	sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;

	int alignMoveFrames;
	int createWallFrame;
	int alignFrames;
	int followFrames;
	int recoveryLoops;
	int recoveryCounter;
	
	int frame;

	double acceleration;
	double speed;

	int approachFrames;
	int totalFrame;
	

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	CollisionBox wallHitbox;
	int wallHitboxWidth;
	int wallHitboxHeight;
	double idealRadius;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	//Tileset *ts_testBlood;
	//sf::Sprite bloodSprite;
	//int bloodFrame;
	bool facingRight;

	CubicBezier approachAccelBez;

	
	sf::Vector2<double> offsetPlayer;
	sf::Vector2<double> origPosition;
	sf::Vector2<double> origOffset;
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

struct Copycat : Enemy, LauncherEnemy
{
	enum Action
	{
		NEUTRAL,
		THROW,
		Count
	};

	Tileset *ts_bulletExplode;
	Launcher *launcher;
	struct PlayerAttack
	{
		enum Type
		{
			FAIR,
			DAIR,
			UAIR,
			STANDN,
			CLIMBATTACK,
			SLIDEATTACK,
			WALLATTACK,
			Count
		};

		int currAttackIndex;
		Copycat *parent;
		PlayerAttack( Copycat *parent );
		//Action a;
		Type t;
		bool facingRight;
		bool reversed;
		int speedLevel;
		sf::Vector2<float> position;
		sf::Vector2<float> swordPosition;
		float angle;
		bool attackActive;
		int index;
		PlayerAttack *nextAttack;
		PlayerAttack *prevAttack;
		int frame;
		sf::Sprite sprite;
		sf::Sprite swordSprite;
		bool Update();
		void UpdateHitboxes();
		void UpdateSprite();
		void Set( PlayerAttack::Type nt,
			bool facingRight,
			bool reversed,
			int speedLevel,
			const sf::Vector2f &pos,
			const sf::Vector2f &swordPos,
			float angle );
		//void SetType( Type nt );
		void Draw( sf::RenderTarget *target );
		void DebugDraw( sf::RenderTarget *target );

		void CopyHitboxes( int index,
		std::map<int, std::list<CollisionBox>*> &playerBoxes );

		std::map<int, std::list<CollisionBox>*> 
		hitboxes[PlayerAttack::Type::Count];
	};


	int GetAttackIndex();
	Tileset *ts_attacks[PlayerAttack::Type::Count];
	Tileset *ts_swords[PlayerAttack::Type::Count * 3];
	void BulletHitTarget( BasicBullet *b );
	PlayerAttack *GetAttack();
	PlayerAttack *PopAttack();
	void RemoveAttack( PlayerAttack *pa );
	void ClearTargets();
	void ClearTarget( int index );
	void SetTarget( int index, const sf::Vector2f &pos );
	void ResetAttacks();
	PlayerAttack *activeAttacksFront;
	PlayerAttack *activeAttacksBack;
	PlayerAttack *inactiveAttacks;
	PlayerAttack **allAttacks;
	int attackBufferSize;

	//std::map<sf::Vector2<double>, int> bulletMap;

	sf::Vector2<float> destPos;
	bool fire;

	Copycat( GameSession *owner, bool hasMonitor,
		sf::Vector2i &pos );
	void QueueAttack( PlayerAttack::Type t,
		bool facingRight,
		bool reversed, int speedLevel,
		const sf::Vector2<float> &pos,
		const sf::Vector2<float> &swordPos,
		float angle );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void ActionEnded();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer( int index = 0 );
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void SaveEnemyState();
	void LoadEnemyState();
	void DirectKill();
	void SetTarget();

	std::map<Action,int> actionLength;
	std::map<Action,int> animFactor;

	int currAttackFrame;
	//PlayerAttack currAttack;
	bool activeActive;

	Action action;
	//sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	
	sf::Vector2i originalPos;
	int frame;

	std::list<CollisionBox> *currHitboxes;
	//int numCurrHitboxes;
	HitboxInfo *currHitboxInfo;
	//std::list<CollisionBox> *currHitboxes;

	
	/*std::map<int, std::list<CollisionBox>*> uairHitboxes;
	std::map<int, std::list<CollisionBox>*> dairHitboxes;
	std::map<int, std::list<CollisionBox>*> standHitboxes;
	std::map<int, std::list<CollisionBox>*> wallHitboxes;
	std::map<int, std::list<CollisionBox>*> steepClimbHitboxes;
	std::map<int, std::list<CollisionBox>*> steepSlideHitboxes;*/
	

	int attackLength[PlayerAttack::Type::Count];
	int attackFactor[PlayerAttack::Type::Count];

	sf::VertexArray *targetVA;
	//sf::VertexArray bulletVA;

	bool dying;

	Tileset *ts_target;

	
	sf::Vector2<double> shadowPos;
	sf::Sprite sprite;
	sf::Sprite shadowSprite;
	sf::Sprite shadowSword;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;


	int hitlagFrames;
	int hitstunFrames;
	//int animationFactor;
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