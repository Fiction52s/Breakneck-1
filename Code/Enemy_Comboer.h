#ifndef __ENEMY_Comboer_H__
#define __ENEMY_Comboer_H__

#include "Enemy.h"

struct ComboObject;

struct Comboer : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		S_FLOAT,
		S_SHOT,
		S_EXPLODE,
		S_Count
	};

	enum ComboerType
	{
		T_STRAIGHT,
		T_GRAVITY,
		T_REVERSEGRAVITY,
		T_BOUNCE,
		T_Count
	};

	Comboer(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, std::list<sf::Vector2i> &path, 
		bool loop, int p_level,
		ComboerType t );
	//Comboer(GameSession *owner, std::ifstream &is);
	SurfaceMover *mover;
	void HitTerrainAerial(Edge *, double);

	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void HandleNoHealth();
	void AdvanceTargetNode();
	CollisionBox &GetEnemyHitbox();

	double gravFactor;
	double maxFallSpeed;

	ComboObject *comboObj;

	ComboerType cType;
	V2d velocity;
	int shootFrames;
	int shootLimit;
	int hitLimit;
	int currHits;

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
};

#endif