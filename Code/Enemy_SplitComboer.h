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

	void SetLevel(int lev);
	SplitPiece(SplitComboer *splitComb);
	~SplitPiece();
	bool CountsForEnemyGate() { return false; }

	void ComboHit();

	void ProcessState();
	void UpdateEnemyPhysics();

	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void Shoot(V2d dir);

	SplitComboer *sc;

	V2d velocity;

	int currHits;
	int hitLimit;
};


struct SplitComboer : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_SPLIT,
		S_Count
	};

	void UpdateOnPlacement(ActorParams *ap);
	void UpdatePath();
	void SetLevel(int lev);

	SplitComboer(ActorParams *ap);
	//SplitComboer(sf::Vector2i pos, std::list<sf::Vector2i> &path, bool loop,
	//	int p_level);
	~SplitComboer();

	SplitPiece *pieces[2];

	void CheckedMiniDraw(sf::RenderTarget *target,
		sf::FloatRect &rect);
	void SetZoneSpritePosition();

	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();

	V2d velocity;

	BasicPathFollower pathFollower;
	double acceleration;
	double speed;
	Tileset *ts;

	double shootSpeed;
};

#endif