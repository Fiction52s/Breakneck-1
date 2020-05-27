#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"
#include "EditorGraph.h"

using namespace std;
using namespace sf;

EditModeUI::EditModeUI()
{
	edit = EditSession::GetSession();
	mainPanel = new Panel("edit", 1920, 200, this, false);

	CreateLayerPanel();

	gridCheckbox = mainPanel->AddCheckBox("grid", Vector2i(10, 10), false);
	editPointsCheckbox = mainPanel->AddCheckBox("editpoints", Vector2i(50, 10), false);
	gridSizeTextbox = mainPanel->AddTextBox("gridisize", Vector2i(100, 10), 50, 5, "");
	transformSelected = mainPanel->AddButton("transform", Vector2i(200, 10), Vector2f(80, 80), "transform");
	deleteSelectedButton = mainPanel->AddButton("delete", Vector2i(300, 10), Vector2f(80, 80), "delete selected");

	terrainGridSize = 64;

	int numTerrainLayers = TERRAINLAYER_Count;
	currMatRects.resize(numTerrainLayers);
	mainPanel->ReserveImageRects(numTerrainLayers);
	Vector2f currMatRectPos = Vector2f(terrainLayerDropdown->pos)
		+ Vector2f(terrainLayerDropdown->size.x + 20, 0);
	for (int i = 0; i < numTerrainLayers; ++i)
	{
		currMatRects[i] = mainPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_TERRAINSEARCH,
			currMatRectPos, NULL, 0, 100);
		currMatRects[i]->Init();
		currMatRects[i]->SetImage(edit->GetMatTileset(edit->currTerrainWorld[i],
			edit->currTerrainVar[i]), IntRect(0, 0, 128, 128));
	}

	currMatRects[0]->SetShown(true);

	//matTypePanel->SetPosition(Vector2i(currMatRectPos.x, currMatRectPos.y + 100 + 10));


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
				ChooseRect::ChooseRectIdentity::I_TERRAINLIBRARY,
				Vector2f(worldI * terrainGridSize, i * terrainGridSize),
				edit->GetMatTileset(worldI, i),
				IntRect(0, 0, 128, 128),
				terrainGridSize);
			matTypeRects[ind]->Init();
			if (matTypeRects[ind]->ts != NULL)
			{
				matTypeRects[ind]->SetShown(true);
			}
		}
	}
}

EditModeUI::~EditModeUI()
{
	delete mainPanel;
	delete layerPanel;
	delete lpSlider;
}

void EditModeUI::ExpandTerrainLibrary()
{
	//edit->AddActivePanel(matTypePanel);
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
	//int layerIndex = GetTerrainLayer();
	//currMatRects[layerIndex]->SetImage(icRect->ts, icRect->spr.getTextureRect());

	int layerIndex = 0;
	int world = icRect->pos.x / terrainGridSize;
	int variation = icRect->pos.y / terrainGridSize;

	edit->currTerrainWorld[layerIndex] = world;
	edit->currTerrainVar[layerIndex] = variation;

	//edit->RemoveActivePanel(matTypePanel);
	edit->justCompletedPolyWithClick = true;
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

void EditModeUI::SetShown(bool s)
{
	show = s;
	if (show)
	{
		edit->AddActivePanel(mainPanel);
		edit->AddActivePanel(layerPanel);
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
		//edit->ExecuteTerrainCompletion();
	}
	else if (b == transformBrushButton)
	{
		//edit->removeProgressPointWaiter->Reset();
		//edit->RemovePointFromPolygonInProgress();
	}
	else if (b == copyBrushButton)
	{
		//edit->ClearPolygonInProgress();
	}
	else if (b == pasteBrushButton)
	{
		//edit->ClearPolygonInProgress();
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
	if (cb->panel == layerPanel)
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
}



void EditModeUI::SliderCallback(Slider *slider, const std::string & e)
{

}

void EditModeUI::DropdownCallback(Dropdown *dropdown, const std::string & e)
{
}

void EditModeUI::PanelCallback(Panel *p, const std::string & e)
{
	/*if (e == "leftclickoffpopup" && p == matTypePanel)
	{
		edit->RemoveActivePanel(matTypePanel);
	}*/
}