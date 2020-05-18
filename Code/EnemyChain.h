#ifndef __ENEMYCHAIN_H__
#define __ENEMYCHAIN_H__

#include "Enemy.h"

struct CircleGroup;
struct EnemyChain : Enemy
{
	enum Action
	{
		EXIST,
		Count
	};
	//virtual void InitReadParams(ActorParams *params) = 0;
	virtual void ReadParams(ActorParams *params) = 0;
	virtual Tileset *GetTileset(int variation) = 0;
	virtual Enemy *CreateEnemy(V2d &pos, int ind) = 0;
	virtual void UpdateStartPosition(int ind, V2d &pos) = 0;
	virtual void SetKnockbackDirs() {}
	void SetActionEditLoop();
	void UpdateOnPlacement(ActorParams *ap);
	void UpdateSpriteFromParams(ActorParams *ap);
	//void UpdateOnPlacement(ActorParams *ap);
	void UpdateParams(ActorParams *ap);
	void AddToWorldTrees();
	sf::FloatRect GetAABB();
	void CreateEnemies();
	void UpdateFromParams(ActorParams *ap, int numFrames);

	sf::Vertex *va;
	CircleGroup *circleGroup;
	EnemyChain(ActorParams *ap);
	~EnemyChain();
	void DrawMinimap(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void SetZone(Zone *p_zone);
	int GetNumCamPoints();
	V2d GetCamPoint(int index);
	void UpdatePhysics(int substep);
	void UpdatePrePhysics();
	void DebugDraw(sf::RenderTarget *target);
	void ProcessState();
	void UpdatePostPhysics();
	void UpdateEnemyPhysics();
	void UpdateStartPositions(V2d &pos);

	void UpdateFromPath(ActorParams *ap);

	int liveFrames;
	Enemy **enemies;
	int numEnemies;

	Tileset *ts;
	void ResetEnemy();

	bool checkCol;

	double spacing;
	int chainEnemyVariation;
	int paramsVariation;
	int paramsSpacing;

	std::vector<sf::Vector2i> localPath;
	std::vector<sf::Vector2i> globalPath;
	std::vector<V2d> enemyOffsets;

	ActorParams::RailMode railMode;
};

#endif