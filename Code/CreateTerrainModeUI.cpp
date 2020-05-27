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

	//matTypeRects = &edit->matTypeRects;
	terrainGridSize = edit->terrainGridSize;
	matTypePanel = edit->matTypePanel;

	mainPanel = new Panel("createterrain", 1920, 200, this, false);

	gridCheckbox = mainPanel->AddCheckBox("grid", Vector2i(10, 10), false);
	gridCheckbox->SetToolTip("Toggle Grid (G)");

	snapPointsCheckbox = mainPanel->AddCheckBox("lockpoints", Vector2i(50, 10), false);
	snapPointsCheckbox->SetToolTip("Toggle Snap to points (F)");
	
	gridSizeTextbox = mainPanel->AddTextBox("gridisize", Vector2i(100, 10), 50, 5, "");
	gridSizeTextbox->SetToolTip("Set the grid spacing");

	completeButton = mainPanel->AddButton("complete", Vector2i(200, 10), Vector2f( 80, 80 ), "complete");
	completeButton->SetToolTip("Complete current polygon (Space)");

	removePointButton = mainPanel->AddButton("remove", Vector2i(300, 10), Vector2f(80, 80), "remove");
	removePointButton->SetToolTip("Remove progress point (X / Delete)");

	removeAllPointsButton = mainPanel->AddButton("removeall", Vector2i(400, 10), Vector2f(80, 80), "remove all");
	removeAllPointsButton->SetToolTip("Remove all progress points (no hotkey yet)");

	std::vector<string> actionOptions = { "Add", "Subtract", "Set Inverse Poly" };
	terrainActionDropdown = mainPanel->AddDropdown("actiondrop", Vector2i(500, 10), Vector2i(200, 28), actionOptions, 0);
	terrainActionDropdown->SetToolTip("Choose polygon action\nAdd (A)\nSubtract (S)\nSet Inverse Poly (I)");

	std::vector<string> layerOptions = { "Terrain", "Water", "Pickup" };
	terrainLayerDropdown = mainPanel->AddDropdown("layerdrop", Vector2i(750, 10), Vector2i(200, 28), layerOptions, 0);
	terrainLayerDropdown->SetToolTip("Choose polygon layer\n(E to choose material)");

	realTerrainTool = 0;

	terrainGridSize = 64;

	int numTerrainLayers = TERRAINLAYER_Count;
	currMatRects.resize(numTerrainLayers);
	mainPanel->ReserveImageRects(numTerrainLayers);
	Vector2f currMatRectPos = Vector2f(terrainLayerDropdown->pos)
		+ Vector2f(terrainLayerDropdown->size.x + 20, 0);
	for (int i = 0; i < numTerrainLayers; ++i)
	{
		currMatRects[i] = mainPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_TERRAINSEARCH,
			currMatRectPos,NULL, 0, 100 );
		currMatRects[i]->Init();
		currMatRects[i]->SetImage( edit->GetMatTileset( edit->currTerrainWorld[i],
			edit->currTerrainVar[i]), IntRect(0, 0, 128, 128));
	}

	currMatRects[0]->SetShown(true);

	matPanelPos = Vector2i(currMatRectPos.x, currMatRectPos.y + 100 + 10);
}

CreateTerrainModeUI::~CreateTerrainModeUI()
{
	delete mainPanel;
	
}

void CreateTerrainModeUI::ExpandTerrainLibrary()
{
	if (matTypePanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(matTypePanel);
	}
	else
	{
		matTypePanel->SetPosition(matPanelPos);
		matTypePanel->handler = this;
		edit->SetMatTypePanelLayer(GetTerrainLayer());
		edit->AddActivePanel(matTypePanel);
	}
}

void CreateTerrainModeUI::ChooseMatType(ImageChooseRect *icRect)
{
	int layerIndex = GetTerrainLayer();
	currMatRects[layerIndex]->SetImage(icRect->ts, icRect->spr.getTextureRect());

	int world = icRect->pos.x / terrainGridSize;
	int variation = icRect->pos.y / terrainGridSize;

	edit->currTerrainWorld[layerIndex] = world;
	edit->currTerrainVar[layerIndex] = variation;

	edit->RemoveActivePanel(matTypePanel);
	edit->justCompletedPolyWithClick = true;
}


int CreateTerrainModeUI::GetCurrTerrainTool()
{
	return terrainActionDropdown->selectedIndex;
}

void CreateTerrainModeUI::SetTerrainTool(int t)
{
	terrainActionDropdown->SetSelectedIndex(t);
	realTerrainTool = t;
}

void CreateTerrainModeUI::SetTempTerrainTool(int t)
{
	terrainActionDropdown->SetSelectedIndex(t);
}

void CreateTerrainModeUI::RevertTerrainTool()
{
	terrainActionDropdown->SetSelectedIndex(realTerrainTool);
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

int CreateTerrainModeUI::GetTerrainLayer()
{
	return terrainLayerDropdown->selectedIndex;
}

void CreateTerrainModeUI::SetShown(bool s)
{
	show = s;
	if (show)
	{
		edit->AddActivePanel(mainPanel);
	}
	else
	{
		edit->RemoveActivePanel(mainPanel);
	}
}

void CreateTerrainModeUI::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::E_LEFTCLICKED ||
		eventType == ChooseRect::E_LEFTRELEASED )
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_TERRAINSEARCH && eventType == ChooseRect::E_LEFTCLICKED )
			{
				ExpandTerrainLibrary();
			}
			else if (icRect->rectIdentity == ChooseRect::I_TERRAINLIBRARY)
			{
				ChooseMatType(icRect);
			}
		}
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
	if (cb == gridCheckbox)
	{
		edit->showGraph = cb->checked;
	}
}

void CreateTerrainModeUI::SliderCallback(Slider *slider, const std::string & e)
{

}

void CreateTerrainModeUI::DropdownCallback(Dropdown *dropdown, const std::string & e)
{
	if (dropdown == terrainLayerDropdown)
	{
		int selectedIndex = dropdown->selectedIndex;
		for (int i = 0; i < TERRAINLAYER_Count; ++i)
		{
			currMatRects[i]->SetShown(false);
		}

		currMatRects[selectedIndex]->SetShown(true);
	}
	else if (dropdown == terrainActionDropdown)
	{
		int selectedIndex = dropdown->selectedIndex;
		realTerrainTool = selectedIndex;
	}
}

void CreateTerrainModeUI::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup" && p == matTypePanel)
	{
		edit->RemoveActivePanel(matTypePanel);
	}
}