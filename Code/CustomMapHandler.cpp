#include "MainMenu.h"
#include "GameSession.h"
#include "LevelSelector.h"
#include "CustomMapClient.h"

using namespace sf;
using namespace std;

CustomMapsHandler::CustomMapsHandler(MainMenu *p_menu)
	:menu(p_menu), showNamePopup(false),
	showDownloadPopup(false), showLoginPopup(false), showRemovePopup(false)
{
}

//void CustomMapsHandler::LoadAndResaveMap( 

void CustomMapsHandler::ButtonCallback(Button *b, const std::string & e)
{
	LevelSelector &ls = *menu->levelSelector;
	if (ls.text[ls.selectedIndex].getFillColor() == Color::White)
	{
		if (b->name == "Play")
		{
			menu->gameRunType = MainMenu::GRT_FREEPLAY;
			GameSession *gs = new GameSession(NULL, ls.GetSelectedPath());
			GameSession::sLoad(gs);
			gs->Run();
			//gs->Reload(ls.fullPaths[ls.selectedIndex+1]);
			//gs->Run();
			menu->window->setView(menu->uiView);
			delete gs;
		}
		else if (b->name == "Edit")
		{
			menu->GameEditLoop(ls.GetSelectedPath());//ls.paths[ls.selectedIndex].().string() );//ls.text[ls.selectedIndex].getString() );
			ls.UpdateSelectedPreview();
			menu->window->setView(menu->uiView);
		}
		else if (b->name == "Delete")
		{
			//std::stringstream SSr;
			//SSr << ls.GetSelectedPath() << ls.GetSelectedName() << ".brknk";
			boost::filesystem::remove(ls.GetSelectedPath());
			ls.UpdateMapList();
		}
		else if (b->name == "Upload")
		{
			string path = "Resources/Maps/" + ls.localPaths[ls.selectedIndex];
			string name = ls.text[ls.selectedIndex].getString().toAnsiString();

			bool res = ls.customMapClient->AttemptUploadMapToServer(path, name, true);
			if (res)
			{
				cout << "successful upload" << endl;
			}
			else
			{
				cout << "upload failed" << endl;
			}
		}
	}
	else
	{

	}

	if (b->name == "Create New")
	{
		showNamePopup = true;
		//cout << "what" << endl;
		//cout << b->owner->name << ", " << b->owner->textBoxes.size() << ", " << b->owner->textBoxes.count( "name" ) << endl;

	}
	else if (b->name == "ok")
	{
		showNamePopup = false;
		ls.newLevelName = b->panel->textBoxes["name"]->text.getString().toAnsiString();
	}
	else if (b->name == "List")
	{
		cout << "..getting map list.." << endl;
		ls.customMapClient->AttempGetMapListFromServer();
		ls.customMapClient->PrintMapEntries();
	}
	else if (b->name == "Download")
	{
		ls.customMapClient->AttempGetMapListFromServer();
		showDownloadPopup = true;
	}
	else if (b->name == "downloadok")
	{
		showDownloadPopup = false;
		stringstream ss;
		ss << b->panel->textBoxes["index"]->text.getString().toAnsiString();
		int index;
		ss >> index;
		if (!ss.fail())
		{
			if (index >= 0 && index < ls.customMapClient->mapEntries.size())
			{
				cout << "attempting to download a map" << endl;
				CustomMapEntry &entry = ls.customMapClient->mapEntries[index];
				ls.customMapClient->AttemptDownloadMapFromServer(
					"Resources/Maps/DownloadedMaps/", entry);
			}
			else
			{
				cout << "invalid index: " << index << endl;
			}
		}
		else
		{
			cout << "index could not be read." << endl;
		}
	}
	else if (b->name == "Login")
	{
		if (!ls.customMapClient->IsLoggedIn())
		{
			showLoginPopup = true;
		}
		else
		{
			cout << "you are already logged in" << endl;
		}
	}
	else if (b->name == "loginok")
	{
		showLoginPopup = false;
		string user = b->panel->textBoxes["user"]->text.getString().toAnsiString();
		string pass = b->panel->textBoxes["pass"]->text.getString().toAnsiString();
		ls.customMapClient->AttemptUserLogin(user, pass);
	}
	else if (b->name == "Remove")
	{
		ls.customMapClient->AttempGetMapListFromServer();
		showRemovePopup = true;
	}
	else if (b->name == "removeok")
	{
		showRemovePopup = false;

		stringstream ss;
		ss << b->panel->textBoxes["index"]->text.getString().toAnsiString();
		int index;
		ss >> index;
		if (!ss.fail())
		{
			if (index >= 0 && index < ls.customMapClient->mapEntries.size())
			{
				cout << "attempting to remove a map" << endl;
				CustomMapEntry &entry = ls.customMapClient->mapEntries[index];
				bool res = ls.customMapClient->AttemptDeleteMapFromServer(entry);
				if (!res)
				{
					cout << "failed to remove map" << endl;
				}
			}
			else
			{
				cout << "invalid index: " << index << endl;
			}
		}
		else
		{
			cout << "index could not be read." << endl;
		}
	}
}

void CustomMapsHandler::TextBoxCallback(TextBox *tb, const std::string & e)
{
}

void CustomMapsHandler::GridSelectorCallback(GridSelector *gs, const std::string & e)
{
}

void CustomMapsHandler::CheckBoxCallback(CheckBox *cb, const std::string & e)
{
}

void CustomMapsHandler::SliderCallback(Slider *slider)
{

}

void CustomMapsHandler::DropdownCallback(Dropdown *dropdown, const std::string & e)
{

}