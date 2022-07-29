#ifndef __CIRCLEGROUP_H__
#define __CIRCLEGROUP_H__

#include <SFML\Graphics.hpp>

struct CircleGroup
{
	int numCircles;
	sf::Vertex *va;
	float radius;
	sf::Color color;
	int pointsPerCircle;
	int numVerts;
	sf::Vector2f *circleCenters;

	CircleGroup(int num, int rad, sf::Color col, int pointsPerCircle);
	~CircleGroup();
	
	void SetColor(int index, sf::Color c);
	void ResetColor();
	void SetVisible(int index, bool vis);
	void SetPosition(int index, sf::Vector2f &pos);
	const sf::Vector2f GetPosition(int index);
	void Draw(sf::RenderTarget *target);
	float GetRadius();
	void ShowAll();
	void HideAll();
};

#endif