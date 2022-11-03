#include <assert.h>
#include "WorkshopBrowser.h"
#include "WorkshopMapPopup.h"
#include "MainMenu.h"
#include "MapBrowserScreen.h"
#include "MapBrowser.h"

using namespace sf;
using namespace std;

WorkshopBrowser::WorkshopBrowser()
{
	workshopMapPopup = new WorkshopMapPopup;
	mapBrowserScreen = MainMenu::GetInstance()->mapBrowserScreen;
	assert(mapBrowserScreen != NULL);
}

WorkshopBrowser::~WorkshopBrowser()
{
	delete workshopMapPopup;
}

void WorkshopBrowser::HandleEvent(sf::Event ev)
{
	switch (action)
	{
	case A_BROWSER:
	{
		switch (ev.type)
		{
		case Event::KeyPressed:
			if (ev.key.code == Keyboard::Escape)
			{
				Quit();
			}
			break;
		}
		mapBrowserScreen->HandleEvent(ev);
		break;
	}
	case A_POPUP:
	{
		workshopMapPopup->HandleEvent(ev);
		break;
	}
	}
}

void WorkshopBrowser::Start()
{
	action = A_BROWSER;
	frame = 0;
	mapBrowserScreen->browserHandler->ClearSelection();

	mapBrowserScreen->StartWorkshopBrowsing(MapBrowser::Mode::WORKSHOP);
}

void WorkshopBrowser::Quit()
{
	action = A_BACK;
	mapBrowserScreen->browserHandler->UnfocusFile();
	mapBrowserScreen->browserHandler->ClearSelection();
	mapBrowserScreen->browserHandler->chooser->ClearNodes();
}

void WorkshopBrowser::Update()
{
	switch (action)
	{
	case A_BROWSER:
	{
		mapBrowserScreen->Update();

		if (mapBrowserScreen->browserHandler->chooser->selectedRect != NULL)
		{
			MapNode *mp = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;

			workshopMapPopup->Activate(mp);
			action = A_POPUP;
		}
		break;
	}
	case A_POPUP:
	{
		workshopMapPopup->Update();

		switch (workshopMapPopup->action)
		{
		case WorkshopMapPopup::Action::A_BACK:
			action = A_BROWSER;
			//selectedMap = NULL;
			mapBrowserScreen->browserHandler->ClearSelection();
			break;
		case WorkshopMapPopup::Action::A_EDIT:
			break;
		case WorkshopMapPopup::Action::A_HOST:
			
			break;
		case WorkshopMapPopup::Action::A_PLAY:
			break;
		case WorkshopMapPopup::Action::A_SAVE:
			break;
		}
		break;
	}
	}
}

void WorkshopBrowser::Draw(sf::RenderTarget *target)
{
	switch (action)
	{
	case A_BROWSER:
	{
		mapBrowserScreen->Draw(target);
		break;
	}
	case A_POPUP:
	{
		mapBrowserScreen->Draw(target);
		sf::RectangleShape rect;
		rect.setFillColor(Color(0, 0, 0, 100));
		rect.setSize(Vector2f(1920, 1080));
		rect.setPosition(0, 0);
		target->draw(rect);
		workshopMapPopup->Draw(target);
		break;
	}
	}
}