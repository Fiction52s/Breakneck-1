#ifndef __CLOSED_BETA_SCREEN_H_
#define __CLOSED_BETA_SCREEN_H_

#include <SFML/Graphics.hpp>

struct ClosedBetaScreen
{
	enum Action
	{
		A_IDLE,
		A_DONE,
	};

	int action;
	int frame;

	sf::Vertex quad[4];
	sf::Text text;

	ClosedBetaScreen();
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif