#ifndef __SINGLEINPUTMENUBUTTON_H__
#define __SINGLEINPUTMENUBUTTON_H__

#include <SFML\Graphics.hpp>
#include "Input.h"
#include "Tileset.h"

struct SingleInputMenuButton
{
	enum Action
	{
		A_OFF,
		A_ON,
		A_Count
	};

	sf::Vertex bgQuad[4];
	sf::Vertex buttonQuad[4];
	sf::Text offText;
	sf::Text onText;
	sf::Vector2f topLeft;
	sf::Vector2f size;
	Tileset *ts_buttons;
	int action;
	ControllerDualStateQueue *controllerStates;
	int button;
	float buttonSize;
	sf::Color offColor;
	sf::Color onColor;

	SingleInputMenuButton( const std::string &offStr, const std::string &onStr, 
		sf::Color offColor, sf::Color onColor, 
		sf::Color offTextColor, sf::Color onTextColor, sf::Vector2f p_size,
		XBoxButton p_button, float buttonSize );
	void Reset();
	void Update( bool onPress, bool offPress );
	void SetCenter(sf::Vector2f center);
	void SetTopLeft(sf::Vector2f p_topLeft);
	void UpdateButtonIconsWhenControllerIsChanged();
	void Draw(sf::RenderTarget *target);
};

#endif