#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"
#include "EditorGraph.h"
#include "EditorRail.h"
#include "Actor.h"

using namespace std;
using namespace sf;

EditModeUI::EditModeUI()
{
	edit = EditSession::GetSession();


	nameBrushPanel = edit->nameBrushPanel;
	terrainGridSize = edit->terrainGridSize;
	matTypePanel = edit->matTypePanel;
	//grassTypePanel = edit->grassTypePanel;

	shardTypePanel = edit->shardTypePanel;

	mainPanel = new Panel("edit", /*1310*/1500, 120, this, false);
	mainPanel->SetPosition(Vector2i(0, edit->generalUI->height));
	mainPanel->SetAutoSpacing(true, false, Vector2i(10, 10), Vector2i(20, 0 ));
	mainPanel->ReserveImageRects(1);

	CreateLayerPanel();

	currEnemyPanel = NULL;
	currRail = NULL;
	currParams = NULL;

	labelCharHeight = 24;
	labelExtraSpacing = 30;
	labelExtraY = 10;
	labelExtra = Vector2i(30, 10);

	mainPanel->AddLabel("movelabel", Vector2i( 0, labelExtra.y ), labelCharHeight, "Move Tool:");
	moveToolCheckbox = mainPanel->AddCheckBox("move", Vector2i(0, 0), true);
	moveToolCheckbox->SetToolTip("Toggle Move Tool (Q)");

	mainPanel->AddLabel("grasslabel", labelExtra, labelCharHeight, "Edit Grass:");
	showGrassCheckbox = mainPanel->AddCheckBox("showgrass", Vector2i(0, 0), false);
	showGrassCheckbox->SetToolTip("Toggle Editable Grass (R)");

	mainPanel->AddLabel("pointslabel", labelExtra, labelCharHeight, "Edit Points:");
	editPointsCheckbox = mainPanel->AddCheckBox("editpoints", Vector2i(0, 0), false);
	editPointsCheckbox->SetToolTip("Toggle Edit Point Mode (B)");

	mainPanel->AddLabel("gridlabel", labelExtra, labelCharHeight, "Grid:");
	gridCheckbox = mainPanel->AddCheckBox("grid", Vector2i(0, 0), false);
	gridCheckbox->SetToolTip("Toggle Grid (G)");

	gridSizeTextbox = mainPanel->AddTextBox("gridsize", Vector2i(0, 0), 50, 5, "");
	gridSizeTextbox->SetToolTip("Set the grid spacing");
	SetGridSize(edit->graph->GetSpacing());

	transformBrushButton = mainPanel->AddButton("transform", Vector2i(0, 0), Vector2f(200, 28 + 4), "transform");
	transformBrushButton->SetToolTip("Transform the current brush (N)");

	deleteBrushButton = mainPanel->AddButton("delete", Vector2i(0, 0), Vector2f(200, 28 + 4), "delete selected");
	deleteBrushButton->SetToolTip("Delete the current brush (X / Delete)");

	matPanelPos = Vector2i(960 - matTypePanel->size.x / 2, 540 - matTypePanel->size.y / 2);
	shardPanelPos = Vector2i(960 - shardTypePanel->size.x / 2, 540 - shardTypePanel->size.y / 2);

	Vector2f currTypeRectPos = Vector2f(0, 0);

	currGrassTypeRect = mainPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_GRASSSEARCH,
		currTypeRectPos, NULL, 0, 100);
	currGrassTypeRect->Init();

	currGrassTypeRect->SetShown(true);

	Tileset *ts_grassTypes = edit->GetSizedTileset("Env/grass_128x128.png");

	grassTypePanel = new Panel("type", 600, 600, this, true);
	int numGrassTypes = Grass::Count;
	grassTypePanel->ReserveImageRects(numGrassTypes);
	grassRects.resize(numGrassTypes);
	for (int i = 0; i < numGrassTypes; ++i)
	{
		grassRects[i] = grassTypePanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_GRASSLIBRARY,
			Vector2f(0, i * 80), ts_grassTypes, i, 80);
		grassRects[i]->SetInfo((void*)i);
		//physRects[i]->Init();
		grassRects[i]->SetShown(true);
	}

	grassTypePanel->SetPosition(Vector2i(currGrassTypeRect->pos) + Vector2i(0, 120));

	SetGrassType(grassRects[0]);

	CreateKinOptionsPanel();
}


