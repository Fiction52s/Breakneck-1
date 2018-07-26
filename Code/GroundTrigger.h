#ifndef __GROUNDTRIGGER_H__
#define __GROUNDTRIGGER_H__

#include "Enemy.h"
enum TriggerType
{
	TRIGGER_NEXUSCORE1,
	TRIGGER_SHIPPICKUP
};

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

	GroundTrigger(GameSession *owner,
		Edge *ground, double quantity, 
		bool facingRight,
		TriggerType t );
	void ProcessState();
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