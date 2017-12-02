#ifndef __ENEMY_CURVETURRET_H__
#define __ENEMY_CURVETURRET_H__

#include "Enemy.h"

struct CurveTurret : Enemy, LauncherEnemy
{
	CurveTurret(GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity,
		double bulletSpeed,
		int framesWait,
		sf::Vector2i &gravFactor,
		bool relativeGrav);
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
	void DirectKill();
	//void UpdateBulletHitboxes();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
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

#endif