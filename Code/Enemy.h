#ifndef __ENEMY_H__
#define __ENEMY_H__


//#include "Actor.h"
#include <list>
#include "Mover.h"
#include "Movement.h"
#include "EffectLayer.h"
#include <SFML/Audio.hpp>

struct Zone;
struct Monitor;
struct Tileset;
struct Actor;
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
		PATROLLER,
		BAT_UP,
		BAT_DOWN,
		CURVE_TURRET,
		BOSS_CRAWLER,
		BOSS_BIRD,
		CACTUS_SHOTGUN,
		OWL,
		BIG_OWL,
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

	void DebugDraw(sf::RenderTarget *target);
	
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
	bool active;
	int slowMultiple;
	//int maxFramesToLive;
	int framesToLive;
	//sf::VertexArray *va;
	sf::Transform transform;
	Tileset *ts;
	int index;
	int frame;
	int bounceCount;

	double numPhysSteps;
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
	~Launcher();
	static double GetRadius(BasicBullet::BType bt);
	static sf::Vector2f GetOffset(BasicBullet::BType bt);
	//Launcher( LauncherEnemy *handler,
	bool interactWithTerrain;
	void CapBulletVel( double speed );
	
	void Reset();
	BasicBullet *inactiveBullets;
	BasicBullet *activeBullets;
	void DeactivateBullet( BasicBullet *b );
	void DeactivateAllBullets();
	void UpdatePrePhysics();
	void UpdatePhysics( int substep, 
		bool lowRes = false );
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

	void DebugDraw(sf::RenderTarget *target);

	void SetDefaultCollision( int framesToLive,
		Edge *e, sf::Vector2<double> &pos);
	int def_framesToLive;
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

struct EnemyParams
{
	EnemyParams(int p_worldIndex,
		int p_flashFrames, double p_speedBar,
		int p_charge, int p_maxHealth)
		:worldIndex(p_worldIndex),
		flashFrames(p_flashFrames),
		speedBar(p_speedBar),
		charge(p_charge),
		maxHealth( p_maxHealth )
	{

	}

	int worldIndex;
	int flashFrames;
	double speedBar;
	int charge;
	int maxHealth;
};

enum EnemyType
{
	EN_BASICEFFECT,
	EN_PATROLLER,
	EN_AIRDASHER,
	EN_CRAWLER,
	EN_SHROOM,
	EN_SHROOMJELLY,
	EN_BASICTURRET,
	EN_FOOTTRAP,
	EN_CRAWLERQUEEN,
	EN_FLOATINGBOMB,
	EN_BAT,
	EN_STAGBEETLE,
	EN_POISONFROG,
	EN_CURVETURRET,
	EN_GRAVITYFALLER,
	EN_PULSER,
	EN_BADGER,
	EN_CACTUS,
	EN_CACTUSSHOTGUN,
	EN_ROADRUNNER,
	EN_OWL,
	EN_TURTLE,
	EN_CHEETAH,
	EN_CORALNANOBOTS,
	EN_CORAL_BLOCK,
	EN_SPIDER,
	EN_GHOST,
	EN_OVERGROWTH,
	EN_OVERGROWTH_TREE,
	EN_SHARK,
	EN_SWARM,
	EN_COPYCAT,
	EN_GORILLA,
	EN_FLYINGHEAD,
	EN_SPECTER,
	EN_GOAL,
	EN_KEY,
	EN_BOSS_CRAWLER,
	EN_BOSS_BIRD,
	EN_GRAVRING,
	EN_BOSS_COYOTE,
	EN_BOSS_TIGER,
	EN_BOSS_ALLIGATOR,
	EN_BOSS_SKELETON,
	EN_BOSS_BEAR,
	EN_GATEMONITOR,
	EN_HEALTHFLY,
	EN_NEXUS,
	EN_SHIPPICKUP,
	EN_GROUNDTRIGGER,
	EN_SHARD,
	EN_MINE,
	EN_BLOCKER,
	EN_BLOCKERCHAIN,
	EN_RACEFIGHTTARGET,
	EN_BOOSTER,
	EN_GRAVITYGRASS,
	EN_SPRING,
	EN_RAIL,
	EN_COMBOER,
	EN_FLOWERPOD,
	EN_Count
};

