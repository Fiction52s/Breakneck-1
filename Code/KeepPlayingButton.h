#ifndef __KEEPPLAYINGBUTTON_H__
#define __KEEPPLAYINGBUTTON_H__

#include <SFML\Graphics.hpp>

struct KeepPlayingButton
{
	enum Action
	{
		A_OFF,
		A_ON,
		A_Count
	};

	sf::Vertex bgQuad[4];
	sf::Vertex buttonQuad[4];
	sf::Text startText;
	sf::Vector2f topLeft;
	sf::Vector2f size;
	int action;

	KeepPlayingButton();
	void Reset();
	void Update();
	void SetCenter(sf::Vector2f center);
	void SetTopLeft(sf::Vector2f p_topLeft);
	void UpdateButtonIconsWhenControllerIsChanged();
	void Draw(sf::RenderTarget *target);
};

#endif