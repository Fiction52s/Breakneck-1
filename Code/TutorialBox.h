#ifndef __TUTORIALBOX_H__
#define __TUTORIALBOX_H__

#include <SFML\Graphics.hpp>
#include <string>

struct Session;

struct TutorialBox
{
	struct ButtonInfo
	{
		ButtonInfo()
		{
			buttonType = -1;
		}
		ButtonInfo(int p_buttonType, sf::Vector2f &p_pos)
		{
			buttonType = p_buttonType;
			pos = p_pos;
		}
		int buttonType;
		sf::Vector2f pos;

	};


	sf::Text text;
	sf::Vertex quad[4];
	Session *sess;

	const static int MAX_BUTTONS = 12;
	sf::Vertex buttonQuad[4 * MAX_BUTTONS];

	sf::Vector2f lockedSize;
	float rectBuffer;
	float charHeight;
	sf::Color quadColor;
	sf::Color textColor;
	bool topLeftMode;
	std::vector<std::string> buttonStrings;
	std::vector<ButtonInfo> buttonInfos;
	int currControllerType;

	TutorialBox(int charHeight, sf::Vector2f lockedSize, sf::Color quadColor, sf::Color textColor, float rectBuffer );
	TutorialBox();
	int NextButtonStringIndex(const std::string &s);
	bool CalcButtonPos(
		std::string &startString,
		const std::string &buttonStr,
		sf::Vector2f &buttonPos);
	void ClearButtons();
	void SetText(const std::string &str);
	void SetCenterPos(sf::Vector2f &pos);
	void SetTopLeft(sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
	void UpdateButtonIconsWhenControllerIsChanged();
};

#endif