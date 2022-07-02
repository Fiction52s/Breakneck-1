#include "MainMenu.h"
#include "MenuPopup.h"

using namespace sf;
using namespace std;

MenuInfoPopup::MenuInfoPopup(MainMenu *mainMenu)
{
	size = Vector2f(500, 300);
	SetRectColor(popupBGQuad, Color::Black);

	text.setFont(mainMenu->arial);
	text.setCharacterSize(40);
	text.setFillColor(Color::White);
	text.setString("HELLO");

	SetPos(Vector2f(960, 540));

	forcedStayOpenFrames = 0;
}

void MenuInfoPopup::Pop(const std::string &str, int p_forcedStayOpenFrames )
{
	forcedStayOpenFrames = p_forcedStayOpenFrames;
	SetText(str);
}

bool MenuInfoPopup::Update(ControllerState &currInput,
	ControllerState &prevInput)
{
	if (forcedStayOpenFrames > 0)
	{
		--forcedStayOpenFrames;
		return false;
	}

	if ((currInput.A && !prevInput.A)
		|| (currInput.B && !prevInput.B)
		|| (currInput.Y && !prevInput.Y)
		|| (currInput.X && !prevInput.X)
		|| (currInput.rightShoulder && !prevInput.rightShoulder))
	{
		return true;
	}

	return false;
}

void MenuInfoPopup::SetPos(sf::Vector2f &pos)
{
	position = pos;
	SetRectCenter(popupBGQuad, size.x, size.y, pos);
	text.setPosition(pos);
}

void MenuInfoPopup::SetText(const std::string &str)
{
	text.setString(str);
	text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2,
		text.getLocalBounds().top + text.getLocalBounds().height / 2);
	size.x = text.getGlobalBounds().width + 40;
	size.y = text.getGlobalBounds().height + 40;
	SetPos(position);
}

void MenuInfoPopup::Draw(sf::RenderTarget *target)
{
	target->draw(popupBGQuad, 4, sf::Quads);
	target->draw(text);
}