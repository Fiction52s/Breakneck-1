#ifndef __ENEMY_H__
#define __ENEMY_H__


//#include "Actor.h"
#include <list>
#include "Mover.h"
#include "Movement.h"
#include "EffectLayer.h"

struct Zone;
struct Monitor;
struct Tileset;
//projectile shotting process

//struct Bullet;
struct BasicBullet;
struct LauncherEnemy
{
	virtual void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos ){};
	virtual void BulletHitPlayer( BasicBullet *b ){};
	virtual void BulletHitTarget( BasicBullet *b ){};
	virtual void BulletTTLDeath(BasicBullet *b) {};
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
	static double GetRadius(BasicBullet::BType bt);
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

	void SetDefaultCollision( int framesToLive,
		int substep, Edge *e, sf::Vector2<double> &pos);
	int def_framesToLive;
	int def_substep;
	Edge* def_e;
	bool skipPlayerCollideForSubstep;
	sf::Vector2<double> def_pos;
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
		BLOCKER,
		BLOCKERCHAIN,
		RACEFIGHTTARGET,
		BOOSTER,
		GRAVITYGRASS,
		SPRING,
		RAIL,

		Count
	};

	Enemy( GameSession *owner, EnemyType t,
		bool hasMonitor, int world );
	virtual void Init(){};
	virtual void Setup() {};

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
	Tileset *ts_killSpack;
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

	virtual void HandleQuery( QuadTreeCollider * qtc );
	virtual bool IsTouchingBox( const sf::Rect<double> &r );

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













struct Booster : Enemy
{
	enum Action
	{
		NEUTRAL,
		BOOST,
		REFRESH,
		Count
	};

	Action action;
	Booster(GameSession *owner,
		sf::Vector2i &pos, int strength );
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
	int frame;

	sf::Sprite sprite;
	Tileset *ts;
	//CollisionBox hurtBody;
	CollisionBox hitBody;

	int strength;
	//HitboxInfo *hitboxInfo;

	int animationFactor;

	void SaveEnemyState() {}
	void LoadEnemyState() {}
};

struct Spring : Enemy
{
	//MovementSequence testSeq;
	Spring(GameSession *owner,
		sf::Vector2i &pos, sf::Vector2i &other, int moveFrames );
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
	
	//void HandleQuery(QuadTreeCollider * qtc);
	//bool IsTouchingBox(const sf::Rect<double> &r);

	int frame;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	//HitboxInfo *hitboxInfo;

	int animationFactor;

	void SaveEnemyState() {}
	void LoadEnemyState() {}

	sf::Vector2<double> dir;
	int speed;
	int stunFrames;
};

//w1


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









//w3







//w4










//w5









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