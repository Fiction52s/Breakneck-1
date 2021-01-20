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
		Edge *edge,V2d &pos) {};
	virtual void BulletHitPlayer(
		int playerIndex, BasicBullet *b,
		int hitResult ) {};
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

	struct MyData
	{
		BasicBullet *prev;
		BasicBullet *next;
		sf::Vector2<double> gravity;
		sf::Vector2<double> position;
		sf::Vector2<double> velocity;
		int slowCounter;
		bool active;
		int slowMultiple;
		int framesToLive;
		int frame;
		sf::Transform transform;
		int bounceCount;
	};

	BasicBullet *prev;
	BasicBullet *next;
	sf::Vector2<double> gravity;
	sf::Vector2<double> position;
	sf::Vector2<double> velocity;
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
	sf::Vector2<double> tempVel;
	Launcher *launcher;
	

	BasicBullet(int indexVA, BType bType, Launcher *launcher);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	void SetIndex(int ind);
	virtual void HandleEntrant(QuadTreeEntrant *qte);
	virtual void UpdatePrePhysics();
	virtual void Reset(V2d &pos,V2d &vel);

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
};

struct SinBullet : BasicBullet
{
	SinBullet(int indexVA, Launcher *launcher);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void Reset(
		sf::Vector2<double> &pos,
		sf::Vector2<double> &vel);

	SinBullet *prev;
	SinBullet *next;
	//int slowCounter;
	CollisionBox hurtBody;
	sf::Vector2<double> tempadd;
};

struct CopycatBullet : BasicBullet
{
	CopycatBullet(int indexVA, Launcher *launcher);
	void UpdatePrePhysics();
	void UpdatePhysics();

	//modified reset
	void Reset(
		sf::Vector2<double> &pos0,
		sf::Vector2<double> &pos1);

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
	struct MyData
	{
		BasicBullet *inactiveBullets;
		BasicBullet *activeBullets;
		//sf::Vector2<double> position;
		//sf::Vector2<double> facingDir;
	};

	BasicBullet *inactiveBullets;
	BasicBullet *activeBullets;
	sf::Vector2<double> position;
	sf::Vector2<double> facingDir;
	
	int bytesStoredPerBullet;
	int playerIndex;
	int bulletTilesetIndex;
	bool interactWithTerrain;
	sf::Vertex *bulletVA;
	BasicBullet::BType bulletType;
	LauncherEnemy *handler;
	Tileset *ts_bullet;
	bool drawOwnBullets;
	Session *sess;
	int totalBullets;
	int perShot;
	HitboxInfo *hitboxInfo;
	double bulletSpeed;
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
		Edge *e, sf::Vector2<double> &pos);
};


#endif