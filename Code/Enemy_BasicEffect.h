#ifndef __ENEMY_BASICEFFECT_H__
#define __ENEMY_BASICEFFECT_H__

#include "Enemy.h"

struct BasicEffect : Enemy
{
	BasicEffect(GameSession *owner);
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target);
	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics(sf::Vector2<double> vel);
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	void Init(Tileset *ts,
		sf::Vector2<double> position,
		double angle, int frameCount,
		int animationFactor, bool right,
		float depth);
	float depth;
	int frameCount;
	sf::Sprite s;
	Tileset *ts;
	int frame;
	bool pauseImmune;
	bool activated;
	int animationFactor;
	bool facingRight;
	EffectLayer layer;
	int stored_frame;
	//sf::Vector2<double> position;
};

#endif