struct SlowableObject
{
	int slowCounter;
	int slowMultiple;
	virtual bool IsSlowed(int playerIndex) = 0;
	void ResetSlow()
	{
		slowCounter = 1;
		slowMultiple = 1;
	}
	void SlowCheck( int playerIndex )
	{
		if (IsSlowed(playerIndex))
		{
			if (slowMultiple == 1)
			{
				slowCounter = 1;
				slowMultiple = 5;
			}
		}
		else
		{
			slowCounter = 1;
			slowMultiple = 1;
		}
	}
	bool UpdateAccountingForSlow()
	{
		if (slowCounter == slowMultiple)
		{
			slowCounter = 1;
			return true;
		}
		else
		{
			slowCounter++;
			return false;
		}
	}
};

struct HittableObject
{
	HittableObject() :receivedHit(NULL) {}
	virtual HitboxInfo * IsHit(Actor *player) = 0;
	const bool ReceivedHit() { return receivedHit; }
	bool CheckHit(Actor *player, EnemyType et);
	virtual void ProcessHit() = 0;
	HitboxInfo *receivedHit;
	int numHealth;
};

struct CuttableObject
{
	CuttableObject();
	sf::Vertex quads[2 * 4];
	sf::Vector2f splitDir;
	int separateFrame;
	int totalSeparateFrames;
	float separateSpeed;
	bool active;
	void SetCutRootPos(sf::Vector2f &p_rPos);
	void Reset();
	bool DoneSeparatingCut();
	void IncrementFrame();
	void SetFlipHoriz(bool h)
	{
		flipHoriz = h;
	}
	void SetFlipVert(bool v)
	{
		flipVert = v;
	}
	void UpdateCutObject( int slowCounter );
	int rectWidth;
	int rectHeight;

	bool flipHoriz;
	bool flipVert;
	void Draw( sf::RenderTarget *target );
	Tileset *ts;
	int tIndexFront;
	int tIndexBack;
	void SetSubRectFront( int fIndex );
	void SetSubRectBack( int bIndex );
	void SetTileset(Tileset *ts);
	int rotateAngle;

private:
	sf::Vector2f rootPos;
};

struct Enemy;
struct ComboObject
{
	ComboObject( Enemy *en)
		:enemyHitboxFrame( -1 ),
		enemyHitBody( NULL ),
		nextComboObj( NULL ),
		enemy( en )
	{
	}
	void Reset();
	V2d GetComboPos();
	CollisionBody *enemyHitBody;
	HitboxInfo *enemyHitboxInfo;
	int enemyHitboxFrame;
	ComboObject *nextComboObj;
	void ComboHit();
	void Draw(sf::RenderTarget *target);
	Enemy *enemy;
};

struct Shield;

