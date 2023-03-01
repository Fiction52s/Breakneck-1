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
	};

	int action;
	int frame;

	sf::Text alertText;

	sf::Vertex boxQuad[4];

	AlertBox();

	void Update();
	void Start(const std::string &msg);
	void Draw(sf::RenderTarget *target);
};

#endif