#ifndef __ENEMY_MOMENTUMBOOSTER_H__
#define __ENEMY_MOMENTUMBOOSTER_H__

#include "Enemy.h"

struct MomentumBooster : Enemy
{
	enum Action
	{
		NEUTRAL,
		BOOST,
		REFRESH,
		Count
	};

	struct MyData : StoredEnemyData
	{
		int flameFrame;
		float spinnerAngle;
	};
	MyData data;

	Tileset *ts;
	Tileset *ts_refresh;
	int strength;
	sf::Vertex quads[4 * 3];

	//flame
	//shine
	//rotating part

	int flameAnimFactor;

	MomentumBooster(ActorParams *ap);//sf::Vector2i &pos, int level);

	bool CountsForEnemyGate() { return false; }
	bool IsHomingTarget() { return false; }
	void ProcessState();
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	bool Boost();
	bool IsBoostable();
	void AddToWorldTrees();
	sf::FloatRect GetAABB();
	void FrameIncrement();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif