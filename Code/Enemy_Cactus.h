#ifndef __ENEMY_CACTUS_H__
#define __ENEMY_CACTUS_H__

#include "Enemy.h"
#include "ObjectPool.h"
#include "Bullet.h"

struct Cactus;
struct CactusShotgun : Enemy, LauncherEnemy, PoolMember
{
	enum Action
	{
		CHASINGPLAYER,
		BLINKING,
		SHOOTING,
		PUSHBACK,
		STASIS,
		EXPLODING,
		A_Count,
	};

	CactusShotgun(Cactus *parent, 
		ObjectPool *pool, int poolIndex);
	
	void ActionEnded();
	void ClearSprite();
	void ProcessState();
	
	
	void UpdateEnemyPhysics();
	void HandleRemove();
	void HandleNoHealth();
	void UpdateSprite();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		V2d &pos);
	void BulletHitPlayer(int playerIndex, 
		BasicBullet *b, int hitResult);
	void DirectKill();
	void ResetEnemy();
	void SetParams(V2d &position, V2d &dir);

	sf::Vertex *va;

	Cactus *parent;

	V2d velocity;

	ObjectPool *myPool;
	double bulletSpeed;
};

struct Cactus : Enemy
{
	enum Action
	{
		IDLE,
		ACTIVE,
		SHOOT,
		A_Count,
	};

	Shield *shield;
	const static int MAX_SHOTGUNS = 16;
	CactusShotgun *shotgunArray[MAX_SHOTGUNS];
	Tileset *ts;
	Tileset *ts_shotgun;
	Tileset *ts_bulletExplode;
	
	ObjectPool *shotgunPool;
	sf::Vertex *shotgunVA;

	int framesWait;
	int firingCounter;
	double bulletSpeed;

	Cactus(ActorParams *ap );
	~Cactus();
	void SetLevel(int lev);
	void ProcessState();
	bool LaunchersAreDone();
	void ActionEnded();
	int SetLaunchersStartIndex(int ind);
	void UpdateEnemyPhysics();
	void FrameIncrement();
	//void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void DirectKill();
	void ResetEnemy();
	void ThrowShotgun();
	void Draw(sf::RenderTarget *target);

	
};

#endif