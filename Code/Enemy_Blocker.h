#ifndef __ENEMY_BLOCKER_H__
#define __ENEMY_BLOCKER_H__

#include "Enemy.h"

struct CircleGroup;

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
	Blocker( BlockerChain *bc, sf::Vector2i &pos, int index);
	void DrawMinimap(sf::RenderTarget *target);
	void ClearSprite();
	void UpdateSprite();
	void ResetEnemy();
	void ProcessHit();
	void IHitPlayer(int index);
	void SetSpritePosition(V2d &pos);

	int minimapCirclePoints;
	int minimapCircleRadius;

	BlockerChain *bc;

	bool checkCol;

	int vaIndex;
};

struct BlockerChain : Enemy
{
	enum Action
	{
		EXIST,
		Count
	};

	enum BlockerType : int
	{
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA
	};

	void UpdateOnPlacement(ActorParams *ap);
	void UpdateSpriteFromEditParams();
	//void UpdateOnPlacement(ActorParams *ap);
	void UpdateParams(ActorParams *ap);
	void SetLevel(int lev);
	void AddToWorldTrees();
	sf::FloatRect GetAABB();
	void CreateBlockers();
	void UpdateFromEditParams(int numFrames);

	sf::Vertex *va;
	CircleGroup *circleGroup;
	BlockerChain(ActorParams *ap);
	~BlockerChain();
	void DrawMinimap(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void SetZone(Zone *p_zone);
	int GetNumCamPoints();
	V2d GetCamPoint(int index);
	void UpdatePhysics( int substep );
	void UpdatePrePhysics();
	void DebugDraw(sf::RenderTarget *target);
	void ProcessState();
	void UpdatePostPhysics();
	void UpdateEnemyPhysics();

	void UpdateFromPath( ActorParams *ap );

	int liveFrames;
	Blocker **blockers;
	int numBlockers;

	Tileset *ts;
	void ResetEnemy();

	bool checkCol;
	
	double spacing;
	bool armored;
	BlockerType bType;

	std::vector<sf::Vector2i> localPath;
	std::vector<sf::Vector2i> globalPath;
};

#endif