struct Enemy : QuadTreeCollider, QuadTreeEntrant, 
	SlowableObject, HittableObject
{
private:
	static EnemyParams * enemyTypeHitParams[Count];
public:
	virtual void UpdatePreLauncherPhysics() {}
	CuttableObject *cutObject;
	Launcher **launchers;
	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	HitboxInfo *hitboxInfo;
	sf::SoundBuffer *genericDeathSound;
	virtual void PlayDeathSound();
	virtual int GetNumCamPoints() { return 1; }
	virtual V2d GetCamPoint(int index) { return position; }
	virtual void ComboHit();
	virtual void SetZone(Zone *p_zone) { zone = p_zone; }
	CollisionBody *currHitboxes;
	static void SyncSpriteInfo(sf::Sprite &dest, sf::Sprite &source);
	void SetHitboxes(CollisionBody *cb, int frame);
	void ClearHitboxes() { 
		currHitboxes = NULL;
		currHitboxFrame = -1;
	}
	void ClearHurtboxes() {
		currHurtboxes = NULL;
		currHurtboxFrame = -1;
	}
	void SetHurtboxes(CollisionBody *cb, int frame);
	void DrawSpriteIfExists( 
		sf::RenderTarget *target,
		sf::Sprite &spr );
	//CollisionBox physBox;
	int currHitboxFrame;
	CollisionBody *currHurtboxes;
	bool highResPhysics;
	int currHurtboxFrame;
	virtual void UpdateEnemyPhysics() {}
	virtual void HandleHitAndSurvive() {}
	void CheckedMiniDraw(sf::RenderTarget *target,
		sf::FloatRect &rect);
	void CheckedZoneDraw(sf::RenderTarget *target,
		sf::FloatRect &rect);
	void SetZoneSpritePosition();
	void CheckedZoneUpdate(sf::FloatRect &rect);
	virtual void ShieldDestroyed( Shield *shield ) {};
	//std::list<CollisionBox> *activeHurtboxes;
	CollisionBox *physicsBox;
	virtual void UpdatePhysics( int substep );
	int numLaunchers;
	bool LaunchersAreDone();
	virtual bool IsSlowed( int index );
	virtual HitboxInfo * IsHit(Actor *player);
	Enemy( GameSession *owner, EnemyType t,
		bool hasMonitor, int world, bool cuttable = true );
	virtual ~Enemy();
	virtual void HandleNoHealth();
	virtual void ProcessState() = 0;
	virtual void DebugDraw(sf::RenderTarget *target);
	virtual void UpdateHitboxes() {}
	virtual void ResetEnemy() = 0;
	virtual V2d TurretSetup();//return finals pos
	virtual void Init(){};
	virtual void Setup() {};
	virtual void ProcessHit();
	virtual void ProcessShieldHit();
	virtual void ConfirmHitNoKill();
	virtual void ConfirmKill();
	void MovePos(V2d &vel,
		int slowMult,
		int numPhysSteps);
	
	virtual void RecordEnemy();
	virtual void DirectKill();
	virtual void HandleEntrant(QuadTreeEntrant *qte) {}
	virtual void UpdatePrePhysics();
	virtual void UpdatePostPhysics();
	virtual void UpdateSprite() {}
	virtual void FrameIncrement(){}
	virtual void DrawMinimap( sf::RenderTarget *target );
	virtual void IHitPlayer(int index = 0) {}
	virtual bool CheckHitPlayer(int index = 0);
	virtual int NumTotalBullets();
	virtual void HandleQuery(QuadTreeCollider * qtc);
	virtual bool IsTouchingBox(const sf::Rect<double> &r);

	void Record(int enemyIndex);
	bool RightWireHitMe( CollisionBox hurtBox );
	bool LeftWireHitMe( CollisionBox hurtBox );

	virtual void EnemyDraw(sf::RenderTarget *target) {}
	void Draw(sf::RenderTarget *target);
	virtual void UpdateZoneSprite();
	sf::Sprite zonedSprite;
	Tileset *ts_zoned;
	virtual void ZoneDraw(sf::RenderTarget *target);

	void Reset();
	
	Shield *currShield;
	int pauseFrames;
	double numPhysSteps;
	Enemy *prev;
	Enemy *next;
	GameSession *owner;
	bool spawned;
	sf::Color auraColor;
	sf::Rect<double> spawnRect;
	int frame;
	EnemyType type;
	bool spawnedByClone;
	int initHealth;
	int health;
	sf::Vector2<double> position;
	Zone *zone;
	bool hasMonitor;
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
	Tileset *ts_key;
	sf::Color keyColor;
	int world;

	Enemy *tempPrev;
	Enemy *tempNext;
};

struct PathFollower
{

};

struct EnemyParamsManager
{
	EnemyParamsManager();
	~EnemyParamsManager();
	EnemyParams *GetHitParams(EnemyType et);
	EnemyParams *params[EN_Count];
};


struct EnterNexus1Seq;

//set up the movers first

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
	std::pair<bool,bool> PlayerHitMe( int index = 0 );
	//bool PlayerSlowingMe();
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

struct Specter;
struct SpecterArea : QuadTreeEntrant
{
	SpecterArea(Specter *sp, sf::Vector2i &pos, int rad);
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	int radius;
	sf::Rect<double> testRect;
	CollisionBox barrier;
	Specter *specter;
};


#endif