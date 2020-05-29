#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"
#include "EditorGraph.h"
#include "EditorRail.h"

using namespace std;
using namespace sf;

CreateRailModeUI::CreateRailModeUI()
{
	edit = EditSession::GetSession();

	//matTypeRects = &edit->matTypeRects;
	//terrainGridSize = edit->terrainGridSize;
	//matTypePanel = edit->matTypePanel;

	mainPanel = new Panel("createrail", 820, 120, this, false);
	mainPanel->SetPosition(Vector2i(0, edit->generalUI->height));

	mainPanel->SetAutoSpacing(true, false, Vector2i(10, 10), Vector2i(20, 0));

	int labelCharHeight = 24;
	int labelExtraSpacing = 30;
	int labelExtraY = 10;
	Vector2i labelExtra(30, 10);

	enemyPanel = new Panel("enemy", 600, 600, this, true);
	physicalPanel = new Panel("type", 600, 600, this, true);

	railTypeGridSize = 100;

	
	mainPanel->AddLabel("snaplabel", Vector2i(0, labelExtra.y), labelCharHeight, "Snap to Points:");
	snapPointsCheckbox = mainPanel->AddCheckBox("lockpoints", Vector2i(0, 0), false);
	snapPointsCheckbox->SetToolTip("Toggle Snap to points (F)");

	mainPanel->AddLabel("movelabel", labelExtra, labelCharHeight, "Grid:");
	gridCheckbox = mainPanel->AddCheckBox("grid", Vector2i(0, 0), false);
	gridCheckbox->SetToolTip("Toggle Grid (G)");

	gridSizeTextbox = mainPanel->AddTextBox("gridsize", Vector2i(0, 0), 50, 5, "");
	gridSizeTextbox->SetToolTip("Set the grid spacing");
	SetGridSize(edit->graph->GetSpacing());

	std::vector<string> catOptions = { "Physical", "Enemy" };
	railCategoryDropdown = mainPanel->AddDropdown("actiondrop", Vector2i(0, 0), Vector2i(200, 28), catOptions, 0);
	railCategoryDropdown->SetToolTip("Choose rail category\nAdd (A)\nSubtract (S)\nSet Inverse Poly (I)");

	mainPanel->ReserveImageRects(1);
	mainPanel->ReserveEnemyRects(1);

	mainPanel->StopAutoSpacing();

	Vector2f currTypeRectPos = Vector2f(0, 0);/*Vector2f(railCategoryDropdown->pos)
		+ Vector2f(railCategoryDropdown->size.x + 20, 0);*/

	currPhysicalTypeRect = mainPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_RAILSEARCH,
		currTypeRectPos, NULL, 0, 100);
	currPhysicalTypeRect->Init();
	
	currPhysicalTypeRect->SetShown(true);

	currEnemyTypeRect = mainPanel->AddEnemyRect(ChooseRect::ChooseRectIdentity::I_RAILSEARCH,
		currTypeRectPos, NULL, 0);
	currEnemyTypeRect->Init();

	Tileset *ts_physicalTypes = edit->GetSizedTileset("Editor/gatecategories_128x128.png");
	
	int numPhysTypes = PhysTypes_Count;
	physicalPanel->ReserveImageRects(numPhysTypes);
	physRects.resize(numPhysTypes);
	for (int i = 0; i < numPhysTypes; ++i)
	{
		physRects[i] = physicalPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_RAILLIBRARY,
			Vector2f( 0, i * railTypeGridSize), ts_physicalTypes, i, railTypeGridSize);
		//physRects[i]->Init();
		physRects[i]->SetShown(true);
	}

	physicalPanel->SetPosition(Vector2i(currPhysicalTypeRect->pos) + Vector2i(0, 120));
	enemyPanel->SetPosition(Vector2i(currEnemyTypeRect->pos) + Vector2i(0, 120));

	ChoosePhysicalType(physRects[0]);

	int numEnemyTypes = 2;
	enemyPanel->ReserveEnemyRects(numEnemyTypes);
	enemyRects.resize(numEnemyTypes);
	std::vector<ActorType*> types;
	types.reserve(numEnemyTypes);
	types.push_back(edit->types["blocker"]);
	types.push_back(edit->types["healthfly"]);

	for (int i = 0; i < numEnemyTypes; ++i)
	{
		enemyRects[i] = enemyPanel->AddEnemyRect(ChooseRect::ChooseRectIdentity::I_RAILLIBRARY,
			Vector2f(0, i * railTypeGridSize), types[i], 1 );
		enemyRects[i]->SetShown(true);
	}

	ChooseEnemyType(enemyRects[0]);

	mainPanel->SetAutoSpacing(true, false, Vector2i(10, 70), Vector2i(20, 0));

	completeButton = mainPanel->AddButton("complete", Vector2i(0, 0), Vector2f(200, 28 + 4), "complete");
	completeButton->SetToolTip("Complete current rail (Space)");

	removePointButton = mainPanel->AddButton("remove", Vector2i(0, 0), Vector2f(200, 28 + 4), "remove");
	removePointButton->SetToolTip("Remove progress point (X / Delete)");

	removeAllPointsButton = mainPanel->AddButton("removeall", Vector2i(0, 0), Vector2f(200, 28 + 4), "remove all");
	removeAllPointsButton->SetToolTip("Remove all progress points (no hotkey yet)");
	/*Vector2f currMatRectPos = Vector2f(terrainLayerDropdown->pos)
		+ Vector2f(terrainLayerDropdown->size.x + 20, 0);
	for (int i = 0; i < numTerrainLayers; ++i)
	{
		currMatRects[i] = mainPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_TERRAINSEARCH,
			currMatRectPos, NULL, 0, 100);
		currMatRects[i]->Init();
		currMatRects[i]->SetImage(edit->GetMatTileset(edit->currTerrainWorld[i],
			edit->currTerrainVar[i]), IntRect(0, 0, 128, 128));
	}

	currMatRects[0]->SetShown(true);*/



	//matPanelPos = Vector2i(currMatRectPos.x, currMatRectPos.y + 100 + 10) + mainPanel->pos;
}

