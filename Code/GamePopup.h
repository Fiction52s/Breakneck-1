#ifndef __GAME_POPUP_H__
#define __GAME_POPUP_H__

#include <SFML\Graphics.hpp>
#include "Input.h"

struct GamePopup
{
	enum Options
	{
		OPTION_YES,
		OPTION_NO,
		OPTION_BACK,
		OPTION_NOTHING,
	};

	int currentlySelectedOption;

	sf::Vector2f size;
	sf::Vertex popupBGQuad[4];
	sf::Text optionText;
	sf::Vertex yesSelectedQuad[4];
	sf::Vertex noSelectedQuad[4];
	sf::Vertex backSelectedQuad[4];

	int numOptions;

	sf::Text yesText;
	sf::Text noText;
	sf::Text backText;
	sf::Text okText;
	sf::Vector2f position;

	GamePopup();
	void SetOption(int op);
	int Update(ControllerDualStateQueue *controllerInput);
	void SetText(const std::string &str);
	void SetPos(sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
	void SetNumOptions(int n);
	void SetInfo(const std::string &str, int numOptions, int initialOption = OPTION_YES);
};

#endif