#include "AlertBox.h"
#include "VectorMath.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

AlertBox::AlertBox()
{
	SetRectColor(boxQuad, Color::Black);

	float boxWidth = 200;
	float boxHeight = 100;

	Vector2f topLeft(1920 - boxWidth, 1080 - boxHeight);

	SetRectTopLeft(boxQuad, boxWidth, boxHeight, topLeft);

	MainMenu *mainMenu = MainMenu::GetInstance();
	alertText.setFont(mainMenu->arial);
	alertText.setCharacterSize(20);
	alertText.setPosition(topLeft);
	alertText.setFillColor(Color::White);

	action = A_HIDDEN;
	frame = 0;
}

void AlertBox::Start(const std::string &msg)
{
	alertText.setString(msg);
	action = A_SLIDE_IN;
	frame = 0;
}

void AlertBox::Update()
{
	if (action == A_SLIDE_IN && frame == 30)
	{
		action = A_DISPLAY;
		frame = 0;
	}
	else if (action == A_SLIDE_OUT && frame == 30)
	{
		action = A_HIDDEN;
		frame = 0;
	}
	else if (action == A_DISPLAY && frame == 60)
	{
		action = A_SLIDE_OUT;
		frame = 0;
	}

	++frame;
}

void AlertBox::Draw(sf::RenderTarget *target)
{
	if (action == A_HIDDEN)
		return;

	target->draw(boxQuad, 4, sf::Quads);

	target->draw(alertText);
}