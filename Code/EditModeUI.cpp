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

EditModeUI::EditModeUI()
{
	edit = EditSession::GetSession();

	terrainGridSize = edit->terrainGridSize;
	matTypePanel = edit->matTypePanel;

	mainPanel = new Panel("edit", 1310, 50, this, false);
	mainPanel->SetPosition(Vector2i(0, edit->generalUI->height));
	mainPanel->SetAutoSpacing(true, false, Vector2i(10, 10), Vector2i(20, 0 ));

	CreateLayerPanel();

	currEnemyPanel = NULL;
	currRail = NULL;

	int labelCharHeight = 24;
	int labelExtraSpacing = 30;
	int labelExtraY = 10;
	Vector2i labelExtra(30, 10);

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
}

EditModeUI::~EditModeUI()
{
	delete mainPanel;
	delete layerPanel;
	delete lpSlider;
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

	layerPanel->AddSlider("testslider", Vector2i(10, 400), 200, 0, 100, 50);

	int startY = 60 + 100;

	layerPanel->AddLabel("show", Vector2i(10, startY - 20), 16, "show");
	layerPanel->AddLabel("lock", Vector2i(50, startY - 20), 16, "lock");

	AddLayerToPanel(layerMap[LAYER_ACTOR], 0, startY);
	AddLayerToPanel(layerMap[LAYER_IMAGE], 1, startY);
	AddLayerToPanel(layerMap[LAYER_TERRAIN], 2, startY);
	AddLayerToPanel(layerMap[LAYER_WATER], 3, startY);


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
		edit->AddActivePanel(p);
		p->handler = this;
	}

	currEnemyPanel = p;
	currRail = NULL;
}

void EditModeUI::SetCurrRailPanel(TerrainRail *tr)
{
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
		switch (layer)
		{
		case LAYER_ACTOR:
			edit->DeselectObjectType(ISelectable::ACTOR);
			break;
		}
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
		}
	}
	else
	{
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
		}
	}
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
	else if (b->name == "setdirection")
	{
		edit->SetDirectionButton();
	}
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
}

void EditModeUI::SliderCallback(Slider *slider)
{
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
	/*if (e == "leftclickoffpopup" && p == matTypePanel)
	{
		edit->RemoveActivePanel(matTypePanel);
	}*/
}