#ifndef __ENEMY_BASICEFFECT_H__
#define __ENEMY_BASICEFFECT_H__

struct Effect
{
	Effect(GameSession *owner);
	void Draw(sf::RenderTarget *target);
	void UpdateSprite();
	void Reset();
	void HandleNoHealth();
	void ProcessState();
	void Init(Tileset *ts,
		sf::Vector2<double> position,
		double angle, int frameCount,
		int animationFactor, bool right,
		float depth);
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