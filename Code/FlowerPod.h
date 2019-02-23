#ifndef __FLOWERPOD_H__
#define __FLOWERPOD_H__

#include "Enemy.h"

struct StorySequence;

struct FlowerPod : Enemy, RayCastHandler
{
	enum Action
	{
		IDLE,
		ACTIVATE,
		BROADCAST,
		HIDE,
		DEACTIVATED,
		A_Count,
	};
	
	enum PodType
	{
		SEESHARDS,
	};

	int actionLength[A_Count];
	int animFactor[A_Count];

	StorySequence *storySeq;
	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
	Edge *rcEdge;
	V2d rayEnd;
	V2d rayStart;
	double rcQuantity;


	static PodType GetType(const std::string &tStr);
	PodType podType;
	Action action;
	FlowerPod(GameSession *owner, const std::string &podType,
		Edge *ground, double quantity);
	void ProcessState();
	void ActionEnded();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void DirectKill();
	void ResetEnemy();
	void IHitPlayer(int index = 0);

	sf::Sprite sprite;
	Tileset *ts;

	Edge *ground;
	double edgeQuantity;

	CollisionBody *hitBody;
};

#endif