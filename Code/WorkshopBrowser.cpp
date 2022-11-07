#include <assert.h>
#include "WorkshopBrowser.h"
#include "WorkshopMapPopup.h"
#include "MainMenu.h"
#include "MapBrowserScreen.h"
#include "MapBrowser.h"
#include "SavePopup.h"

using namespace sf;
using namespace std;

WorkshopBrowser::WorkshopBrowser()
{
	workshopMapPopup = new WorkshopMapPopup;
	savePopup = new SavePopup;
	saveMessagePopup = new MessagePopup;
	mapBrowserScreen = MainMenu::GetInstance()->mapBrowserScreen;
	assert(mapBrowserScreen != NULL);
}

WorkshopBrowser::~WorkshopBrowser()
{
	delete workshopMapPopup;
	delete savePopup;
	delete saveMessagePopup;
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
	case A_SAVE_POPUP:
	{
		savePopup->HandleEvent(ev);
		break;
	}
	case A_SAVE_MESSAGE:
	{
		saveMessagePopup->panel->HandleEvent(ev);
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
		{
			MapNode *mp = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;
			action = A_SAVE_POPUP;
			savePopup->Activate("Resources\\Maps\\WorkshopDownloads", mp->fileName );
			break;
		}
			
		}
		break;
	}
	case A_SAVE_POPUP:
	{
		savePopup->Update();


		if (savePopup->action == SavePopup::A_CONFIRMED)
		{
			if (savePopup->browserHandler->chooser->fileNameTextBox->GetString() == "")
			{
				//check all valid filenames here plz
				savePopup->action = SavePopup::A_ACTIVE;
			}
			else
			{
				action = A_SAVE_MESSAGE;
				saveMessagePopup->Pop("Successfully saved file");
				workshopMapPopup->action = WorkshopMapPopup::A_ACTIVE;
				savePopup->Deactivate();
			}
		}
		else if (savePopup->action == SavePopup::A_CANCELLED )
		{
			action = A_POPUP;
			workshopMapPopup->action = WorkshopMapPopup::A_ACTIVE;
			savePopup->Deactivate();
		}
		/*else if (savePopup->action == SavePopup::A_CONFIRMED)
		{

		}*/
		break;
	}
	case A_SAVE_MESSAGE:
	{
		saveMessagePopup->panel->MouseUpdate();

		if (saveMessagePopup->action == MessagePopup::A_INACTIVE)
		{
			action = A_POPUP;
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
	case A_SAVE_POPUP:
	{
		mapBrowserScreen->Draw(target);
		sf::RectangleShape rect;
		rect.setFillColor(Color(0, 0, 0, 100));
		rect.setSize(Vector2f(1920, 1080));
		rect.setPosition(0, 0);
		target->draw(rect);
		workshopMapPopup->Draw(target);
		target->draw(rect);
		savePopup->Draw(target);
		break;
	}
	case A_SAVE_MESSAGE:
	{
		mapBrowserScreen->Draw(target);
		sf::RectangleShape rect;
		rect.setFillColor(Color(0, 0, 0, 100));
		rect.setSize(Vector2f(1920, 1080));
		rect.setPosition(0, 0);
		target->draw(rect);
		workshopMapPopup->Draw(target);
		target->draw(rect);
		saveMessagePopup->Draw(target);
		break;
	}
	}
}