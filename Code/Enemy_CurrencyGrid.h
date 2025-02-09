#ifndef __ENEMY_CURRENCY_GRID_H__
#define __ENEMY_CURRENCY_GRID_H__

#include "Enemy.h"

struct CircleGroup;



struct CurrencyGrid : Enemy, QuadTreeCollider
{

	struct GridCurrencyInfo : QuadTreeEntrant
	{
		bool exists;
		int state;
		int frame;
		sf::Rect<double> myRect;
		CurrencyGrid *myGrid;
		V2d center;
		int index;
		int playerChasingIndex;

		GridCurrencyInfo();
		void Reset();
		void ProcessState();
		void HandleQuery(QuadTreeCollider * qtc);
		bool IsTouchingBox(const sf::Rect<double> &r);
		void UpdateSprite();
		void Collect( int pIndex );
	};

	enum Action
	{
		NEUTRAL,
		PLAYER_COLLECT,
		INACTIVE,
		Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	QuadTree *currencyTree;
	int numCurrencyTotal;
	Actor *currCheckingCollectionActor; //used during a frame not between frames, doesn't need to be stored I think
	int currencyType;

	sf::Vector2i gridSize;
	
	int gridDistance;

	int givenHealAmount;
	int givenCurrencyAmount;

	float currencyRad;


	Tileset *ts;
	bool checkCol;

	sf::Vertex *va;

	CollisionBody testBody;

	std::vector<GridCurrencyInfo> currencyInfoVec;

	CurrencyGrid(ActorParams *ap, EnemyType at, V2d pos, sf::Vector2i p_gridSize, std::vector<bool> & p_hasCurrencyVec, int p_currencyType );
	~CurrencyGrid();
	
	
	void CheckCollection(Actor *a);
	void ResetEnemy();
	void HandleEntrant(QuadTreeEntrant *qte);
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
	V2d GetCurrencyStartPosition(int index);
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