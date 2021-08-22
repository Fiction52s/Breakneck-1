#ifndef __ENEMY_BLOCKER_H__
#define __ENEMY_BLOCKER_H__

#include "Enemy.h"
#include "EnemyChain.h"

struct CircleGroup;

struct BlockerChain;


struct BlockerChain : EnemyChain
{
	BlockerChain(ActorParams *ap);
	//void InitReadParams(ActorParams *params);
	void ReadParams(ActorParams *params);
	Tileset *GetTileset(int variation);
	Enemy *CreateEnemy(V2d &pos, int ind);
	void UpdateStartPosition(int ind, V2d &pos);
	void EnemyDraw(sf::RenderTarget *target);
	sf::Shader blockerShader;
	sf::Glsl::Vec4 paletteArray[16];
	int blockerType;
};


struct Blocker : Enemy, QuadTreeEntrant
{
	enum BlockerType : int
	{
		GREY,
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA,
		BLACK,
	};

	enum Action
	{
		WAIT,
		MALFUNCTION,
		HITTING,
		EXPLODE,
		Count
	};
	bool CountsForEnemyGate() { return false; }
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	bool IsFastDying();
	void ProcessState();
	Blocker( BlockerChain *bc, V2d &pos, int index);
	//void DrawMinimap(sf::RenderTarget *target);
	void ClearSprite();
	void UpdateSprite();
	void ResetEnemy();
	void ProcessHit();
	void IHitPlayer(int index);
	void SetStartPosition(V2d &pos);
	sf::FloatRect GetAABB();
	void UpdateHitboxes();
	int minimapCirclePoints;
	int minimapCircleRadius;

	
	BlockerChain *bc;

	

	//bool checkCol;

	int vaIndex;
};

//struct BlockerChain : Enemy
//{
//	enum Action
//	{
//		EXIST,
//		Count
//	};
//
//	
//	int bType;
//	void SetActionEditLoop();
//	void UpdateOnPlacement(ActorParams *ap);
//	void UpdateSpriteFromParams( ActorParams *ap );
//	//void UpdateOnPlacement(ActorParams *ap);
//	void UpdateParams(ActorParams *ap);
//	void SetLevel(int lev);
//	void AddToWorldTrees();
//	sf::FloatRect GetAABB();
//	void CreateBlockers();
//	void UpdateFromParams( ActorParams *ap, int numFrames);
//
//	sf::Vertex *va;
//	CircleGroup *circleGroup;
//	BlockerChain(ActorParams *ap);
//	~BlockerChain();
//	void DrawMinimap(sf::RenderTarget *target);
//	void EnemyDraw(sf::RenderTarget *target);
//	void SetZone(Zone *p_zone);
//	int GetNumCamPoints();
//	V2d GetCamPoint(int index);
//	void UpdatePhysics( int substep );
//	void UpdatePrePhysics();
//	void DebugDraw(sf::RenderTarget *target);
//	void ProcessState();
//	void UpdatePostPhysics();
//	void UpdateEnemyPhysics();
//
//	void UpdateFromPath( ActorParams *ap );
//
//	int liveFrames;
//	Blocker **blockers;
//	int numBlockers;
//
//	Tileset *ts;
//	void ResetEnemy();
//
//	bool checkCol;
//	
//	double spacing;
//	bool armored;
//
//	std::vector<sf::Vector2i> localPath;
//	std::vector<sf::Vector2i> globalPath;
//	std::vector<V2d> blockerOffsets;
//
//	ActorParams::RailMode railMode;
//};

#endif