EditModeUI::~EditModeUI()
{
	delete mainPanel;
	delete layerPanel;
	delete lpSlider;
	delete kinOptionsPanel;

	delete grassTypePanel;
}


void EditModeUI::AddKinOption(const std::string &text, const std::string &toolTipText, int upgradeIndex)
{
	kinCheckboxes[upgradeIndex] = kinOptionsPanel->AddLabeledCheckBox(
		text, Vector2i(0, 0), text, edit->playerOptionsField.GetBit(upgradeIndex));
	kinCheckboxes[upgradeIndex]->SetToolTip(toolTipText);
}

void EditModeUI::CreateKinOptionsPanel()
{
	kinOptionsPanel = new Panel("kinoptions", 600, 800, this, true);
	kinOptionsPanel->SetPosition(Vector2i(960 - kinOptionsPanel->size.x / 2,
		540 - kinOptionsPanel->size.y / 2));
	kinOptionsPanel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));

	kinCheckboxes.resize(Actor::UPGRADE_Count);

	AddKinOption("Airdash: ", "Toggle Airdash Power", Actor::UPGRADE_POWER_AIRDASH);
	AddKinOption("Gravity Cling: ", "Toggle Gravity Cling Power", Actor::UPGRADE_POWER_GRAV);
	AddKinOption("Bounce Scorpion: ", "Toggle Bounce Scorpion Power", Actor::UPGRADE_POWER_BOUNCE);
	AddKinOption("Grind Wheel: ", "Toggle Grind Power", Actor::UPGRADE_POWER_GRIND);
	AddKinOption("Time Slow Bubbles ", "Toggle Time Slow", Actor::UPGRADE_POWER_TIME);
	AddKinOption("Left Wire: ", "Toggle Left Wire Power", Actor::UPGRADE_POWER_LWIRE);
	AddKinOption("Right Wire: ", "Toggle Right Wire Power", Actor::UPGRADE_POWER_RWIRE);

	kinOptionsPanel->SetAutoSpacing(false, true, Vector2i(300, 10), Vector2i(0, 20));
	AddKinOption("Dash Boost: ", "Toggle Dash Boost", Actor::UPGRADE_W1_DASH_BOOSTER_1);
	AddKinOption("Airdash Boost: ", "Toggle Airdash Boost", Actor::UPGRADE_W1_AIRDASH_BOOSTER_1);
}

void EditModeUI::ToggleKinOptionsPanel()
{
	if (kinOptionsPanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(kinOptionsPanel);
		SaveKinOptions();
	}
	else
	{
		edit->AddActivePanel(kinOptionsPanel);
	}
}

void EditModeUI::ExpandTerrainLibrary( int layer )
{
	if (matTypePanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(matTypePanel);
	}
	else
	{
		edit->SetMatTypePanelLayer(layer);
		matTypePanel->SetPosition(matPanelPos);
		matTypePanel->handler = this;

		edit->AddActivePanel(matTypePanel);
	}
}

void EditModeUI::ExpandGrassLibrary()
{
	if (grassTypePanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(grassTypePanel);
	}
	else
	{
		grassTypePanel->handler = this;
		edit->AddActivePanel(grassTypePanel);
	}
}

void EditModeUI::ExpandShardLibrary()
{
	if (shardTypePanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(shardTypePanel);
	}
	else
	{
		shardTypePanel->SetPosition(shardPanelPos);
		shardTypePanel->handler = this;

		edit->AddActivePanel(shardTypePanel);
	}
}

