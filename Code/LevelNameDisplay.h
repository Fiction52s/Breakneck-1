#ifndef __LEVEL_NAME_DISPLAY_H__
#define __LEVEL_NAME_DISPLAY_H__

#include <SFML\Graphics.hpp>

struct LevelNameDisplay
{
	enum Action
	{
		APPEAR,
		HOLD,
		DISAPPEAR,
		DONE,
		Count
	};

	int action;
	int frame;

	sf::Color textColor;

	int actionLength[Count];

	sf::Text nameText;
	sf::Vector2f position;

	LevelNameDisplay();
	void Reset();
	void SetString(const std::string &str);
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif