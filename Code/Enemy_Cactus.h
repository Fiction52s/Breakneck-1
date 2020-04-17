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

	int actionLength[A_Count];
	int animFactor[A_Count];
	Action action;
	CactusShotgun(GameSession *owner, Cactus *parent, 
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
		sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
	void DirectKill();
	void ResetEnemy();
	void SetParams(V2d &position, V2d &dir);
	sf::Vertex *va;

	Cactus *parent;

	V2d velocity;
	Edge *ground;
	double edgeQuantity;

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

	int actionLength[A_Count];
	int animFactor[A_Count];

	Action action;
	Cactus(GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity, int level );
	void ProcessState();
	bool LaunchersAreDone();
	void ActionEnded();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	//void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void DirectKill();
	void ResetEnemy();
	void ThrowShotgun();
	void Draw(sf::RenderTarget *target);

	Shield *shield;

	sf::Sprite sprite;
	Tileset *ts;
	Tileset *ts_shotgun;
	Tileset *ts_bulletExplode;
	const static int MAX_SHOTGUNS = 16;
	ObjectPool *shotgunPool;
	sf::Vertex *shotgunVA;

	int framesWait;
	int firingCounter;
	Edge *ground;
	double edgeQuantity;


	double bulletSpeed;
};

#endif