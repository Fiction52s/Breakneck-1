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
	matTypePanel = new Panel("mattype", 600, 600, this, true);
	matTypePanel->SetPosition(Vector2i(0, 250));

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

	int terrainBoxSize = 64;

	int numTerrainLayers = EditSession::TERRAINLAYER_Count;
	currMatRects.resize(numTerrainLayers);
	mainPanel->ReserveImageRects(numTerrainLayers);
	for (int i = 0; i < numTerrainLayers; ++i)
	{
		currMatRects[i] = mainPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_TERRAINSEARCH,
			Vector2f(terrainLayerDropdown->pos)
			+ Vector2f(terrainLayerDropdown->size.x + 20, 0),NULL, 0, 100 );
		currMatRects[i]->Init();
		currMatRects[i]->SetImage( edit->GetMatTileset( edit->currTerrainWorld[i],
			edit->currTerrainVar[i]), IntRect(0, 0, 100, 100));
	}

	currMatRects[0]->SetShown(true);

	int maxTexPerWorld = EditSession::MAX_TERRAINTEX_PER_WORLD;
	int numTypeRects = 8 * maxTexPerWorld;
	matTypeRects.resize(numTypeRects);
	
	matTypePanel->ReserveImageRects(numTypeRects);

	for (int worldI = 0; worldI < 8; ++worldI)
	{
		int ind;
		for (int i = 0; i < maxTexPerWorld; ++i)
		{
			ind = worldI * maxTexPerWorld + i;

			matTypeRects[ind] = matTypePanel->AddImageRect(
				ChooseRect::ChooseRectIdentity::I_TERRAINSEARCH,
				Vector2f(worldI * terrainBoxSize, i * terrainBoxSize),
				edit->GetMatTileset(worldI, i), 
				IntRect(0, 0, terrainBoxSize, terrainBoxSize),
				terrainBoxSize);
			matTypeRects[ind]->Init();
			if (matTypeRects[ind]->ts != NULL)
			{
				matTypeRects[ind]->SetShown(true);
			}
		}
	}
}

CreateTerrainModeUI::~CreateTerrainModeUI()
{
	delete mainPanel;
	delete matTypePanel;
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

void CreateTerrainModeUI::SetShown(bool s)
{
	show = s;
	if (show)
	{
		edit->AddActivePanel(mainPanel);
		edit->AddActivePanel(matTypePanel);
		//if (showLibrary)
		//{
		//edit->AddActivePanel(libPanel);
		//}
	}
	else
	{
		//edit->RemoveActivePanel(varSelector->panel);
		edit->RemoveActivePanel(mainPanel);
		edit->RemoveActivePanel(matTypePanel);
		//if (showLibrary)
		//{
		//	edit->RemoveActivePanel(libPanel);
		//}
	}
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

