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
	//browser = new MapBrowser( )
}

MapBrowserScreen::~MapBrowserScreen()
{
	delete browserHandler;
}

void MapBrowserScreen::Start()
{
	//browserHandler->chooser->StartRelative(".brknk", MapBrowser::SAVE, "Resources\\Maps\\CustomMaps");
	browserHandler->chooser->SetToWorkshop();
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
	//target->draw(bgQuad, 4, sf::Quads);
	browserHandler->chooser->panel->Draw(target);
}