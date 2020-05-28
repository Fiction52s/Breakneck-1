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

	mainPanel = new Panel("createterrain", 1280, 120, this, false);
	mainPanel->SetPosition(Vector2i(0, edit->generalUI->height));


	mainPanel->SetAutoSpacing(true, false, Vector2i(10, 10), Vector2i(20, 0));

	int labelCharHeight = 24;
	int labelExtraSpacing = 30;
	int labelExtraY = 10;
	Vector2i labelExtra(30, 10);

	realTerrainTool = 0;
	terrainGridSize = 64;
	
	
	mainPanel->AddLabel("snaplabel", Vector2i(0, labelExtra.y), labelCharHeight, "Snap to Points:");
	snapPointsCheckbox = mainPanel->AddCheckBox("lockpoints", Vector2i(0, 0), false);
	snapPointsCheckbox->SetToolTip("Toggle Snap to points (F)");

	mainPanel->AddLabel("movelabel", labelExtra, labelCharHeight, "Grid:");
	gridCheckbox = mainPanel->AddCheckBox("grid", Vector2i(0, 0), false);
	gridCheckbox->SetToolTip("Toggle Grid (G)");

	gridSizeTextbox = mainPanel->AddTextBox("gridisize", Vector2i(0, 0), 50, 5, "");
	gridSizeTextbox->SetToolTip("Set the grid spacing");
	SetGridSize(edit->graph->GetSpacing());
	
	std::vector<string> drawOptions = { "Draw", "Box" };// , "Brush"
	drawModeDropdown = mainPanel->AddDropdown("drawmodedrop", Vector2i(0, 0), Vector2i(200, 28), drawOptions, 0);
	drawModeDropdown->SetToolTip("Choose creation tool\nDraw (D)\nBox (B)");

	std::vector<string> actionOptions = { "Add", "Subtract", "Set Inverse Poly" };
	terrainActionDropdown = mainPanel->AddDropdown("actiondrop", Vector2i(0, 0), Vector2i(200, 28), actionOptions, 0);
	terrainActionDropdown->SetToolTip("Choose polygon action\nAdd (A)\nSubtract (S)\nSet Inverse Poly (I)");

	std::vector<string> layerOptions = { "Terrain", "Water", "Pickup" };
	terrainLayerDropdown = mainPanel->AddDropdown("layerdrop", Vector2i(0, 0), Vector2i(200, 28), layerOptions, 0);
	terrainLayerDropdown->SetToolTip("Choose polygon layer\n(E to choose material)");

	int numTerrainLayers = TERRAINLAYER_Count;
	currMatRects.resize(numTerrainLayers);
	mainPanel->ReserveImageRects(numTerrainLayers);

	//SetAutoSpacing( false)
	Vector2i storedAutoStart = mainPanel->autoStart;
	

	Vector2f currMatRectPos = Vector2f(20, 0);//Vector2f(terrainLayerDropdown->pos)
		//+ Vector2f(terrainLayerDropdown->size.x + 20, 0);

	mainPanel->StopAutoSpacing();

	for (int i = 0; i < numTerrainLayers; ++i)
	{
		if (i == numTerrainLayers - 1)
			mainPanel->RestartAutoSpacing();

		currMatRects[i] = mainPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_TERRAINSEARCH,
			currMatRectPos, NULL, 0, 100);
		currMatRects[i]->Init();
		currMatRects[i]->SetImage(edit->GetMatTileset(edit->currTerrainWorld[i],
			edit->currTerrainVar[i]), IntRect(0, 0, 128, 128));
	}
	currMatRects[0]->SetShown(true);

	
	mainPanel->SetAutoSpacing(true, false, Vector2i(10, 70), Vector2i(20, 0));
	

	completeButton = mainPanel->AddButton("complete", Vector2i(0, 0), Vector2f(200, 28 + 4), "complete");
	completeButton->SetToolTip("Complete current polygon (Space)");

	removePointButton = mainPanel->AddButton("remove", Vector2i(0, 0), Vector2f(200, 28 + 4), "remove");
	removePointButton->SetToolTip("Remove progress point (X / Delete)");

	removeAllPointsButton = mainPanel->AddButton("removeall", Vector2i(0, 0), Vector2f(200, 28 + 4), "remove all");
	removeAllPointsButton->SetToolTip("Remove all progress points (no hotkey yet)");


	matPanelPos = Vector2i(currMatRects[0]->pos.x, currMatRects[0]->pos.y + 100 + 10) + mainPanel->pos;
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

int CreateTerrainModeUI::GetCurrDrawTool()
{
	return drawModeDropdown->selectedIndex;
}

void CreateTerrainModeUI::SetDrawTool(int t)
{
	if (t != drawModeDropdown->selectedIndex)
	{
		drawModeDropdown->SetSelectedIndex(t);
		switch (t)
		{
		case EditSession::TOOL_BOX:
			edit->SetBoxTool();
			break;
		}
	}
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
	else if (dropdown == drawModeDropdown)
	{
		int selectedIndex = dropdown->selectedIndex;
		SetDrawTool(selectedIndex);
	}
}

void CreateTerrainModeUI::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup" && p == matTypePanel)
	{
		edit->RemoveActivePanel(matTypePanel);
	}
}