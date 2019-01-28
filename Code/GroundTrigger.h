#ifndef __GROUNDTRIGGER_H__
#define __GROUNDTRIGGER_H__

#include "Enemy.h"
enum TriggerType
{
	TRIGGER_NEXUSCORE1,
	TRIGGER_SHIPPICKUP,
	TRIGGER_HOUSEFAMILY
};

struct StorySequence;

struct GroundTrigger : Enemy
{
	enum Action
	{
		IDLE,
		FOUND,
		DONE,
		Count
	};

	TriggerType trigType;
	Action action;
	int actionLength[Count];
	int animFactor[Count];

	StorySequence *storySeq;

	GroundTrigger(GameSession *owner,
		Edge *ground, double quantity, 
		bool facingRight,
		const std::string &trigTypeStr );
	void ProcessState();
	TriggerType GetTriggerType(const std::string &typeStr);
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void ResetEnemy();

	bool facingRight;
	sf::Sprite sprite;
	Tileset *ts;
	Edge *ground;
	double edgeQuantity;

};

#endif