void EditModeUI::CreateLayerPanel()
{
	layerPanel = new Panel("layers", 300, 900, this);
	lpSlider = new PanelSlider(layerPanel, Vector2i(-270, 100), Vector2i(0, 100));
	layerPanel->extraUpdater = lpSlider;

	int currLayerIndex = 0;

	layerMap[LAYER_ACTOR] = "actors";
	layerMap[LAYER_IMAGE] = "images";
	layerMap[LAYER_TERRAIN] = "terrain";
	layerMap[LAYER_WATER] = "water";
	layerMap[LAYER_CAMERA] = "camera";
	layerMap[LAYER_POI] = "poi";


	terrainEditLayerMap[TERRAINLAYER_NORMAL] = LAYER_TERRAIN;
	terrainEditLayerMap[TERRAINLAYER_WATER] = LAYER_WATER;

	for (auto it = terrainEditLayerMap.begin(); it != terrainEditLayerMap.end(); ++it)
	{
		terrainEditLayerReverseMap[(*it).second] = (*it).first;
	}

	for (auto it = layerMap.begin(); it != layerMap.end(); ++it)
	{
		reverseLayerMap[(*it).second] = (*it).first;
	}

	vector<string> dropdownOptions{ "testing", "shephard", "water", "terrain" };
	layerPanel->AddDropdown("test", Vector2i(10, 20), Vector2i(150, 24), dropdownOptions, 0);

	float maxFloatValue = 5.f;
	float floatStep = .25;
	float floatStart = 0;
	int numSpaces = (maxFloatValue - floatStep) / floatStep + 1;
	//Slider *sl = layerPanel->AddSlider("testslider", Vector2i(10, 400), 200, 0, numSpaces, 50);
	//Slider *sl = layerPanel->AddFloatSlider("testslider", Vector2i(10, 400), 200, 0, 5.0, 2.5, .25);
	//sl->SetDecimalLabeling(floatStart, floatStep);

	int startY = 60 + 100;

	layerPanel->AddLabel("show", Vector2i(10, startY - 20), 16, "show");
	layerPanel->AddLabel("lock", Vector2i(50, startY - 20), 16, "lock");

	AddLayerToPanel(layerMap[LAYER_ACTOR], 0, startY);
	AddLayerToPanel(layerMap[LAYER_IMAGE], 1, startY);
	AddLayerToPanel(layerMap[LAYER_TERRAIN], 2, startY);
	AddLayerToPanel(layerMap[LAYER_WATER], 3, startY);
	AddLayerToPanel(layerMap[LAYER_CAMERA], 4, startY);
	AddLayerToPanel(layerMap[LAYER_POI], 5, startY);


	layerPanel->checkBoxes[GetLayerShowName(LAYER_ACTOR)]->SetLockedStatus(true, true);
}

std::string EditModeUI::GetLayerShowName(int layer)
{
	return layerMap[layer] + "_show";
}

std::string EditModeUI::GetLayerLockedName(int layer)
{
	return layerMap[layer] + "_lock";
}

void EditModeUI::AddLayerToPanel(const std::string &name, int currLayerIndex, int startY)
{
	int label = 100;
	int checkbox0 = 10;
	int checkbox1 = 50;
	string show = "_show";
	string lock = "_lock";
	int posY = currLayerIndex * 50 + startY;
	layerPanel->AddCheckBox(name + show, Vector2i(checkbox0, posY), true);
	layerPanel->AddCheckBox(name + lock, Vector2i(checkbox1, posY), false);
	layerPanel->AddLabel(name, Vector2i(label, posY), 20, name);
}

void EditModeUI::ChooseMatType(ImageChooseRect *icRect)
{
	int world = icRect->pos.x / terrainGridSize;
	int variation = icRect->pos.y / terrainGridSize;

	if (edit->matTypeRectsCurrLayer == TERRAINLAYER_WATER)
	{
		world += 8;
	}
	else if (edit->matTypeRectsCurrLayer == TERRAINLAYER_FLY)
	{
		world += 9;
	}

	edit->ModifySelectedTerrainMat(world, variation);

	edit->RemoveActivePanel(matTypePanel);

	edit->justCompletedPolyWithClick = true;
}

void EditModeUI::SetEnemyPanel(ActorParams * ap )
{
	Panel *p = NULL;
	if( ap != NULL )
		p = ap->type->panel;

	if (currEnemyPanel != NULL && currEnemyPanel != p)
	{
		edit->RemoveActivePanel(currEnemyPanel);
	}
	else if (p != currEnemyPanel && p != NULL)
	{
		ap->SetPanelInfo();
		p->SetPosition(Vector2i(0, mainPanel->pos.y + mainPanel->size.y));
		edit->AddActivePanel(p);
		p->handler = this;
	}

	currEnemyPanel = p;
	currParams = ap;
	currRail = NULL;
}

void EditModeUI::SetCurrRailPanel(TerrainRail *tr)
{
	if (tr == NULL)
	{
		SetEnemyPanel(NULL);
		return;
	}
		
	if (tr->enemyParams != NULL)
	{
		tr->enemyParams->SetPanelInfo();
		SetEnemyPanel(tr->enemyParams);
		currRail = tr;
	}
}



