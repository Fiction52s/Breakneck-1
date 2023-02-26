#include "MapBrowserScreen.h"
#include <iostream>
#include "MainMenu.h"
#include "MapBrowser.h"
#include "WorkshopMapPopup.h"
#include "globals.h"

using namespace std;
using namespace sf;

MapBrowserScreen::MapBrowserScreen(MainMenu *mm)
{
	mainMenu = mm;
	browserHandler = new MapBrowserHandler(5, 5, true );

	SetRectColor(bgQuad, Color::Blue);
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	//SetRectColor(downloadingQuad, Color::Red);
	//SetRectCenter(downloadingQuad, 300, 300, Vector2f(960, 540));
}

MapBrowserScreen::~MapBrowserScreen()
{
	delete browserHandler;
}

void MapBrowserScreen::TurnOff()
{
	browserHandler->chooser->TurnOff();
	CONTROLLERS.SetKeyboardActiveAsController(true);
}

void MapBrowserScreen::StartLocalBrowsing( int mode )
{
	MapBrowser::Mode m = (MapBrowser::Mode)mode;
	browserHandler->confirmedMapFilePath = "";
	browserHandler->chooser->StartRelative(MAP_EXT, m, "Resources\\Maps");
	browserHandler->ClearFocus();

	browserHandler->chooser->panel->tabGroups["tabs"]->SelectTab(0);

	//dont want this because of searching!
	CONTROLLERS.SetKeyboardActiveAsController(false);
	//browserHandler->chooser->
}

void MapBrowserScreen::StartWorkshopBrowsing( int mode )
{
	browserHandler->confirmedMapFilePath = "";
	browserHandler->chooser->StartWorkshop((MapBrowser::Mode)mode );
	browserHandler->ClearFocus();

	

	CONTROLLERS.SetKeyboardActiveAsController(false);
}

bool MapBrowserScreen::HandleEvent(sf::Event ev)
{
	return browserHandler->chooser->panel->HandleEvent(ev);
}

bool MapBrowserScreen::IsCancelled()
{
	return (browserHandler->chooser->action == MapBrowser::A_CANCELLED);
}

bool MapBrowserScreen::IsConfirmed()
{
	return (browserHandler->chooser->action == MapBrowser::A_CONFIRMED);
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