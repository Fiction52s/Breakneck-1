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

	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];


	SplitPiece(SplitComboer *splitComb);
	~SplitPiece();

	void ComboHit();

	void ProcessState();
	void UpdateEnemyPhysics();

	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void Shoot(V2d dir);

	SplitComboer *sc;

	bool facingRight;

	sf::Sprite sprite;

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

	SplitComboer(GameSession *owner,
		sf::Vector2i pos, std::list<sf::Vector2i> &path, bool loop,
		int p_level);
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
	void AdvanceTargetNode();

	V2d velocity;

	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;
	int targetNode;
	bool forward;
	double acceleration;
	double speed;
	int nodeWaitFrames;
	sf::Sprite sprite;
	Tileset *ts;
	bool facingRight;

	double shootSpeed;
};

#endif