CreateRailModeUI::~CreateRailModeUI()
{
	delete mainPanel;
	delete physicalPanel;
	delete enemyPanel;
}

int CreateRailModeUI::GetRailCategory()
{
	return railCategoryDropdown->selectedIndex;
}

void CreateRailModeUI::ExpandPhysicalLibrary()
{
	if (physicalPanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(physicalPanel);
	}
	else
	{
		edit->AddActivePanel(physicalPanel);
	}
}

void CreateRailModeUI::ExpandEnemyLibrary()
{
	if (enemyPanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(enemyPanel);
	}
	else
	{
		edit->AddActivePanel(enemyPanel);
	}
}

void CreateRailModeUI::ExpandLibrary()
{
	int cat = GetRailCategory();
	switch (cat)
	{
	case PHYSICAL:
		ExpandPhysicalLibrary();
		break;
	case ENEMY:
		ExpandEnemyLibrary();
		break;
	}
}

void CreateRailModeUI::ChoosePhysicalType(ImageChooseRect *icRect)
{
	currPhysicalTypeRect->SetImage(icRect->ts, icRect->spr.getTextureRect());

	int x = icRect->pos.x / railTypeGridSize;
	int y = icRect->pos.y / railTypeGridSize;

	physTypeIndex = y;
	//edit->currTerrainWorld[layerIndex] = world;
	//edit->currTerrainVar[layerIndex] = variation;

	edit->RemoveActivePanel(physicalPanel);
	edit->justCompletedRailWithClick = true;
}

