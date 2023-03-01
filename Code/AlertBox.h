#ifndef __ALERT_BOX_H__
#define __ALERT_BOX_H__

#include <SFML\Graphics.hpp>
#include <string>

struct AlertBox
{
	enum Action
	{
		A_HIDDEN,
		A_SLIDE_IN,
		A_DISPLAY,
		A_SLIDE_OUT,
		A_Count,
	};

	int action;
	int frame;

	sf::Text alertText;

	int actionLength[A_Count];
	sf::Vertex boxQuad[4];

	sf::Vector2f showPos;
	sf::Vector2f hidePos;
	float boxWidth;
	float boxHeight;

	AlertBox();

	void Update();
	void SetTopLeft(sf::Vector2f pos);
	void Start(const std::string &msg);
	void Draw(sf::RenderTarget *target);
};

#endif