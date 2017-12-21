#ifndef __ENEMY_CACTUS_H__
#define __ENEMY_CACTUS_H__

#include "Enemy.h"

struct Cactus : Enemy, LauncherEnemy
{
	Cactus(GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity,
		int p_bulletSpeed, int p_rhythm,
		int p_amplitude);
	//	void HandleEdge( Edge *e );
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *target);
	bool IHitPlayerWithBullets();
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	std::pair<bool, bool> PlayerHitMyBullets();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	//void UpdateBulletHitboxes();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
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

#endif