#include "KeepPlayingButton.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

//sf::Vertex bgQuad[4];
//sf::Vertex buttonQuad[4];
//sf::Text startText;

KeepPlayingButton::KeepPlayingButton()
{
	MainMenu *mainMenu = MainMenu::GetInstance();

	startText.setFont(mainMenu->arial);
	startText.setCharacterSize(40);
	startText.setString("KEEP PLAYING");
	startText.setFillColor(Color::White);
	auto lb = startText.getLocalBounds();
	startText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);

	size.x = 500;
	size.y = 200;

	SetCenter(Vector2f(960, 540));

	Reset();

}
void KeepPlayingButton::Reset()
{
	action = A_OFF;
	SetRectColor(bgQuad, Color::Red);
}

void KeepPlayingButton::Update()
{
	if (CONTROLLERS.ButtonPressed_Start())
	{
		action = A_ON;
		SetRectColor(bgQuad, Color::Green);
	}
	else if (CONTROLLERS.ButtonPressed_B())
	{
		action = A_OFF;
		SetRectColor(bgQuad, Color::Red);
	}
}

void KeepPlayingButton::UpdateButtonIconsWhenControllerIsChanged()
{

}

void KeepPlayingButton::SetCenter(sf::Vector2f center)
{
	SetTopLeft(Vector2f(center.x - size.x / 2, center.y - size.y / 2));
}

void KeepPlayingButton::SetTopLeft(sf::Vector2f p_topLeft)
{
	topLeft = p_topLeft;

	Vector2f center = topLeft + Vector2f(size.x / 2, size.y / 2);

	startText.setPosition(center);

	SetRectTopLeft(bgQuad, size.x, size.y, topLeft);
}

void KeepPlayingButton::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	target->draw(startText);
}