#include <iostream>
#include "MainMenu.h"
#include "FreeplayScreen.h"
#include "UIMouse.h"

using namespace std;
using namespace sf;

FreeplayScreen::FreeplayScreen(MainMenu *mm)
{
	mainMenu = mm;

	SetRectColor(bgQuad, Color::Magenta);
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	Start();
}

FreeplayScreen::~FreeplayScreen()
{
}

void FreeplayScreen::Start()
{
	action = A_WAITING_FOR_PLAYERS;
}

void FreeplayScreen::Quit()
{

}

bool FreeplayScreen::HandleEvent(sf::Event ev)
{
	return false;//browserHandler->chooser->panel->HandleEvent(ev);
}

void FreeplayScreen::Update()
{
	if (MOUSE.IsMouseRightClicked())
	{
		action = A_READY;
	//mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN);
	}


	/*if (MOUSE.IsMouseRightClicked())
	{
		mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN);
	}*/
}

void FreeplayScreen::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
}