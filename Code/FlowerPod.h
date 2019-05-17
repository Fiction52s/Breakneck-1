#ifndef __FLOWERPOD_H__
#define __FLOWERPOD_H__

#include "Enemy.h"


struct StorySequence;
struct FlowerPod;
struct TextDisp;
struct Script;
struct Ring;

struct MomentaBroadcast
{
	enum BroadcastType
	{
		SEESHARDS,
		DESTROYGOALS,
	};

	MomentaBroadcast( FlowerPod *pod, const std::string &btypeStr );
	~MomentaBroadcast();
	static BroadcastType GetType(const std::string &tStr);
	BroadcastType bType;
	Tileset *ts_broadcast;
	Tileset *ts_basicFlower;
	int initialFlowerLength;
	bool basicFlower;
	sf::Sprite sprite;
	FlowerPod *pod;
	int *imageLength;
	int numImages;
	bool Update();
	int imageIndex;
	int frame;
	void Reset();
	TextDisp *textDisp;
	Script *script;
	bool endPadding;
	int numPadding;
	void Draw(sf::RenderTarget *target);
};


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

	Ring *healRing;
	Actor *healingPlayer;
	MomentaBroadcast *broadcast;
	//StorySequence *storySeq;
	void HandleRayCollision(Edge *edge, double edgeQuantity, double rayPortion);
	Edge *rcEdge;
	V2d rayEnd;
	V2d rayStart;
	double rcQuantity;

	V2d camPosition;
	PodType podType;
	Action action;
	FlowerPod(GameSession *owner, const std::string &podType,
		Edge *ground, double quantity);
	~FlowerPod();
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
	Tileset *ts_flower;
	Tileset *ts_bud;
	Tileset *ts_rise;

	Edge *ground;
	double edgeQuantity;

	CollisionBody *hitBody;
};

#endif