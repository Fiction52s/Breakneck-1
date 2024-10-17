#ifndef __ENEMY_CURRENCY_GRID_H__
#define __ENEMY_CURRENCY_GRID_H__

#include "Enemy.h"

struct CircleGroup;
//struct CurrencyGrid : Enemy
//{
//	enum Action
//	{
//		EXIST,
//		Count
//	};
//
//	struct MyData : StoredEnemyData
//	{
//
//	};
//	MyData data;
//
//	int numCurrencyTotal;
//
//	Tileset *ts;
//	bool checkCol;
//
//	double spacing;
//	int chainEnemyVariation;
//	int paramsVariation;
//	int paramsSpacing;
//
//	sf::Vertex *va;
//
//	CurrencyGrid(ActorParams *ap, EnemyType at);
//	~CurrencyGrid();
//	void ResetEnemy();
//	//virtual void InitReadParams(ActorParams *params) = 0;
//	virtual void ReadParams(ActorParams *params) = 0;
//	virtual Tileset *GetTileset(int variation) = 0;
//	virtual Enemy *CreateEnemy(V2d &pos, int ind) = 0;
//	virtual void UpdateStartPosition(int ind, V2d &pos) = 0;
//	virtual void SetKnockbackDirs() {}
//	void SetActionEditLoop();
//	void UpdateOnPlacement(ActorParams *ap);
//	void UpdateSpriteFromParams(ActorParams *ap);
//	//void UpdateOnPlacement(ActorParams *ap);
//	void UpdateParams(ActorParams *ap);
//	void AddToWorldTrees();
//	sf::FloatRect GetAABB();
//	void CreateEnemies();
//	void UpdateFromParams(ActorParams *ap, int numFrames);
//	void DrawMinimap(sf::RenderTarget *target);
//	virtual void EnemyDraw(sf::RenderTarget *target);
//	void SetZone(Zone *p_zone);
//	int GetNumCamPoints();
//	V2d GetCamPoint(int index);
//	void UpdateCustomPhysics(int substep);
//	void UpdatePrePhysics();
//	void DebugDraw(sf::RenderTarget *target);
//	void ProcessState();
//	void UpdatePostPhysics();
//	void UpdateEnemyPhysics();
//	void UpdateStartPositions(V2d &pos);
//	void ResetCheckCollisions();
//	void UpdateFromPath(ActorParams *ap);
//	void AddToGame();
//
//	int GetNumStoredBytes();
//	void StoreBytes(unsigned char *bytes);
//	void SetFromBytes(unsigned char *bytes);
//};

#endif