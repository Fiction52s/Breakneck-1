#ifndef __ENEMY_OVERGROWTH_H__
#define __ENEMY_OVERGROWTH_H__

#include "Enemy.h"

struct Overgrowth;
struct Tree : Enemy, LauncherEnemy
{
	Tree(Overgrowth *parent, sf::VertexArray &va,
		Tileset *ts, int index);
	void SetParams(Edge *ground,
		double edgeQuantity);
	void ClearSprite();
	void UpdatePostPhysics();
	void UpdateSprite();


	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);

	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void DrawMinimap(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *target);
	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	bool PlayerSlowingMe();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	//void UpdateBulletHitboxes();
	//int NumTotalBullets();
	Overgrowth *parent;
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	int vaIndex;
	int frame;
	int animFactor;
	Edge *ground;

	bool active;
	sf::VertexArray &va;
	double edgeQuantity;
	Launcher *launcher;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	double angle;

	//Contact minContact;
	//bool col;
	//std::string queryMode;
	//int possibleEdgeCount;
	Tileset *ts;
	//int frame;
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

	int framesToLive;
	int maxFramesToLive;

	//sf::VertexArray &treeVA;

	//sf::Transform trans;
};

struct Overgrowth : Enemy
{


	Overgrowth(GameSession *owner,
		bool hasMonitor,
		Edge *ground, double quantity,
		double bulletSpeed, int lifeCycleFrames);
	//	void HandleEdge( Edge *e );
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *target);
	//bool IHitPlayer( int index = 0 );
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void InitTrees();
	void AddTree(Tree *tree);
	int NumTotalBullets();
	void DeactivateTree(Tree *tree);
	Tree * ActivateTree(Edge *g, double q);

	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	bool PlayerSlowingMe();

	int animationFactor;
	void AddToList(Tree *tree,
		Tree *&list);

	Edge *origGround;
	double origQuantity;

	Tileset *ts;
	Tree *activeTrees;
	Tree *inactiveTrees;
	//void UpdateBulletHitboxes();



	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	const static int MAX_TREES = 16;
	//const static int MAX_TREES = 16;
	sf::VertexArray treeVA;
};

#endif