#ifndef __ENEMY_BASICEFFECT_H__
#define __ENEMY_BASICEFFECT_H__

#include "Enemy.h"

struct BasicEffect : Enemy
{
	BasicEffect(GameSession *owner);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void ResetEnemy();
	void HandleNoHealth();
	void ProcessState();
	void Init(Tileset *ts,
		sf::Vector2<double> position,
		double angle, int frameCount,
		int animationFactor, bool right,
		float depth);
	int GetNumCamPoints() { return 0; }
	bool CanTouchSpecter();
	float depth;
	int frameCount;
	sf::Sprite s;
	Tileset *ts;
	bool pauseImmune;
	bool activated;
	int animationFactor;
	bool facingRight;
	EffectLayer layer;
	int stored_frame;
	int startFrame;
	//sf::Vector2<double> position;
};

#endif