#ifndef __ENEMY_CORALNANOBOTS_H__
#define __ENEMY_CORALNANOBOTS_H__

#include "Enemy.h"

struct CoralNanobots;
struct CoralBlock : Enemy
{
	CoralBlock(CoralNanobots *parent,
		sf::VertexArray &va,
		Tileset *ts, int index);
	void SetParams(sf::Vector2<double> &pos,
		sf::Vector2<double> &dir,
		int iteration);
	void ClearSprite();
	void UpdatePostPhysics();
	void UpdateSprite();


	bool ResolvePhysics(sf::Vector2<double> &vel);

	void BlockHitTerrain(BasicBullet *b,
		Edge *edge,
		sf::Vector2<double> &pos);


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
	CoralNanobots *parent;
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	int vaIndex;
	int frame;
	int animFactor;
	//Edge *ground;

	bool active;
	std::string queryMode;
	bool topOpen;
	bool leftOpen;
	bool rightOpen;
	bool botOpen;

	int iteration;

	CubicBezier bez;
	sf::Vector2<double> startPos;
	sf::Vector2<double> direction;
	sf::Vector2<double> oldPos;
	MovementSequence move;
	sf::VertexArray &va;
	//double edgeQuantity;
	//Launcher *launcher;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	CollisionBox physBody;
	HitboxInfo *hitboxInfo;

	double angle;
	sf::Vector2<double> tempVel;
	sf::Vector2<double> dir;

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

	bool lockedIn;

	//int framesToLive;
	//int maxFramesToLive;

	//sf::VertexArray &blockVA;

	Contact minContact;
	bool col;
	//sf::Transform trans;
};

struct CoralNanobots : Enemy//, LauncherEnemy
{
	CoralNanobots(GameSession *owner,
		bool hasMonitor,
		sf::Vector2i &pos, int moveFrames);
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos);
	void BulletHitPlayer(BasicBullet *b);
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
	void InitBlocks();
	void AddBlock(CoralBlock *block);
	//int NumTotalBullets();
	void DeactivateBlock(CoralBlock *block);
	CoralBlock * ActivateBlock(
		sf::Vector2<double> &pos,
		sf::Vector2<double> &dir,
		int iteration);

	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	bool PlayerSlowingMe();

	int animationFactor;
	void AddToList(CoralBlock *block,
		CoralBlock *&list);

	//Edge *origGround;
	sf::Vector2<double> origPosition;
	//double origQuantity;

	int moveFrames;

	Tileset *ts;
	CoralBlock *activeBlocks;
	CoralBlock *inactiveBlocks;


	//void UpdateBulletHitboxes();


	int blockSizeX;
	int blockSizeY;
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	const static int MAX_BLOCKS = 25;
	Launcher *launcher;
	//const static int MAX_TREES = 16;
	sf::VertexArray blockVA;
};

#endif