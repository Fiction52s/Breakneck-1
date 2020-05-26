#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"

using namespace std;
using namespace sf;

CreateTerrainModeUI::CreateTerrainModeUI()
{
	edit = EditSession::GetSession();
	mainPanel = new Panel("createterrain", 1920, 200, this, false);

	gridCheckbox = mainPanel->AddCheckBox("grid", Vector2i(10, 10), false);
	snapPointsCheckbox = mainPanel->AddCheckBox("lockpoints", Vector2i(50, 10), false);
}

void CreateTerrainModeUI::Update()
{

}

bool CreateTerrainModeUI::IsGridOn()
{
	return gridCheckbox->checked;
}

void CreateTerrainModeUI::FlipGrid()
{
	gridCheckbox->checked = !gridCheckbox->checked;
}


bool CreateTerrainModeUI::IsSnapPointsOn()
{
	return snapPointsCheckbox->checked;
}

void CreateTerrainModeUI::FlipSnapPoints()
{
	snapPointsCheckbox->checked = !snapPointsCheckbox->checked;
}

CreateTerrainModeUI::~CreateTerrainModeUI()
{
	delete mainPanel;
}

void CreateTerrainModeUI::ButtonCallback(Button *b, const std::string & e)
{

}

void CreateTerrainModeUI::TextBoxCallback(TextBox *tb, const std::string & e)
{

}

void CreateTerrainModeUI::GridSelectorCallback(GridSelector *gs, const std::string & e)
{

}

void CreateTerrainModeUI::CheckBoxCallback(CheckBox *cb, const std::string & e)
{

}

void CreateTerrainModeUI::SliderCallback(Slider *slider, const std::string & e)
{

}

void CreateTerrainModeUI::DropdownCallback(Dropdown *dropdown, const std::string & e)
{

}

void CreateTerrainModeUI::SetShown(bool s)
{
	show = s;
	if (show)
	{
		edit->AddActivePanel(mainPanel);
		//if (showLibrary)
		//{
			//edit->AddActivePanel(libPanel);
		//}
	}
	else
	{
		//edit->RemoveActivePanel(varSelector->panel);
		edit->RemoveActivePanel(mainPanel);
		//if (showLibrary)
		//{
		//	edit->RemoveActivePanel(libPanel);
		//}
	}
}