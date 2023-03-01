#include "AlertBox.h"
#include "VectorMath.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

AlertBox::AlertBox()
{
	SetRectColor(boxQuad, Color::Black);

	MainMenu *mainMenu = MainMenu::GetInstance();
	alertText.setFont(mainMenu->arial);
	alertText.setCharacterSize(20);
	
	alertText.setFillColor(Color::White);

	actionLength[A_HIDDEN] = 0;
	actionLength[A_SLIDE_IN] = 30;
	actionLength[A_DISPLAY] = 60 * 7;
	actionLength[A_SLIDE_OUT] = 30;

	//boxWidth = 400;
	//boxHeight = 50;

	action = A_HIDDEN;
	frame = 0;
}

void AlertBox::Start(const std::string &msg)
{
	alertText.setString(msg);

	boxWidth = alertText.getGlobalBounds().width + 20;
	boxHeight = alertText.getGlobalBounds().height + 20;

	showPos = Vector2f(1920 - boxWidth, 1080 - boxHeight);
	hidePos = Vector2f(1920 - boxWidth, 1080);

	action = A_SLIDE_IN;
	frame = 0;
	SetTopLeft(hidePos);
}

void AlertBox::Update()
{
	if (frame == actionLength[action])
	{
		switch (action)
		{
		case A_SLIDE_IN:
		{
			action = A_DISPLAY;
			frame = 0;
			SetTopLeft(showPos);
			break;
		}
		case A_SLIDE_OUT:
		{
			action = A_HIDDEN;
			frame = 0;
			SetTopLeft(hidePos);
			break;
		}
		case A_DISPLAY:
		{
			action = A_SLIDE_OUT;
			frame = 0;
			break;
		}
		}
	}

	switch (action)
	{
	case A_SLIDE_IN:
	{
		float a = frame / (float)actionLength[A_SLIDE_IN];
		Vector2f topLeft = hidePos * (1.f - a) + a * showPos;
		SetTopLeft(topLeft);
		break;
	}
	case A_SLIDE_OUT:
	{
		float a = frame / (float)actionLength[A_SLIDE_OUT];
		Vector2f topLeft = showPos * (1.f - a) + a * hidePos;
		SetTopLeft(topLeft);
		break;
	}
	}

	++frame;
}

void AlertBox::SetTopLeft(sf::Vector2f topLeft)
{
	alertText.setPosition(topLeft + Vector2f( 10, 10 ));
	SetRectTopLeft(boxQuad, boxWidth, boxHeight, topLeft);
}

void AlertBox::Draw(sf::RenderTarget *target)
{
	if (action == A_HIDDEN)
		return;

	target->draw(boxQuad, 4, sf::Quads);

	target->draw(alertText);
}