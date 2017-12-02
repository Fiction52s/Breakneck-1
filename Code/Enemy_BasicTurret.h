#ifndef __ENEMY_BASICTURRET_H__
#define __ENEMY_BASICTURRET_H__

#include "Enemy.h"

struct BasicTurret : Enemy, LauncherEnemy
{
	BasicTurret(GameSession *owner, bool hasMonitor, Edge *ground, double quantity,
		double bulletSpeed,
		int framesWait);
	//	void HandleEdge( Edge *e );
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *target);
	bool IHitPlayer(int index = 0);
	bool IHitPlayerWithBullets();
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	std::pair<bool, bool> PlayerHitMyBullets();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void UpdateBulletHitboxes();
	void Setup();
	Tileset *ts_bulletExplode;
	int testSubstep;
	int frameTestCounter;

	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);

	CollisionBox prelimBox;
	bool playerPrelimHit[4];

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
	bool ResolvePhysics(Bullet *b, sf::Vector2<double> vel);

	void AddBullet();
	void DeactivateBullet(Bullet *bullet);
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

#endif