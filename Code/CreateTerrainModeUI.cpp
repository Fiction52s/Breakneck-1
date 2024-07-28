#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"
#include "EditorGraph.h"
#include "BrushManager.h"
#include "Action.h"

using namespace std;
using namespace sf;

CreateTerrainModeUI::CreateTerrainModeUI()
{
	edit = EditSession::GetSession();

	//matTypeRects = &edit->matTypeRects;
	terrainGridSize = edit->terrainGridSize;
	matTypePanel = edit->matTypePanel;

	mainPanel = new Panel("createterrain", 1920, 120, this, false);
	mainPanel->SetPosition(Vector2i(0, edit->generalUI->height));


	mainPanel->SetAutoSpacing(true, false, Vector2i(10, 10), Vector2i(20, 0));

	int labelCharHeight = 24;
	int labelExtraSpacing = 30;
	int labelExtraY = 10;
	Vector2i labelExtra(30, 10);

	realTerrainTool = 0;
	terrainGridSize = 100;
	
	
	mainPanel->AddLabel("snaplabel", Vector2i(0, labelExtra.y), labelCharHeight, "Snap to Points:");
	snapPointsCheckbox = mainPanel->AddCheckBox("lockpoints", Vector2i(0, 0), false);
	snapPointsCheckbox->SetToolTip("Toggle Snap to points (F)");

	mainPanel->AddLabel("movelabel", labelExtra, labelCharHeight, "Grid:");
	gridCheckbox = mainPanel->AddCheckBox("grid", Vector2i(0, 0), false);
	gridCheckbox->SetToolTip("Toggle Grid (G)");

	gridSizeTextbox = mainPanel->AddBasicTextBox("gridsize", Vector2i(0, 0), 50, 5, "");
	gridSizeTextbox->SetToolTip("Set the grid spacing");
	SetGridSize(edit->graph->GetSpacing());

	minEdgeLenTextbox = mainPanel->AddLabeledTextBox("minedge", Vector2i(0, 0), true, 50, 1, 20, 4, "", "Min Draw Edge Length:");
	minEdgeLenTextbox->SetNumbersOnly(true);
	minEdgeLenTextbox->SetToolTip("Minimum edge length while drawing polygons.\nIf zoomed out, it uses screen pixels,"
		"\nand if zoomed in, it uses world pixels");
	
	std::vector<string> categoryOptions = { "Terrain", "Water", "Pickup", "Visual", "Visual Water" };
	terrainCategoryDropdown = mainPanel->AddDropdown("categorydrop", Vector2i(0, 0), Vector2i(200, 28), categoryOptions, 0);
	terrainCategoryDropdown->SetToolTip("Choose polygon category\n(E to choose material)");

	Vector2f currMatRectPos = Vector2f(0, 0);

	mainPanel->PauseAutoSpacing();

	int numTerrainCategories = TerrainPolygon::CATEGORY_Count;
	currMatRects.resize(numTerrainCategories);
	mainPanel->ReserveImageRects(numTerrainCategories);

	for (int i = 0; i < numTerrainCategories; ++i)
	{
		if (i == numTerrainCategories - 1)
			mainPanel->UnpauseAutoSpacing();

		currMatRects[i] = mainPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_TERRAINSEARCH,
			currMatRectPos, NULL, 0, terrainGridSize);
		currMatRects[i]->Init();
		//currMatRects[i]->SetImage(edit->GetMatTileset(edit->currTerrainWorld[i],
		//	edit->currTerrainVar[i]), IntRect(0, 0, 128, 128));
	}
	currMatRects[0]->SetShown(true);

	std::vector<string> drawOptions = { "Draw", "Box", "Brush" };
	drawModeDropdown = mainPanel->AddDropdown("drawmodedrop", Vector2i(0, 0), Vector2i(200, 28), drawOptions, 0);
	drawModeDropdown->SetToolTip("Choose creation tool\nDraw (D)\nBox (B)\nBrush (R)");

	std::vector<string> terrainOptions = { "Add", "Subtract", "Set Inverse Poly" };
	terrainActionDropdown = mainPanel->AddDropdown("terraindrop", Vector2i(0, 0), Vector2i(200, 28), terrainOptions, 0);
	terrainActionDropdown->SetToolTip("Choose polygon action\nAdd (A)\nSubtract (S)\nSet Inverse Poly (I)");


	mainPanel->StopAutoSpacing();

	std::vector<string> waterOptions = { "Add", "Subtract"};
	limitedTerrainActionDropdown = mainPanel->AddDropdown("waterdrop", terrainActionDropdown->pos, Vector2i(200, 28), waterOptions, 0);
	limitedTerrainActionDropdown->SetToolTip("Choose polygon action\nAdd (A)\nSubtract");
	limitedTerrainActionDropdown->Deactivate();
	limitedTerrainActionDropdown->HideMember();

	mainPanel->SetAutoSpacing(true, false, Vector2i(10, 70), Vector2i(20, 0));



	//SetAutoSpacing( false)
	//Vector2i storedAutoStart = mainPanel->autoStart;
	

	
	

	completeButton = mainPanel->AddButton("complete", Vector2i(0, 0), Vector2f(200, 28 + 4), "complete");
	completeButton->SetToolTip("Complete current polygon (Space)");

	removePointButton = mainPanel->AddButton("remove", Vector2i(0, 0), Vector2f(200, 28 + 4), "remove");
	removePointButton->SetToolTip("Remove progress point (X / Delete)");

	removeAllPointsButton = mainPanel->AddButton("removeall", Vector2i(0, 0), Vector2f(200, 28 + 4), "remove all");
	removeAllPointsButton->SetToolTip("Remove all progress points (no hotkey yet)");


	Color panelColor;
	panelColor = Color::Cyan;
	panelColor.a = 80;
	int totalHotbarCount = BrushManager::MAX_RECENT_BRUSHES;
	int hotbarRectSize = 200;
	int hotbarSpacing = 20;
	int totalHotbarSize = hotbarRectSize * totalHotbarCount + hotbarSpacing * (totalHotbarCount - 1);
	int extraHotbarSpacing = (1920 - totalHotbarSize) / 2;

	//brushHotbar = new 
	brushHotbar = new Panel("topbarpanel", 1920, 220, this, false);
	brushHotbar->SetPosition(Vector2i(0, 20));
	brushHotbar->SetColor(panelColor);
	brushHotbar->ReserveImageRects(totalHotbarCount);
	brushHotbarImages.resize(totalHotbarCount);

	for (int i = 0; i < totalHotbarCount; ++i)
	{
		brushHotbarImages[i] = brushHotbar->AddImageRect(ChooseRect::I_BRUSHHOTBAR,
			Vector2f(extraHotbarSpacing + 10 + i * (hotbarRectSize + hotbarSpacing), 10),
			NULL, 0, hotbarRectSize);
		brushHotbarImages[i]->SetShown(false);
		brushHotbarImages[i]->Init();
	}

	matPanelPos = Vector2i(currMatRects[0]->pos.x, currMatRects[0]->pos.y + 100 + 10) + mainPanel->pos;

	//set up rectangles for starting material
	auto &mtr0 = edit->matTypeRects[0];
	auto &mtr1 = edit->matTypeRects[1];
	auto &mtr2 = edit->matTypeRects[2];
	auto &mtr3 = edit->matTypeRects[3];
	auto &mtr4 = edit->matTypeRects[4];

	terrainCategoryDropdown->SetSelectedIndex(0);
	ChooseMatType(mtr0.at(0));
	terrainCategoryDropdown->SetSelectedIndex(1);
	ChooseMatType(mtr1.at(0));
	terrainCategoryDropdown->SetSelectedIndex(2);
	ChooseMatType(mtr2.at(0));
	terrainCategoryDropdown->SetSelectedIndex(3);
	ChooseMatType(mtr3.at(0));
	terrainCategoryDropdown->SetSelectedIndex(4);
	ChooseMatType(mtr4.at(0));

	terrainCategoryDropdown->SetSelectedIndex(0);

	currVisualDrawLayer = DrawLayer::BG_5; //TERRAIN_BACK;
}

