#ifndef __BULLET_H__
#define __BULLET_H__

#include "VectorMath.h"
#include "QuadTree.h"
#include "Physics.h"


struct BasicBullet;
struct Edge;
struct Tileset;

struct LauncherEnemy
{
	virtual void BulletHitTerrain(BasicBullet *b,
		Edge *edge,V2d &pos) {};
	virtual void BulletHitPlayer(BasicBullet *b) {};
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

	BasicBullet(int indexVA, BType bType, Launcher *launcher);
	BType bulletType;
	BasicBullet *prev;
	BasicBullet *next;
	sf::Vector2<double> position;
	//CollisionBox hurtBody;
	CollisionBox physBody;
	CollisionBox hitBody;

	virtual void HandleEntrant(QuadTreeEntrant *qte);
	virtual void UpdatePrePhysics();
	virtual void Reset(
		sf::Vector2<double> &pos,
		sf::Vector2<double> &vel);

	void DebugDraw(sf::RenderTarget *target);

	virtual void UpdatePhysics();
	virtual void UpdateSprite();
	virtual void UpdatePostPhysics();
	virtual void ResetSprite();
	bool ResolvePhysics(
		sf::Vector2<double> vel);
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
	int bulletTilesetIndex;
	BasicBullet::BType bulletType;
	Launcher(LauncherEnemy *handler,
		BasicBullet::BType bulletType,
		GameSession *owner,
		int numTotalBullets,
		int bulletsPerShot,
		sf::Vector2<double> position,
		sf::Vector2<double> direction,
		double angleSpread,
		int maxFramesToLive,
		bool hitTerrain = true,
		int wavelength = 0,
		double amplitude = 0.0);
	Launcher(LauncherEnemy *handler,
		GameSession *owner,
		int maxFramesToLive);
	~Launcher();
	static double GetRadius(BasicBullet::BType bt);
	static sf::Vector2f GetOffset(BasicBullet::BType bt);
	//Launcher( LauncherEnemy *handler,
	bool interactWithTerrain;
	void CapBulletVel(double speed);

	void Reset();
	BasicBullet *inactiveBullets;
	BasicBullet *activeBullets;
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

	void SetDefaultCollision(int framesToLive,
		Edge *e, sf::Vector2<double> &pos);
	int def_framesToLive;
	Edge* def_e;
	bool skipPlayerCollideForSubstep;
	sf::Vector2<double> def_pos;
	//Launcher *next;
};

//struct Bullet : QuadTreeCollider
//{
//	Bullet(int indexVA, Launcher *launcher,
//		double radius);
//
//	virtual void HandleEntrant(QuadTreeEntrant *qte);
//	virtual void UpdatePrePhysics();
//	virtual void Reset(
//		sf::Vector2<double> &pos,
//		sf::Vector2<double> &vel);
//	virtual void UpdatePhysics();
//	virtual void UpdateSprite();
//	virtual void UpdatePostPhysics();
//	virtual void ResetSprite();
//	bool ResolvePhysics(
//		sf::Vector2<double> vel);
//	virtual bool HitTerrain();
//	void HitPlayer();
//
//
//	CollisionBox physBody;
//	CollisionBox hitBody;
//	CollisionBox hurtBody;
//	Bullet *prev;
//	Bullet *next;
//	sf::Vector2<double> position;
//	sf::Vector2<double> velocity;
//	int slowCounter;
//	int slowMultiple;
//	int framesToLive;
//	sf::Transform transform;
//	Tileset *ts;
//	int index;
//	bool col;
//	Contact minContact;
//	sf::Vector2<double> tempVel;
//	Launcher *launcher;
//};
//
//namespace BulletType
//{
//	enum Type
//	{
//		NORMAL,
//		SIN,
//		BIRDBOSS,
//		Count
//	};
//}
//Bullet* CreateBullet(BulletType::Type type, int vaIndex, Launcher *launcher);

#endif