bool EditModeUI::IsGridOn()
{
	return gridCheckbox->checked;
}

void EditModeUI::FlipGrid()
{
	gridCheckbox->checked = !gridCheckbox->checked;
}

void EditModeUI::SetGridSize(int gs)
{
	gridSizeTextbox->SetString(to_string(gs));
}

bool EditModeUI::IsEditPointsOn()
{
	return editPointsCheckbox->checked;
}

void EditModeUI::FlipEditPoints()
{
	editPointsCheckbox->checked = !editPointsCheckbox->checked;
}

bool EditModeUI::IsShowGrassOn()
{
	return showGrassCheckbox->checked;
}

void EditModeUI::FlipShowGrass()
{
	showGrassCheckbox->checked = !showGrassCheckbox->checked;
}

bool EditModeUI::IsMoveOn()
{
	return moveToolCheckbox->checked;
}

void EditModeUI::FlipMove()
{
	moveToolCheckbox->checked = !moveToolCheckbox->checked;
}

void EditModeUI::SetShown(bool s)
{
	show = s;
	if (show)
	{
		edit->AddActivePanel(mainPanel);
		edit->AddActivePanel(layerPanel);
		if (currEnemyPanel != NULL)
		{
			edit->AddActivePanel(currEnemyPanel);
		}
		//SetEnemyPanel(NULL);
	}
	else
	{
		edit->RemoveActivePanel(mainPanel);
		edit->RemoveActivePanel(layerPanel);

	}
}

bool EditModeUI::IsLayerActionable(int layer)
{
	return (IsLayerShowing(layer) && !IsLayerLocked(layer));
}

bool EditModeUI::IsLayerShowing(int layer)
{
	string checkName = GetLayerShowName(layer);
	assert(layerPanel->checkBoxes.find(checkName) != layerPanel->checkBoxes.end());
	return layerPanel->checkBoxes[checkName]->checked;
}

bool EditModeUI::IsLayerLocked(int layer)
{
	string checkName = GetLayerLockedName(layer);
	assert(layerPanel->checkBoxes.find(checkName) != layerPanel->checkBoxes.end());
	return layerPanel->checkBoxes[checkName]->checked;
}

void EditModeUI::UpdateLayerShow(int layer, bool show)
{
	if (show)
	{
		switch (layer)
		{
		case LAYER_ACTOR:
			break;
		}
	}
	else
	{
		UpdateLayerLock(layer, true);
		/*switch (layer)
		{
		case LAYER_ACTOR:
			edit->DeselectObjectType(ISelectable::ACTOR);
			break;
		case LAYER_CAMERA:
		{
			edit->DeselectActorType("camerashot");
			break;
		}
		case LAYER_POI:
			edit->DeselectActorType("poi");
			break;
			
		}*/
	}


}

void EditModeUI::UpdateLayerLock(int layer, bool lock)
{
	if (lock)
	{
		switch (layer)
		{
		case LAYER_ACTOR:
			edit->DeselectObjectType(ISelectable::ACTOR);
			break;
		case LAYER_CAMERA:
		{
			edit->DeselectActorType("camerashot");
			break;
		}
		case LAYER_POI:
			edit->DeselectActorType("poi");
			break;

		}
	}
	else
	{
	}
}

void EditModeUI::SaveKinOptions()
{
	for (int i = 0; i < Actor::UPGRADE_Count; ++i)
	{
		if (kinCheckboxes[i] != NULL)
		{
			edit->playerOptionsField.SetBit( i, kinCheckboxes[i]->checked);
		}
	}
}

void EditModeUI::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::E_LEFTCLICKED ||
		eventType == ChooseRect::E_LEFTRELEASED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_TERRAINLIBRARY)
			{
				ChooseMatType(icRect);
			}
			else if(icRect->rectIdentity == ChooseRect::I_SHARDLIBRARY)
			{
				int x = icRect->pos.x / edit->shardGridSize;
				int y = icRect->pos.y / edit->shardGridSize;

				int world = y / edit->shardNumY;
				int localIndex = (y % edit->shardNumY) * edit->shardNumX + x;

				edit->SetCurrSelectedShardType(world, localIndex);

				edit->RemoveActivePanel(shardTypePanel);
			}
			else if (icRect->rectIdentity == ChooseRect::I_GRASSSEARCH
				&& eventType == ChooseRect::E_LEFTCLICKED)
			{
				ExpandGrassLibrary();
			}
			else if (icRect->rectIdentity == ChooseRect::I_GRASSLIBRARY)
			{
				SetGrassType(icRect);
			}
		}


	}
}