CreateTerrainModeUI::~CreateTerrainModeUI()
{
	delete mainPanel;
	delete brushHotbar;
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
		edit->SetMatTypePanelCategory(GetTerrainCategory());
		edit->AddActivePanel(matTypePanel);
	}
}

void CreateTerrainModeUI::ChooseMatType(ImageChooseRect *icRect)
{
	int categoryIndex = GetTerrainCategory();
	currMatRects[categoryIndex]->SetImage(icRect->ts, icRect->spr.getTextureRect());

	int world = icRect->pos.x / terrainGridSize + categoryIndex * 8;

	if (categoryIndex == TerrainPolygon::CATEGORY_VISUAL)
	{
		world = icRect->pos.x / terrainGridSize;
	}

	int variation = icRect->pos.y / terrainGridSize;

	edit->currTerrainWorld[categoryIndex] = world;
	edit->currTerrainVar[categoryIndex] = variation;

	edit->RemoveActivePanel(matTypePanel);
	edit->justCompletedPolyWithClick = true;
}


void CreateTerrainModeUI::UpdateBrushHotbar()
{
	int i = 0;
	auto &recentBrushes = edit->brushManager->recentBrushes;
	for (auto it = recentBrushes.begin(); it != recentBrushes.end(); ++it)
	{
		brushHotbarImages[i]->SetImage((*it)->ts_preview, 0);
		brushHotbarImages[i]->SetShown(true);
		brushHotbarImages[i]->SetInfo((*it));
		++i;
	}
	int activeHotbarSize = i;
	int maxRecentBrushes = BrushManager::MAX_RECENT_BRUSHES;
	for (; i < maxRecentBrushes; ++i)
	{
		brushHotbarImages[i]->SetShown(false);
		//brushHotbarImages[i]->SetImage(NULL, 0);
	}
}

