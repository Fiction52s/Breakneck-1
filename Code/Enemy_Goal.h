#ifndef __ENEMY_GOAL_H__
#define __ENEMY_GOAL_H__

#include "Enemy.h"

struct ActorParams;

struct Goal : Enemy
{
	enum Action
	{
		A_SITTING,
		A_KINKILLING,
		A_EXPLODING,
		A_DESTROYED,
		A_Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	int explosionLength;
	int explosionAnimFactor;
	int explosionYOffset;
	int initialYOffset;

	sf::Sprite miniSprite;
	Tileset *ts;
	Tileset *ts_mini;
	Tileset *ts_explosion;
	Tileset *ts_explosion1;
	float goalKillStartZoom;
	sf::Vector2f goalKillStartPos;

	int deathFrame;
	int animationFactor;
	V2d gn;

	Goal(ActorParams *ap);
	~Goal();
	bool CanBeHitByComboer() { return false; }
	bool CanBeHitByWireTip(bool red) { return false; }
	bool CanBeAnchoredByWire(bool red) { return false; }
	void SetMapGoalPos();
	bool CountsForEnemyGate() { return false; }
	void DrawMinimap(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void ProcessState();
	void ConfirmKill();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};


#endif