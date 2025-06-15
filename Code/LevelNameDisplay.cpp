#include "LevelNameDisplay.h"
#include "MainMenu.h"
#include "GameSession.h"

using namespace sf;
using namespace std;

LevelNameDisplay::LevelNameDisplay()
{
	MainMenu *mm = MainMenu::GetInstance();
	nameText.setFont(mm->arial);
	nameText.setCharacterSize(100);
	nameText.setFillColor(Color::White);

	textColor = Color::White;
	position = Vector2f(960, 540);

	nameText.setPosition(position);

	actionLength[APPEAR] = 30;
	actionLength[HOLD] = 30;
	actionLength[DISAPPEAR] = 30;
	actionLength[DONE] = 1;
}

void LevelNameDisplay::Reset()
{
	action = APPEAR;
	frame = 0;
	nameText.setFillColor(Color::Transparent);

	if (GameSession::GetSession()->hasRespawned)
	{
		action = DONE;
		frame = 0;
	}
}

void LevelNameDisplay::SetString(const std::string &str)
{
	nameText.setString(str);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2, nameText.getLocalBounds().top + nameText.getLocalBounds().height / 2);
}

void LevelNameDisplay::Update()
{
	if (action == DONE)
		return;

	if (frame == actionLength[action])
	{
		switch (action)
		{
		case APPEAR:
			action = HOLD;
			frame = 0;
			break;
		case HOLD:
			action = DISAPPEAR;
			frame = 0;
			break;
		case DISAPPEAR:
			action = DONE;
			frame = 0;
			break;
		case DONE:
			frame = 0;
			break;
		}
	}
	
	float f = 0;
	switch (action)
	{
	case APPEAR:
		f = (float)frame / actionLength[APPEAR];
		nameText.setFillColor(Color(textColor.r, textColor.g, textColor.b, 255 * f));
		break;
	case HOLD:
		nameText.setFillColor(Color(textColor.r, textColor.g, textColor.b, 255));
		break;
	case DISAPPEAR:
		f = (float)frame / actionLength[APPEAR];
		f = 1.f - f;
		nameText.setFillColor(Color(textColor.r, textColor.g, textColor.b, 255 * f));
		break;
	case DONE:
		break;
	}

	nameText.setPosition(position);

	++frame;
}

void LevelNameDisplay::Draw(sf::RenderTarget *target)
{
	if (action == DONE)
		return;

	target->draw(nameText);

}