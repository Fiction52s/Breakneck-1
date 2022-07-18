#include "MapBrowserScreen.h"
#include <iostream>
#include "MainMenu.h"
#include "MapBrowser.h"

using namespace std;
using namespace sf;

MapBrowserScreen::MapBrowserScreen(MainMenu *mm)
{
	mainMenu = mm;
	browserHandler = new DefaultMapBrowserHandler;

	SetRectColor(bgQuad, Color::Blue);
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	//SetRectColor(downloadingQuad, Color::Red);
	//SetRectCenter(downloadingQuad, 300, 300, Vector2f(960, 540));
}

MapBrowserScreen::~MapBrowserScreen()
{
	delete browserHandler;
}

void MapBrowserScreen::StartLocalBrowsing()
{
	browserHandler->chooser->StartRelative(".brknk", MapBrowser::OPEN, "Resources\\Maps");
}

void MapBrowserScreen::StartWorkshopBrowsing()
{
	browserHandler->chooser->StartWorkshop();
}

bool MapBrowserScreen::HandleEvent(sf::Event ev)
{
	return browserHandler->chooser->panel->HandleEvent(ev);
}

void MapBrowserScreen::Update()
{
	/*ControllerState &menuCurrInput = mainMenu->menuCurrInput;
	ControllerState &menuPrevInput = mainMenu->menuPrevInput;

	if (menuCurrInput.B && !menuPrevInput.B)
	{
		mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN);
	}*/

	browserHandler->Update();

	


	if (MOUSE.IsMouseRightClicked())
	{
		mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN);
	}

	//cout << MOUSE.GetPos().x << endl;
}

void MapBrowserScreen::Draw(sf::RenderTarget *target)
{
	/*if (browserHandler->chooser->action == MapBrowser::A_WAITING_FOR_MAP_DOWNLOAD)
	{
		SetRectColor(downloadingQuad, Color::Red);
	}*/
	//target->draw(bgQuad, 4, sf::Quads);
	browserHandler->chooser->panel->Draw(target);
}