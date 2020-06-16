#ifndef __GOALFLOW_H__
#define __GOALFLOW_H__

#include <SFML/Graphics.hpp>
#include <list>
#include "VectorMath.h"

struct Session;

struct GoalFlow
{
	GoalFlow(
		sf::Vector2f &gPos,
		std::list<std::list<std::pair<V2d, bool>>> &infoList);
	~GoalFlow();
	void Update( float camZoom,
		sf::Vector2f &topLeft );
	void Draw(sf::RenderTarget *target);
	
	sf::Shader flowShader;
	float flowRadius;
	int flowFrameCount;
	int flowFrame;
	float maxFlowRadius;
	float radDiff;
	float flowSpacing;
	float maxFlowRings;
	sf::Vector2f goalPos;
	sf::VertexArray * goalEnergyFlowVA;

private:
	void Setup(
		sf::Vector2f &gPos,
		std::list<std::list<std::pair<V2d, bool>>> &infoList);
	void SetGoalPos(sf::Vector2f &gPos);
	Session *sess;
};

#endif