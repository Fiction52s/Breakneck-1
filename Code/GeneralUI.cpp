#include "GUI.h"
#include "VectorMath.h"
#include "EditSession.h"
#include "globals.h"
#include "AdventureCreator.h"
#include "MapBrowser.h"

using namespace std;
using namespace sf;

#define CANMAKEADVENTURES

GeneralUI::GeneralUI()
{
	edit = EditSession::GetSession();
	height = 28;

	int fileWidth = 70;
	int editWidth = 70;
	int modeWidth = 70;

	mainPanel = new Panel("general", 1920, height, this, false);
	mainPanel->SetColor(Color::Red);
	mainPanel->SetAutoSpacing(true);

	
	std::vector<string> fileOptions;
	if (edit->mainMenu->steamOn)
	{
		fileOptions = { "New Map", "Open Map",  "Save Map", "Save Map As", "Edit Adventure",
			"Create Adventure", "Publish", "EXIT" };
	}
	else
	{
		fileOptions = { "New Map", "Open Map",  "Save Map", "Save Map As", "Edit Adventure",
			"Create Adventure", "EXIT" };
	}
	
	fileDropdown = mainPanel->AddMenuDropdown("File", Vector2i(0, 0), 
		Vector2i(fileWidth, height), 200, fileOptions);
	std::vector<string> editOptions(EditOptions_Count);
	editOptions[UNDO] = "Undo (Ctrl + Z)";
	editOptions[REDO] = "Redo (Ctrl + Y)";
	editDropdown = mainPanel->AddMenuDropdown("Edit", Vector2i(0, 0), //Vector2i(fileDropdown->pos.x + fileWidth, 0),
		Vector2i(fileWidth, height), 200, editOptions);

	std::vector<string> modeOptions(ModeOptions_Count);
	modeOptions[CREATE_TERRAIN] = "Create Terrain";
	modeOptions[EDIT] = "Edit";
	modeOptions[CREATE_ENEMIES] = "Create Enemies";
	modeOptions[CREATE_GATES] = "Create Gates";
	modeOptions[CREATE_IMAGES] = "Create Images";
	modeOptions[MAP_OPTIONS] = "Map Options";

	modeDropdown = mainPanel->AddMenuDropdown("Mode", Vector2i(0, 0), //Vector2i(editDropdown->pos.x + editWidth, 0),
		Vector2i(fileWidth, height), 200, modeOptions);
}

GeneralUI::~GeneralUI()
{
	delete mainPanel;
}

void GeneralUI::MenuDropdownCallback(MenuDropdown *menuDrop, const std::string & e)
{
	if (menuDrop == fileDropdown)
	{

		int ind = menuDrop->selectedIndex;
		if (ind >= PUBLISH && !edit->mainMenu->steamOn )
		{
			++ind;
		}
		switch (ind)
		{
		case NEW_MAP:
		{
			edit->TryReloadNew();
			break;
		}
		case OPEN_MAP:
		{
			edit->OpenMapDialog();
			
			break;
		}
		case SAVE_MAP:
		{
			edit->TrySaveMap();
			break;
		}
		case SAVE_MAP_AS:
		{
			edit->SaveMapDialog(MapBrowser::EDITOR_SAVE);
			break;
		}
		case EDIT_ADVENTURE:
		{
#ifdef CANMAKEADVENTURES
			edit->mapBrowserHandler->chooser->StartRelative(ADVENTURE_EXT, MapBrowser::EDITOR_OPEN, "Resources/Adventure");
#endif
			//edit->fileChooser->chooser->StartRelative(ADVENTURE_EXT, FileChooser::OPEN, "");
			break;
		}
		case CREATE_ADVENTURE:
		{
#ifdef CANMAKEADVENTURES
			edit->adventureCreator->Open();
#endif
			break;
		}
		case PUBLISH:
		{
			edit->PublishMap();
			break;
		}
		case EXIT_EDITOR:
		{
			edit->TryExitEditor();
			//are you sure you want to exit without saving?
		}
		}
		
		
	}
	else if (menuDrop == editDropdown)
	{
		switch (menuDrop->selectedIndex)
		{
		case UNDO:
			edit->UndoMostRecentAction();
			break;
		case REDO:
			edit->RedoMostRecentUndoneAction();
			break;
		}
	}
	else if (menuDrop = modeDropdown)
	{
		switch (menuDrop->selectedIndex)
		{
		case CREATE_TERRAIN:
			edit->SetMode(EditSession::CREATE_TERRAIN);
			break;
		case EDIT:
			edit->SetMode(EditSession::EDIT);
			break;
		case CREATE_ENEMIES:
			edit->SetMode(EditSession::CREATE_ENEMY);
			break;
		case CREATE_GATES:
			edit->SetMode(EditSession::CREATE_GATES);
			break;
		case CREATE_IMAGES:
			edit->SetMode(EditSession::CREATE_IMAGES);
			break;
		case MAP_OPTIONS:
			edit->SetMode(EditSession::MAP_OPTIONS);
			break;
		}
	}
}

void GeneralUI::Draw(sf::RenderTarget *target)
{
	mainPanel->Draw(target);
}
