#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"
#include "EditorGraph.h"

using namespace std;
using namespace sf;

CreateTerrainModeUI::CreateTerrainModeUI()
{
	edit = EditSession::GetSession();
	mainPanel = new Panel("createterrain", 1920, 200, this, false);

	gridCheckbox = mainPanel->AddCheckBox("grid", Vector2i(10, 10), false);
	snapPointsCheckbox = mainPanel->AddCheckBox("lockpoints", Vector2i(50, 10), false);
	gridSizeTextbox = mainPanel->AddTextBox("gridisize", Vector2i(100, 10), 50, 5, "");
	completeButton = mainPanel->AddButton("complete", Vector2i(200, 10), Vector2f( 80, 80 ), "complete");
	removePointButton = mainPanel->AddButton("remove", Vector2i(300, 10), Vector2f(80, 80), "remove");
	removeAllPointsButton = mainPanel->AddButton("removeall", Vector2i(400, 10), Vector2f(80, 80), "remove all");

	std::vector<string> actionOptions = { "Add", "Subtract", "Set Inverse Poly" };
	terrainActionDropdown = mainPanel->AddDropdown("actiondrop", Vector2i(500, 10), Vector2i(200, 28), actionOptions, 0);

	std::vector<string> layerOptions = { "Terrain", "Water", "Pickup" };
	terrainLayerDropdown = mainPanel->AddDropdown("layerdrop", Vector2i(750, 10), Vector2i(200, 28), layerOptions, 0);

	
	int numTerrainLayers = EditSession::TERRAINLAYER_Count;
	currMatRects.resize(numTerrainLayers);
	mainPanel->ReserveImageRects(numTerrainLayers);
	for (int i = 0; i < numTerrainLayers; ++i)
	{
		currMatRects[i] = mainPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_TERRAINSEARCH,
			Vector2f(terrainLayerDropdown->pos)
			+ Vector2f(terrainLayerDropdown->size.x + 20, 0),NULL, 0 );
		currMatRects[i]->Init();
		currMatRects[i]->SetImage( edit->GetMatTileset( edit->currTerrainWorld[i],
			edit->currTerrainVar[i]), IntRect(0, 0, 64, 64));
		//int ind = edit->currTerrainWorld[EditSession::TERRAINLAYER_NORMAL]
		//	* MAX_TERRAINTEX_PER_WORLD + currTerrainVar[TERRAINLAYER_NORMAL];
		//currTerrainTypeSpr.setTexture(*ts_polyShaders[ind]->texture);
		//currTerrainTypeSpr.setTextureRect(IntRect(0, 0, 64, 64));
	}

	currMatRects[0]->SetShown(true);
}

bool CreateTerrainModeUI::IsGridOn()
{
	return gridCheckbox->checked;
}

void CreateTerrainModeUI::FlipGrid()
{
	gridCheckbox->checked = !gridCheckbox->checked;
}

void CreateTerrainModeUI::SetGridSize(int gs)
{
	gridSizeTextbox->SetString(to_string(gs));
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
	if (b == completeButton)
	{
		edit->ExecuteTerrainCompletion();
	}
	else if (b == removePointButton)
	{
		edit->removeProgressPointWaiter->Reset();
		edit->RemovePointFromPolygonInProgress();
	}
	else if (b == removeAllPointsButton)
	{
		edit->ClearPolygonInProgress();
	}
}

void CreateTerrainModeUI::TextBoxCallback(TextBox *tb, const std::string & e)
{
	if (tb == gridSizeTextbox)
	{
		string str = tb->GetString();
		stringstream ss;
		ss << str;
		int spacing;
		ss >> spacing;
		if (!ss.fail())
		{
			edit->graph->SetSpacing(spacing);
		}
	}
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
	if (dropdown == terrainLayerDropdown)
	{
		int selectedIndex = dropdown->selectedIndex;
		for (int i = 0; i < EditSession::TERRAINLAYER_Count; ++i)
		{
			currMatRects[i]->SetShown(false);
		}

		currMatRects[selectedIndex]->SetShown(true);
	}
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