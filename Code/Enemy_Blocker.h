#ifndef __ENEMY_BLOCKER_H__
#define __ENEMY_BLOCKER_H__

#include "Enemy.h"

struct BlockerChain;

struct Blocker : Enemy, QuadTreeEntrant
{
	enum Action
	{
		WAIT,
		MALFUNCTION,
		HITTING,
		EXPLODE,
		Count
	};

	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	bool IsFastDying();
	void ProcessState();
	Blocker(BlockerChain *bc, sf::Vector2i &pos, int index);
	void DrawMinimap(sf::RenderTarget *target);
	void ClearSprite();
	void UpdateSprite();
	void ResetEnemy();
	void ProcessHit();
	void IHitPlayer(int index);

	int actionLength[Count];
	int animFactor[Count];

	Action action;
	HitboxInfo *hitboxInfo;

	int animationFactor;

	BlockerChain *bc;

	CollisionBody *hurtBody;
	CollisionBody *hitBody;

	bool checkCol;

	int vaIndex;
};

struct BlockerChain : Enemy
{
	enum BlockerType
	{
		NORMAL,
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA
	};

	sf::Vertex *va;

	BlockerChain(GameSession *owner,
		sf::Vector2i &pos, std::list<sf::Vector2i> &path,
		int bType, bool armored, int spacing = 0);
	void DrawMinimap(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdatePhysics( int substep );
	void UpdatePrePhysics();
	void DebugDraw(sf::RenderTarget *target);
	void ProcessState();
	void UpdatePostPhysics();
	void UpdateEnemyPhysics();
	int animationFactor;
	int liveFrames;
	Blocker **blockers;
	int numBlockers;

	Tileset *ts;
	void ResetEnemy();

	bool checkCol;
	

	bool armored;
	BlockerType bType;
};

#endif