void EditModeUI::SetGrassType(ImageChooseRect *icRect)
{
	int index = (int)icRect->info;

	edit->currGrassType = index;

	currGrassTypeRect->SetImage(icRect->ts, icRect->spr.getTextureRect());

	edit->RemoveActivePanel(grassTypePanel);

	edit->justCompletedPolyWithClick = true;
}

void EditModeUI::ButtonCallback(Button *b, const std::string & e)
{
	if (b == deleteBrushButton)
	{
		edit->EditModeDelete();
	}
	else if (b == transformBrushButton)
	{
		edit->EditModeTransform();
	}
	else if (b == copyBrushButton)
	{
		edit->EditModeCopy();
	}
	else if (b == pasteBrushButton)
	{
		edit->EditModePaste();
	}
	else if (b->name == "createpath" || b->name == "createrail")
	{
		edit->CreatePathButton();
	}
	else if (b->name == "createchain")
	{
		if (currRail != NULL)
		{
			edit->railInProgress->SetRailToActorType(currRail->enemyParams);
			edit->CreateChainButton(currRail->enemyParams);
		}
		else
		{
			edit->CreateChainButton();
		}
	}
	else if (b->name == "setdirection")
	{
		edit->SetDirectionButton();
	}
	else if (b->name == "setzoom")
	{
		edit->SetZoomButton();
	}

	/*if (currParams != NULL)
	{
		currParams->ButtonCallback(b, e);
	}*/
}

void EditModeUI::TextBoxCallback(TextBox *tb, const std::string & e)
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

void EditModeUI::GridSelectorCallback(GridSelector *gs, const std::string & e)
{

}

void EditModeUI::CheckBoxCallback(CheckBox *cb, const std::string & e)
{
	if (cb->panel == currEnemyPanel)
	{
		if (currRail)
		{
			currRail->enemyParams->SetParams();
			currRail->UpdateEnemyChain();
		}
		else
		{
			edit->UpdateCurrEnemyParamsFromPanel();
		}
	}
	else if (cb->panel == layerPanel)
	{
		bool isChecked = false;
		if (e == "checked")
		{
			isChecked = true;
		}
		else if (e == "unchecked")
		{
			isChecked = false;
		}
		else
		{
			assert(0);
		}


		int found = cb->name.find('_');
		if (found == string::npos)
		{
			assert(0);
		}
		else
		{
			string layerStr = cb->name.substr(0, found);
			string sub = cb->name.substr(found + 1);
			if (sub == "lock")
			{
				UpdateLayerLock(reverseLayerMap[layerStr], isChecked);
			}
			else if (sub == "show")
			{
				UpdateLayerShow(reverseLayerMap[layerStr], isChecked);
			}
			else
			{
				assert(0);
			}
		}
	}
	else if (cb->panel == mainPanel)
	{
		if (cb == showGrassCheckbox)
		{
			edit->ShowGrass(cb->checked);
		}
		else if (cb == gridCheckbox)
		{
			edit->showGraph = cb->checked;
		}
	}

	/*if (currParams != NULL)
	{
		currParams->CheckBoxCallback(cb, e);
	}*/
}

void EditModeUI::SliderCallback(Slider *slider)
{
	/*if (currParams != NULL)
	{
		currParams->SliderCallback(slider, e);
	}*/

	if (currEnemyPanel != NULL && slider->panel == currEnemyPanel)
	{
		if (currRail)
		{
			currRail->enemyParams->SetParams();
			currRail->UpdateEnemyChain();
		}
		else
		{
			edit->UpdateCurrEnemyParamsFromPanel();
		}
	}

	
}

void EditModeUI::DropdownCallback(Dropdown *dropdown, const std::string & e)
{
	if (currEnemyPanel != NULL && dropdown->panel == currEnemyPanel)
	{
		if (currRail)
		{
			currRail->enemyParams->SetParams();
			currRail->UpdateEnemyChain();
		}
		else
		{
			edit->UpdateCurrEnemyParamsFromPanel();
		}
	}
}

void EditModeUI::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup" )
	{
		if (p == kinOptionsPanel)
		{
			SaveKinOptions();
		}
		edit->RemoveActivePanel(p);
	}
}