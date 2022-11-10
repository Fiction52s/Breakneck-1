#include <iostream>
#include "MainMenu.h"
#include "MapBrowser.h"
#include "SavePopup.h"
#include <boost/filesystem.hpp>

using namespace std;
using namespace sf;

using namespace boost::filesystem;

SavePopup::SavePopup()
{
	//mainMenu = mm;
	browserHandler = new MapBrowserHandler(5, 4, false);
	overwritePopup = new ConfirmPopup;
	//SetRectColor(bgQuad, Color::Blue);
	//SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	//SetRectColor(downloadingQuad, Color::Red);
	//SetRectCenter(downloadingQuad, 300, 300, Vector2f(960, 540));
}

SavePopup::~SavePopup()
{
	delete overwritePopup;
	delete browserHandler;
}

bool SavePopup::Activate(const std::string &activationPath, const std::string &defaultFileName )
{
	action = A_ACTIVE;
	browserHandler->chooser->panel->SetCenterPos(Vector2i(960, 540));
	browserHandler->chooser->StartRelative(".brknk", MapBrowser::SAVE, activationPath);//"Resources\\Maps\\WorkshopDownloads");
	browserHandler->chooser->panel->tabGroups["tabs"]->HideMember();
	browserHandler->chooser->panel->tabGroups["tabs"]->SelectTab(0);

	browserHandler->chooser->fileNameTextBox->SetString(defaultFileName);

	return true;
}


void SavePopup::Deactivate()
{
	action = A_INACTIVE;
	browserHandler->ClearFocus();
	browserHandler->ClearSelection();
	browserHandler->chooser->ClearNodes();
}

bool SavePopup::HandleEvent(sf::Event ev)
{
	if (action == A_CHECK_OVERWRITE)
	{
		return overwritePopup->panel->HandleEvent(ev);
	}
	else
	{
		return browserHandler->chooser->panel->HandleEvent(ev);
	}
}

void SavePopup::Update()
{
	if (action == A_CHECK_OVERWRITE)
	{
		overwritePopup->panel->MouseUpdate();

		if (overwritePopup->action == ConfirmPopup::A_YES)
		{
			action = A_CONFIRMED;
		}
		else if( overwritePopup->action == ConfirmPopup::A_NO)
		{
			action = A_ACTIVE;
		}
	}
	else if( action == A_ACTIVE )
	{
		browserHandler->Update();

		if (browserHandler->chooser->action == MapBrowser::Action::A_CANCELLED)
		{
			action = A_CANCELLED;
			
		}
		else if (browserHandler->chooser->action == MapBrowser::Action::A_CONFIRMED)
		{
			string fileName = browserHandler->chooser->fileNameTextBox->GetString();
			fileName += ".brknk";

			
			path filePath = browserHandler->chooser->currPath.string() + "\\" + fileName;

			try
			{
				if (exists(filePath))
				{
					action = A_CHECK_OVERWRITE;
					overwritePopup->Pop();
					overwritePopup->SetQuestion(browserHandler->chooser->fileNameTextBox->GetString() + string(" already exists.\nDo you want to replace it?"));
				}
				else
				{
					action = A_CONFIRMED;
				}
			}
			catch (boost::filesystem::filesystem_error & e)
			{

			}
			
			browserHandler->chooser->action = MapBrowser::A_IDLE;
			
			//assert(browserHandler->chooser->selectedRect != NULL);
			//MapNode *mp = (MapNode*)browserHandler->chooser->selectedRect->info;
			
		}
	}
	

	
	//else if( browserHandler->chooser->action == MapBrowser::Action::A)
}

void SavePopup::Draw(sf::RenderTarget *target)
{
	/*if (browserHandler->chooser->action == MapBrowser::A_WAITING_FOR_MAP_DOWNLOAD)
	{
	SetRectColor(downloadingQuad, Color::Red);
	}*/
	//target->draw(bgQuad, 4, sf::Quads);
	if (action == A_CHECK_OVERWRITE)
	{
		browserHandler->chooser->panel->Draw(target);
		sf::RectangleShape rect;
		rect.setFillColor(Color(0, 0, 0, 100));
		rect.setSize(Vector2f(1920, 1080));
		rect.setPosition(0, 0);
		target->draw(rect);
		overwritePopup->panel->Draw(target);
	}
	else
	{
		browserHandler->chooser->panel->Draw(target);
	}
	
}