#ifndef __ENEMY_GROWINGTREE_H__
#define __ENEMY_GROWINGTREE_H__

#include "Enemy.h"

struct GrowingTree : Enemy, LauncherEnemy
{
	enum Action
	{
		RECOVER0,
		RECOVER1,
		RECOVER2,
		LEVEL0,
		LEVEL0TO1,
		LEVEL1,
		LEVEL1TO2,
		LEVEL2,
		EXPLODE,
		Count
	};
	Action action;

	GrowingTree(GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity, int level );
	~GrowingTree();
	void ActionEnded();
	void ProcessState();
	//void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void Fire();
	void UpdateHitboxes();
	void ResetEnemy();
	//void HandleNoHealth();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		sf::Vector2<double> &pos);
	void DirectKill();
	void InitRangeMarkerVA();
	void BulletHitPlayer(BasicBullet *b);

	double pulseRadius;
	int pulseFrame;
	Tileset *ts_bulletExplode;
	int powerLevel;
	int totalBullets;

	sf::Sprite sprite;
	Tileset *ts;

	int startPowerLevel;
	Edge *ground;

	int numEnemiesKilledLastFrame;

	sf::Vertex *rangeMarkerVA;
	//sf::VertexArray rangeMarkerVA;
	double edgeQuantity;

	int actionLength[Action::Count];
	int animFactor[Action::Count];

	double angle;

	sf::Vector2<double> gn;
};

#endif