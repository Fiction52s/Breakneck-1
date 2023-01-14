#include "MainMenu.h"

using namespace sf;
using namespace std;

CreditsMenuScreen::CreditsMenuScreen(MainMenu *p_mainMenu)
	:mainMenu(p_mainMenu)
{
	menuOffset = Vector2f(0, 0);
	ts_test = mainMenu->tilesetManager.GetTileset("Menu/power_icon_128x128.png", 128, 128);
	testSprite.setTexture(*ts_test->texture);
	testSprite.setPosition(menuOffset + Vector2f(500, 500));
}

void CreditsMenuScreen::Draw(sf::RenderTarget *target)
{
	target->draw(testSprite);
}

void CreditsMenuScreen::Update()
{
	if (CONTROLLERS.ButtonPressed_B())
	{
		mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN);
	}
}