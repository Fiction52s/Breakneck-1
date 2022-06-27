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
			MatchParams mp;
			mp.mapPath = ls.GetSelectedPath();

			GameSession *gs = new GameSession(&mp);
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
		}
		else if (b->name == "Quickplay")
		{

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