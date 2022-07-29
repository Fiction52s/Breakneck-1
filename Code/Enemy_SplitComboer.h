#ifndef __ENEMY_SPLITCOMBOER_H__
#define __ENEMY_SPLITCOMBOER_H__

#include "Enemy.h"

struct ComboObject;
struct SplitComboer;

struct SplitPiece : Enemy
{
	enum Action
	{
		S_FLY,
		S_EXPLODE,
		S_Count
	};

	struct MyData : StoredEnemyData
	{
		V2d velocity;
		int currHits;
	};
	MyData data;

	SplitComboer *sc;
	int hitLimit;

	SplitPiece(SplitComboer *splitComb);
	~SplitPiece();
	void SetLevel(int lev);
	bool CountsForEnemyGate() { return false; }
	void ComboHit();
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void Shoot(V2d dir);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};


struct SplitComboer : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_SPLIT,
		S_Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	const static int NUM_PIECES = 2;

	SplitPiece *pieces[NUM_PIECES];

	BasicPathFollower pathFollower;
	double acceleration;
	double speed;
	Tileset *ts;
	double shootSpeed;

	SplitComboer(ActorParams *ap);
	~SplitComboer();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);
	void CheckedMiniDraw(sf::RenderTarget *target,
		sf::FloatRect &rect);
	void SetZoneSpritePosition();
	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif