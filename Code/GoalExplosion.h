#ifndef __GOAL_EXPLOSION_H__
#define __GOAL_EXPLOSION_H__

#include <SFML/Graphics.hpp>

struct GameSession;

struct Ring
{
	Ring();
	void Draw(sf::RenderTarget *target);
	void UpdatePoints();
	void Set(sf::Vector2f pos,
		float innerR, float outerR);
	void SetColor(sf::Color c);
	void SetShader(sf::Shader *sh);
	sf::Vector2f position;
	float innerRadius;
	float outerRadius;
	sf::VertexArray circleVA;
	GameSession *owner;
	sf::Color color;
	sf::Shader *shader;
	const static int circlePoints;
};

struct GoalPulse
{
	GoalPulse( GameSession *owner,
		sf::Vector2f &pos );
	sf::VertexArray circleVA;
	GameSession *owner;
	void Draw( sf::RenderTarget *target );
	void Reset();
	void Update();
	void UpdatePoints();
	sf::Vector2f position;
	float innerRadius;
	float outerRadius;
	const static int circlePoints;

	int pulseLength;
	float maxOuterRadius;
	float minOuterRadius;
	float maxInnerRadius;
	float minInnerRadius;
	int frame;
	bool show;
};

#endif