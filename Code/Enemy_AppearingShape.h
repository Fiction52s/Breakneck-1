#ifndef __ENEMY_APPEARING_SHAPE_H__
#define __ENEMY_APPEARING_SHAPE_H__

#include "Enemy.h"

struct AppearingShape;

struct AppearingShapePool
{
	AppearingShapePool();
	~AppearingShapePool();
	void Reset();
	AppearingShape * Appear(int type, 
		double size, V2d &pos);
	void DrawMinimap(sf::RenderTarget * target);
	std::vector<AppearingShape*> shapeVec;
	Tileset *ts;
	int numShapes;
};

struct AppearingShape : Enemy
{
	enum Shape
	{
		SHAPE_CIRCLE,
		SHAPE_SQUARE,
		SHAPE_Count
	};

	enum Action
	{
		NEUTRAL,
		APPEAR,
		HIT,
		DISAPPEAR,
		A_Count
	};

	Tileset *ts;
	
	int shapeType;

	double size;

	sf::Vertex *quads;

	int numCirclePoints;

	AppearingShape( AppearingShapePool *sp );
	~AppearingShape();

	void Appear( int shapeType, double size,
		V2d &pos);

	void ProcessState();
	void ActionEnded();
	void UpdateVertices();
	void EnemyDraw(sf::RenderTarget *target);

	void UpdateSprite();
	void SetColor(sf::Color c);
	void ResetEnemy();
};

#endif