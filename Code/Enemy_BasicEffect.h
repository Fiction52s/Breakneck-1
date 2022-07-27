#ifndef __ENEMY_BASICEFFECT_H__
#define __ENEMY_BASICEFFECT_H__

#include "Enemy.h"

struct PlayerSkinShader;

struct BasicEffect : Enemy
{
	BasicEffect();
	BasicEffect(BasicEffect &be);
	virtual ~BasicEffect();
	virtual void EnemyDraw(sf::RenderTarget *target);
	virtual void UpdateSprite();
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
	Tileset *ts;
	bool pauseImmune;
	bool activated;
	int animationFactor;
	EffectLayer layer;
	int stored_frame;
	int startFrame;
};

struct PlayerEffect : BasicEffect
{
	PlayerEffect();
	~PlayerEffect();
	void SetSkin(int index);
	void BlendSkins(int first, int second, float progress);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();

	PlayerSkinShader *playerShader;
};

#endif