#ifndef __ENEMY_WIRETARGET_H__
#define __ENEMY_WIRETARGET_H__

#include "Enemy.h"

struct ComboObject;

struct WireTarget : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_DESTROY,
		S_Count
	};

	WireTarget(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, int p_level);
	~WireTarget();
	void ProcessState();
	void ProcessHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	bool CanBeHitByWireTip(bool red);
	bool CanBeHitByComboer();
	bool CanBeAnchoredByWire(bool red);

	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];

	V2d origPos;

	sf::Sprite sprite;
	Tileset *ts;
};

#endif