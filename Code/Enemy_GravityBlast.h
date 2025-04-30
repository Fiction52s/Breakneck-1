#ifndef __ENEMY_GRAVITYBLAST_H_
#define __ENEMY_GRAVITYBLAST_H_

#include "Enemy.h"

struct ComboObject;

struct GravityBlast : Enemy
{
	enum Action
	{
		S_EXPLODE,
		S_Count
	};

	struct MyData : StoredEnemyData
	{
		int currHits;
	};
	MyData data;

	int hitLimit;

	Tileset *ts;

	sf::CircleShape testCircle;

	void SetLevel(int lev);

	GravityBlast(Actor *player);
	~GravityBlast();

	void Activate(int playerIndex, V2d pos, V2d dir);

	void ProcessState();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void ComboKill(Enemy *e);
	bool IsActive();
	void DirectKill();
	void DrawMinimap(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	bool CountsForEnemyGate() { return false; }
	bool IsValidTrackEnemy() { return false; }
	bool IsHomingTarget(int trackingType) { return false; }

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);


};

#endif