void CreateRailModeUI::ChooseEnemyType(EnemyChooseRect *ecRect)
{
	currEnemyTypeRect->SetType(ecRect->enemyParams->type, ecRect->enemyParams->GetLevel());
	//int x = ecRect->pos.x / terrainGridSize;
	int y = ecRect->pos.y / railTypeGridSize;

	enemyTypeIndex = y;

	edit->RemoveActivePanel(enemyPanel);
	edit->justCompletedRailWithClick = true;
}

int CreateRailModeUI::GetRailType()
{
	int cat = GetRailCategory();
	if (cat == PHYSICAL)
	{
		return TerrainRail::RailType::NORMAL;
	}
	else if (cat == ENEMY)
	{
		return enemyTypeIndex + 1;
	}
}

bool CreateRailModeUI::IsGridOn()
{
	return gridCheckbox->checked;
}

void CreateRailModeUI::FlipGrid()
{
	gridCheckbox->checked = !gridCheckbox->checked;
}

void CreateRailModeUI::SetGridSize(int gs)
{
	gridSizeTextbox->SetString(to_string(gs));
}

bool CreateRailModeUI::IsSnapPointsOn()
{
	return snapPointsCheckbox->checked;
}

void CreateRailModeUI::FlipSnapPoints()
{
	snapPointsCheckbox->checked = !snapPointsCheckbox->checked;
}

void CreateRailModeUI::SetShown(bool s)
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

void CreateRailModeUI::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::E_LEFTCLICKED ||
		eventType == ChooseRect::E_LEFTRELEASED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		EnemyChooseRect *ecRect = cr->GetAsEnemyChooseRect();

		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_RAILSEARCH
				&& eventType == ChooseRect::E_LEFTCLICKED)
			{
				ExpandLibrary();
			}
			else if (icRect->rectIdentity == ChooseRect::I_RAILLIBRARY)
			{
				ChoosePhysicalType(icRect);
				//ChooseMatType(icRect);
			}
		}
		else if (ecRect != NULL)
		{
			if (ecRect->rectIdentity == ChooseRect::I_RAILSEARCH
				&& eventType == ChooseRect::E_LEFTCLICKED)
			{
				ExpandLibrary();
				//ExpandTerrainLibrary();
			}
			else if (ecRect->rectIdentity == ChooseRect::I_RAILLIBRARY)
			{
				ChooseEnemyType(ecRect);
			}
		}
	}
}

void CreateRailModeUI::ButtonCallback(Button *b, const std::string & e)
{
	if (b == completeButton)
	{
		edit->ExecuteRailCompletion();
	}
	else if (b == removePointButton)
	{
		edit->removeProgressPointWaiter->Reset();
		edit->RemovePointFromRailInProgress();
	}
	else if (b == removeAllPointsButton)
	{
		edit->ClearRailInProgress();
	}
}

void CreateRailModeUI::TextBoxCallback(TextBox *tb, const std::string & e)
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

void CreateRailModeUI::GridSelectorCallback(GridSelector *gs, const std::string & e)
{

}

void CreateRailModeUI::CheckBoxCallback(CheckBox *cb, const std::string & e)
{
	if (cb == gridCheckbox)
	{
		edit->showGraph = cb->checked;
	}
}

void CreateRailModeUI::SliderCallback(Slider *slider)
{

}

void CreateRailModeUI::DropdownCallback(Dropdown *dropdown, const std::string & e)
{
	if (dropdown == railCategoryDropdown)
	{
		int selInd = dropdown->selectedIndex;
		if (selInd == PHYSICAL)
		{
			currPhysicalTypeRect->SetShown(true);
			currEnemyTypeRect->SetShown(false);
		}
		else if( selInd == ENEMY )
		{
			currEnemyTypeRect->SetShown(true);
			currPhysicalTypeRect->SetShown(false);
		}
	}
	/*if (dropdown == terrainLayerDropdown)
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
	}*/
}

void CreateRailModeUI::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup" )
	{
		edit->RemoveActivePanel(p);
	}
}