#include <assert.h>
#include "WorkshopBrowser.h"
#include "WorkshopMapPopup.h"
#include "MainMenu.h"
#include "MapBrowserScreen.h"
#include "MapBrowser.h"
#include "SavePopup.h"
#include "UIController.h"

using namespace sf;
using namespace std;

WorkshopBrowser::WorkshopBrowser()
{
	workshopMapPopup = new WorkshopMapPopup;
	savePopup = new SavePopup;
	saveMessagePopup = new MessagePopup;
	mapBrowserScreen = MainMenu::GetInstance()->mapBrowserScreen;
	assert(mapBrowserScreen != NULL);
	editAfterSaving = false;
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
	
	mapBrowserScreen->browserHandler->Clear();
}

void WorkshopBrowser::RequestPreviews()
{
}

void WorkshopBrowser::Update()
{
	switch (action)
	{
	case A_BROWSER:
	{
		if (UICONTROLLER.IsCancelPressed())
		{
			Quit();
			break;
		}

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
		{
			action = A_BROWSER;
			//selectedMap = NULL;
			mapBrowserScreen->browserHandler->ClearSelection();
			break;

			MapNode *mp = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;

			savePopup->Activate("Resources\\Maps\\WorkshopDownloads", mp->fileName);
			break;
		}
		case WorkshopMapPopup::Action::A_HOST:
			
			break;
		case WorkshopMapPopup::Action::A_PLAY:
		{
			MainMenu *mm = MainMenu::GetInstance();

			//mapBrowserScreen->browserHandler->chooser->ClearAllPreviewsButSelected();
			workshopMapPopup->action = WorkshopMapPopup::A_ACTIVE;
			mm->DownloadAndRunWorkshopMap();
			break;
		}
			
		case WorkshopMapPopup::Action::A_SAVE_AND_EDIT:
		case WorkshopMapPopup::Action::A_SAVE:
		{
			MapNode *mp = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;

			if (mp->CheckIfFullyInstalled())
			{
				if (workshopMapPopup->action == WorkshopMapPopup::Action::A_SAVE_AND_EDIT)
				{
					editAfterSaving = true;
				}
				else
				{
					editAfterSaving = false;
				}

				action = A_SAVE_POPUP;
				savePopup->Activate("Resources\\Maps\\WorkshopDownloads", mp->fileName);
			}
			else
			{
				if (!mp->IsSubscribed())
				{
					mp->Subscribe();
				}
			}
			
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
			string fileNameStr = savePopup->browserHandler->chooser->fileNameTextBox->GetString();
			if ( fileNameStr == "")
			{
				//check all valid filenames here plz
				savePopup->action = SavePopup::A_ACTIVE;
			}
			else
			{
				MapNode *mp = (MapNode*)mapBrowserScreen->browserHandler->chooser->selectedRect->info;

				string copyLocation = savePopup->browserHandler->chooser->currPath.string() + "\\" + fileNameStr;
				string mapCopy = copyLocation + ".brknk";
				string previewCopy = copyLocation + ".png";

				string filePath = mp->filePath.string();
				string previewPath = mp->folderPath.string() + "\\" + mp->fileName + ".png";

				bool error = false;
				try
				{
					if (boost::filesystem::copy_file(mp->filePath, mapCopy, boost::filesystem::copy_option::overwrite_if_exists))
					{
						if (boost::filesystem::exists(previewPath))
						{
							if (!boost::filesystem::copy_file(previewPath, previewCopy, boost::filesystem::copy_option::overwrite_if_exists))
							{
								error = true;
							}
						}
					}
					else
					{
						error = true;
					}
				}
				catch (boost::filesystem::filesystem_error &e)
				{
					error = true;
				}

				
				action = A_SAVE_MESSAGE;
				workshopMapPopup->action = WorkshopMapPopup::A_ACTIVE;
				savePopup->Deactivate();


				if (error)
				{
					saveMessagePopup->Pop("An unknown error occurred.");
					savedPath = "";
				}
				else
				{	
					saveMessagePopup->Pop("Successfully saved file");
					savedPath = mapCopy;
				}
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
			
			if (editAfterSaving)
			{
				if (savedPath == "")
				{
					workshopMapPopup->action = WorkshopMapPopup::A_ACTIVE;
				}
				else
				{
					workshopMapPopup->action = WorkshopMapPopup::A_EDIT;
				}
			}
			else
			{
				workshopMapPopup->action = WorkshopMapPopup::A_ACTIVE;
			}
			
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

void WorkshopBrowser::ClearAllPreviewsButSelected()
{
	mapBrowserScreen->browserHandler->chooser->ClearAllPreviewsButSelected();
	//workshopBrowser->mapBrowserScreen->browserHandler->ts_largePreview = NULL;
	//workshopBrowser->workshopMapPopup->ts_preview = NULL;
}