int CreateTerrainModeUI::GetCurrTerrainTool()
{
	int categoryIndex = GetTerrainCategory();
	if (categoryIndex == TerrainPolygon::CATEGORY_NORMAL )
	{
		return terrainActionDropdown->selectedIndex;
	}
	else if(categoryIndex == TerrainPolygon::CATEGORY_WATER )
	{
		return limitedTerrainActionDropdown->selectedIndex;
	}
	else
	{
		//for pickups. they only need add/subtract rn so its the same as water
		return limitedTerrainActionDropdown->selectedIndex;
	}
	
}

void CreateTerrainModeUI::SetTerrainTool(int t)
{
	int categoryIndex = GetTerrainCategory();

	if (t == EditSession::TERRAINTOOL_SETINVERSE)
	{
		if (categoryIndex != TerrainPolygon::CATEGORY_NORMAL)
		{
			//do nothing!
			return;
		}
	}

	
	if (categoryIndex == TerrainPolygon::CATEGORY_NORMAL)
	{
		terrainActionDropdown->SetSelectedIndex(t);
	}
	else if (categoryIndex == TerrainPolygon::CATEGORY_WATER)
	{
		limitedTerrainActionDropdown->SetSelectedIndex(t);
	}
	else
	{
		limitedTerrainActionDropdown->SetSelectedIndex(t);
	}

	
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

int CreateTerrainModeUI::GetTerrainCategory()
{
	return terrainCategoryDropdown->selectedIndex;
}

int CreateTerrainModeUI::GetCurrDrawTool()
{
	return drawModeDropdown->selectedIndex;
}

void CreateTerrainModeUI::SetDrawTool(int t)
{
	edit->polyDrawStarted = false;

	int sel = drawModeDropdown->selectedIndex;
	if (t != sel)
	{
		if (sel == EditSession::TOOL_BRUSH)
		{
			edit->RemoveActivePanel(brushHotbar);
		}

		drawModeDropdown->SetSelectedIndex(t);
		switch (t)
		{
		case EditSession::TOOL_BOX:
			edit->SetBoxTool();
			break;
		case EditSession::TOOL_BRUSH:
			edit->AddActivePanel(brushHotbar);
			break;
		case EditSession::TOOL_DRAW:
			//edit->polyDrawStarted = false;
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
		minEdgeLenTextbox->SetString(to_string((int)edit->minimumEdgeLength));
	}
	else
	{
		edit->RemoveActivePanel(mainPanel);


	}
}

void CreateTerrainModeUI::SetCategoryTerrain()
{
	limitedTerrainActionDropdown->Deactivate();
	limitedTerrainActionDropdown->HideMember();

	terrainActionDropdown->ShowMember();

	terrainActionDropdown->SetSelectedIndex(GetCurrTerrainTool());
	terrainCategoryDropdown->SetSelectedIndex(0);
	SetCategory(TerrainPolygon::CATEGORY_NORMAL);

	if (matTypePanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(matTypePanel);
		ExpandTerrainLibrary();
	}
}

void CreateTerrainModeUI::SetCategoryWater()
{
	if (GetCurrTerrainTool() == EditSession::TERRAINTOOL_SETINVERSE)
	{
		SetTerrainTool(EditSession::TERRAINTOOL_ADD);
	}

	limitedTerrainActionDropdown->SetSelectedIndex(GetCurrTerrainTool());

	terrainActionDropdown->Deactivate();
	terrainActionDropdown->HideMember();

	limitedTerrainActionDropdown->ShowMember();

	terrainCategoryDropdown->SetSelectedIndex(1);
	SetCategory(TerrainPolygon::CATEGORY_WATER);

	if (matTypePanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(matTypePanel);
		ExpandTerrainLibrary();
	}
}

void CreateTerrainModeUI::SetCategoryPickup()
{
	if (GetCurrTerrainTool() == EditSession::TERRAINTOOL_SETINVERSE)
	{
		SetTerrainTool(EditSession::TERRAINTOOL_ADD);
	}

	terrainActionDropdown->Deactivate();
	terrainActionDropdown->HideMember();

	limitedTerrainActionDropdown->ShowMember();

	terrainCategoryDropdown->SetSelectedIndex(2);
	SetCategory(TerrainPolygon::CATEGORY_ITEM);
}

void CreateTerrainModeUI::SetCategoryVisual()
{
	if (GetCurrTerrainTool() == EditSession::TERRAINTOOL_SETINVERSE)
	{
		SetTerrainTool(EditSession::TERRAINTOOL_ADD);
	}

	terrainActionDropdown->Deactivate();
	terrainActionDropdown->HideMember();

	limitedTerrainActionDropdown->ShowMember();

	terrainCategoryDropdown->SetSelectedIndex(2);
	SetCategory(TerrainPolygon::CATEGORY_VISUAL);
}

void CreateTerrainModeUI::SetCategory(int selectedIndex)
{
	for (int i = 0; i < TerrainPolygon::CATEGORY_Count; ++i)
	{
		currMatRects[i]->SetShown(false);
	}

	currMatRects[selectedIndex]->SetShown(true);

	terrainCategoryDropdown->SetSelectedIndex(selectedIndex);
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
			else if (icRect->rectIdentity == ChooseRect::I_BRUSHHOTBAR )
			{
				NamedBrush *nb = (NamedBrush*)cr->info;
				//nb->myBrush
				edit->DestroyCopiedBrushes();

				edit->copiedBrush = nb->myBrush->CopyTerrainAndAttachedActors();
				edit->freeActorCopiedBrush = nb->myBrush->CopyFreeActors();

				edit->EditModePaste();
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
		if (!ss.fail() && spacing > 0 )
		{
			edit->graph->SetSpacing(spacing);
		}
	}
	else if (tb == minEdgeLenTextbox)
	{
		string str = tb->GetString();
		stringstream ss;
		ss << str;
		int minLen;
		ss >> minLen;
		if (!ss.fail() && minLen > 0 )
		{
			edit->minimumEdgeLength = minLen;
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

void CreateTerrainModeUI::SliderCallback(Slider *slider)
{

}

void CreateTerrainModeUI::DropdownCallback(Dropdown *dropdown, const std::string & e)
{
	if (dropdown == terrainCategoryDropdown)
	{
		SetCategory(dropdown->selectedIndex);
	}
	else if (dropdown == terrainActionDropdown)
	{
		int selectedIndex = dropdown->selectedIndex;
		realTerrainTool = selectedIndex;
	}
	else if (dropdown == limitedTerrainActionDropdown)
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