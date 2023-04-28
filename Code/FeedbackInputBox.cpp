#include "FeedbackInputBox.h"
#include "GameSession.h"
#include "MainMenu.h"

using namespace sf;
using namespace std;

FeedbackInputBox::FeedbackInputBox()
{
	SetRectColor(bgQuad, Color::Black);
	
	MainMenu *mainMenu = MainMenu::GetInstance();

	feedbackText.setFont(mainMenu->arial);
	feedbackText.setCharacterSize(30);
	feedbackText.setFillColor(Color::White);
	feedbackText.setString("Feedback");
	auto lb = feedbackText.getLocalBounds();
	feedbackText.setOrigin(0, lb.top + lb.height / 2);

	//ts_buttons = mainMenu->GetButtonIconTileset(ControllerType::CTYPE_KEYBOARD);
	ts_button = mainMenu->tilesetManager.GetSizedTileset("Menu/f1button_64x64.png");

	SetRectSubRect(buttonIcon, ts_button->GetSubRect(0));

	size = Vector2f(240, 70);
}

void FeedbackInputBox::SetTopLeft(Vector2f pos)
{
	SetRectTopLeft(bgQuad, size.x, size.y, pos);

	

	feedbackText.setPosition(pos + Vector2f(10, size.y / 2));

	float textRight = feedbackText.getGlobalBounds().left + feedbackText.getGlobalBounds().width;

	SetRectTopLeft(buttonIcon, 64, 64, Vector2f(textRight + 20, pos.y + size.y / 2 - 32));
}

void FeedbackInputBox::SetCenter(Vector2f pos)
{
	SetTopLeft(Vector2f(pos.x - size.x / 2, pos.y - size.y / 2));
}

void FeedbackInputBox::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	target->draw(buttonIcon, 4, sf::Quads, ts_button->texture);
	target->draw(feedbackText);
}