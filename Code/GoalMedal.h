#ifndef __GOAL_MEDAL_H__
#define __GOAL_MEDAL_H__

#include "SFML/Graphics.hpp"
#include "Tileset.h"
#include "VectorMath.h"
#include "Enemy_Goal.h"

struct AbsorbParticles;
struct Session;

struct GoalMedal
{
	enum Action
	{
		A_IDLE,
		A_RISE,
		A_HOLD,
		A_DISPERSE,
		A_HIDE,
		A_Count
	};

	sf::Sprite sprite;
	Tileset *ts;
	sf::Vector2f position;
	Goal *myGoal;
	int action;
	int frame;
	int animFactor[A_Count];
	int actionLength[A_Count];
	CubicBezier risingBez;
	Session *sess;
	int medalRank;
	
	AbsorbParticles *particles;

	GoalMedal();
	~GoalMedal();
	void Reset();
	void Update();
	void Rise();
	void SetMedalRank(int r);
	void Disperse();
	bool IsDone();
	void SetPosition(sf::Vector2f pos);
	void Draw(sf::RenderTarget *target);
};

#endif