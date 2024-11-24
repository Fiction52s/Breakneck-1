#ifndef __ENEMY_CURRENCY_GRID_H__
#define __ENEMY_CURRENCY_GRID_H__

#include "Enemy.h"

struct CircleGroup;
struct CurrencyGrid : Enemy
{
	enum Action
	{
		EXIST,
		Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	int numCurrencyTotal;

	sf::Vector2i gridSize;
	sf::Vector2i gridDist;

	int *currStates;
	int *currFrames;


	Tileset *ts;
	bool checkCol;

	sf::Vertex *va;

	std::vector<bool> hasCurrencyVec;

	CurrencyGrid(ActorParams *ap, EnemyType at, V2d pos, sf::Vector2i p_gridSize, sf::Vector2i p_gridDist, std::vector<bool> & p_hasCurrencyVec );
	~CurrencyGrid();
	void ResetEnemy();
	//virtual void InitReadParams(ActorParams *params) = 0;
	void SetOriginPosition(V2d p);
	virtual void SetKnockbackDirs() {}
	void SetActionEditLoop();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdateSpriteFromParams(ActorParams *ap);
	//void UpdateOnPlacement(ActorParams *ap);
	void UpdateParams(ActorParams *ap);
	void UpdateSprite();
	void AddToWorldTrees();
	sf::FloatRect GetAABB();
	void CreateEnemies();
	void UpdateFromParams(ActorParams *ap, int numFrames);
	void DrawMinimap(sf::RenderTarget *target);
	virtual void EnemyDraw(sf::RenderTarget *target);
	int GetNumCamPoints();
	V2d GetCamPoint(int index);
	void UpdatePrePhysics();
	void DebugDraw(sf::RenderTarget *target);
	void ProcessState();
	void UpdatePostPhysics();
	void UpdateEnemyPhysics();
	void UpdateStartPositions(V2d &pos);
	void ResetCheckCollisions();
	void UpdateFromPath(ActorParams *ap);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif