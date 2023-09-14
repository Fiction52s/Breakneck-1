#ifndef __BULLET_H__
#define __BULLET_H__

#include "VectorMath.h"
#include "QuadTree.h"
#include "Physics.h"


struct BasicBullet;
struct Edge;
struct Tileset;
struct Session;

struct LauncherEnemy
{
	virtual void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(
		int playerIndex, BasicBullet *b,
		int hitResult);
	virtual void BulletHitTarget(BasicBullet *b) {};
	virtual void BulletTTLDeath(BasicBullet *b) {};
	virtual int GetAttackIndex() { return -1; };
	virtual void UpdateBullet(BasicBullet *b) {};
	virtual void FireResponse(BasicBullet *b) {};
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
		BAT,
		CURVE_TURRET,
		LOB_TURRET,
		CACTUS,
		OWL,
		BIG_OWL,
		TURTLE,
		LIZARD,
		PARROT,
		GROWING_TREE,
		COPYCAT,
		SPECTER,
		PREDICT,
		BOSS_CRAWLER,
		BOSS_BIRD,
		BOSS_COYOTE,
		BOSS_TIGER,
		Count
	};

	struct BulletData
	{
		int prevID;
		int nextID;
		V2d gravity;
		V2d position;
		V2d velocity;
		int slowCounter;
		bool active;
		int slowMultiple;
		int framesToLive;
		int frame;
		sf::Transform transform;
		int bounceCount;
	};

	int bulletID;

	BasicBullet *prev;
	BasicBullet *next;
	V2d gravity;
	V2d position;
	V2d velocity;
	int slowCounter;
	bool active;
	int slowMultiple;
	int framesToLive;
	int frame;
	sf::Transform transform;
	int bounceCount;
	
	BType bulletType;
	CollisionBox physBody;
	CollisionBox hitBody;
	Tileset *ts;
	int index;
	double numPhysSteps;
	bool col;
	Contact minContact;
	V2d tempVel;
	Launcher *launcher;
	

	BasicBullet(int indexVA, BType bType, Launcher *launcher);

	virtual bool CanInteractWithTerrain();
	virtual int GetNumStoredBytes();
	virtual void StoreBytes(unsigned char *bytes);
	virtual void SetFromBytes(unsigned char *bytes);

	void SetIndex(int ind);
	virtual void HandleEntrant(QuadTreeEntrant *qte);
	virtual void UpdatePrePhysics();
	virtual void Reset(V2d &pos,V2d &vel);

	void Kill( V2d facingDir );
	void DebugDraw(sf::RenderTarget *target);
	virtual void UpdatePhysics();
	virtual void UpdateSprite();
	virtual void UpdatePostPhysics();
	virtual void ResetSprite();
	bool ResolvePhysics(
		V2d vel);
	bool PlayerSlowingMe();
	virtual bool HitTerrain();
	void HitPlayer( int pIndex, 
		int hitResult );
	void StoreBasicBulletData(BulletData &bd);
	void SetBasicBulletData(BulletData &bd);
};

struct SinBullet : BasicBullet
{
	struct MyData : BulletData
	{
		V2d tempadd;
	};

	MyData data;

	//CollisionBox hurtBody;
	

	SinBullet(int indexVA, Launcher *launcher);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void Reset(
		V2d &pos,
		V2d &vel);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

struct GrindBullet : BasicBullet
{
	struct MyData : BulletData
	{
		double grindSpeed;
		bool clockwise;
		EdgeInfo grindEdgeInfo;
		double edgeQuantity;
	};

	MyData data;
	Edge *grindEdge;


	GrindBullet(int indexVA, Launcher *launcher);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void Reset(V2d &pos,
		V2d &vel);
	bool HitTerrain();
	bool CanInteractWithTerrain();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

struct CopycatBullet : BasicBullet
{
	struct MyData : BulletData
	{
		V2d destination;
		V2d trueVel;
		int attackIndex;
	};

	MyData data;

	
	double speed;
	

	CopycatBullet(int indexVA, Launcher *launcher);
	void UpdatePrePhysics();
	void UpdatePhysics();

	//modified reset
	void Reset(
		V2d &pos0,
		V2d &pos1);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

struct Launcher
{
	struct MyData
	{
		int inactiveBulletsID;
		int activeBulletsID;

		//BasicBullet *inactiveBullets;
		//BasicBullet *activeBullets;
		//sf::Vector2<double> position;
		//sf::Vector2<double> facingDir;
	};

	int launcherID;

	double bulletSpeed;
	BasicBullet *inactiveBullets;
	BasicBullet *activeBullets;
	V2d position;
	V2d facingDir;
	
	int bytesStoredPerBullet;
	int playerIndex;
	int bulletTilesetIndex;
	bool interactWithTerrain;
	bool interactWithPlayer; //useful for simulations etc
	sf::Vertex *bulletVA;
	BasicBullet::BType bulletType;
	LauncherEnemy *handler;
	Tileset *ts_bullet;
	bool drawOwnBullets;
	Session *sess;
	int totalBullets;
	int perShot;
	HitboxInfo *hitboxInfo;
	
	double angleSpread;
	double amplitude;
	int wavelength;
	int maxFramesToLive;
	double maxBulletSpeed;
	int def_framesToLive;
	bool skipPlayerCollideForSubstep;
	Edge* def_e;
	sf::Vector2<double> def_pos;
	std::vector<BasicBullet*> allBullets;

	Launcher(LauncherEnemy *handler,
		BasicBullet::BType bulletType,
		int numTotalBullets,
		int bulletsPerShot,
		sf::Vector2<double> position,
		sf::Vector2<double> direction,
		double angleSpread,
		int maxFramesToLive,
		bool hitTerrain = true,
		int wavelength = 0,
		double amplitude = 0.0,
		Tileset *ts_myTileset = NULL );
	Launcher(LauncherEnemy *handler,
		int maxFramesToLive);
	~Launcher();
	int GetBulletID(BasicBullet *b);
	BasicBullet *GetBulletFromID(int id);
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	int GetNumStoredBytes();
	static double GetRadius(BasicBullet::BType bt);
	static sf::Vector2f GetOffset(BasicBullet::BType bt);
	void SetStartIndex(int ind);	
	void CapBulletVel(double speed);
	void Reset();
	void DeactivateBullet(BasicBullet *b);
	void DeactivateAllBullets();
	void UpdatePrePhysics();
	void UpdatePhysics(int substep,
		bool lowRes = false);
	void UpdatePostPhysics();
	void UpdateSprites();
	BasicBullet * ActivateBullet();
	int GetActiveCount();
	void Fire();
	void Fire(double gravStrength);
	virtual BasicBullet * RanOutOfBullets();
	void AddToList(BasicBullet *b,
		BasicBullet *&list);
	void SetGravity(sf::Vector2<double> &grav);
	void SetBulletSpeed(double speed);
	void Draw(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	void SetDefaultCollision(int framesToLive,
		Edge *e, V2d &pos);
	void KillAllBullets();
};


#endif