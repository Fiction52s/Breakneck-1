#include "SingleInputMenuButton.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

//sf::Vertex bgQuad[4];
//sf::Vertex buttonQuad[4];
//sf::Text startText;

SingleInputMenuButton::SingleInputMenuButton(const std::string &offStr, const std::string &onStr,
	sf::Color p_offColor, sf::Color p_onColor, 
	sf::Color p_offTextColor, sf::Color p_onTextColor,
	sf::Vector2f p_size,
	XBoxButton p_button, float p_buttonSize)
{
	MainMenu *mainMenu = MainMenu::GetInstance();

	controllerStates = NULL;
	ts_buttons = NULL;

	onColor = p_onColor;
	offColor = p_offColor;

	offText.setFont(mainMenu->arial);
	offText.setCharacterSize(40);
	offText.setString(offStr);
	offText.setFillColor(p_offTextColor);
	auto lb = offText.getLocalBounds();
	offText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);

	onText.setFont(mainMenu->arial);
	onText.setCharacterSize(40);
	onText.setString(onStr);
	onText.setFillColor(p_onTextColor);
	lb = onText.getLocalBounds();
	onText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);

	buttonSize = p_buttonSize;

	size = p_size;

	button = p_button;

	UpdateButtonIconsWhenControllerIsChanged();

	SetTopLeft(Vector2f(0,0));

	Reset();

}

void SingleInputMenuButton::Reset()
{
	action = A_OFF;
	SetRectColor(bgQuad, offColor);
}

void SingleInputMenuButton::Update( bool onPress, bool offPress )
{
	if (onPress)
	{
		action = A_ON;
		SetRectColor(bgQuad, onColor);
	}
	else if (offPress)
	{
		action = A_OFF;
		SetRectColor(bgQuad, offColor);
	}
}

void SingleInputMenuButton::UpdateButtonIconsWhenControllerIsChanged()
{
	return;

	MainMenu *mainMenu = MainMenu::GetInstance();
	int cType = controllerStates->GetControllerType();

	ts_buttons = mainMenu->GetButtonIconTileset(cType);

	XBoxButton currButton = (XBoxButton)button;//XBoxButton::XBOX_L1;
	if (cType == CTYPE_GAMECUBE && currButton == XBoxButton::XBOX_L1)
	{
		currButton = XBoxButton::XBOX_L2;
	}
	IntRect ir = mainMenu->GetButtonIconTileForMenu(cType, currButton);
	SetRectSubRect(buttonQuad, ir);
}

void SingleInputMenuButton::SetCenter(sf::Vector2f center)
{
	SetTopLeft(Vector2f(center.x - size.x / 2, center.y - size.y / 2));
}

void SingleInputMenuButton::SetTopLeft(sf::Vector2f p_topLeft)
{
	topLeft = p_topLeft;

	Vector2f center = topLeft + Vector2f(size.x / 2, size.y / 2);

	offText.setPosition(center);
	onText.setPosition(center);

	SetRectTopLeft(bgQuad, size.x, size.y, topLeft);

	float actionTextBottom = offText.getGlobalBounds().top + offText.getGlobalBounds().height;

	SetRectTopLeft(buttonQuad, buttonSize, buttonSize, Vector2f(center.x - buttonSize / 2, actionTextBottom + 10));
}

void SingleInputMenuButton::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);

	if (action == A_ON)
	{
		target->draw(onText);
	}
	else
	{
		target->draw(offText);
	}
	

	if (ts_buttons != NULL)
	{
		target->draw(buttonQuad, 4, sf::Quads, ts_buttons->texture);
	}
	
}