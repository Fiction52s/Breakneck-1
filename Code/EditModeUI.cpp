#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"
#include "EditorGraph.h"
#include "EditorRail.h"
#include "Actor.h"
#include "ShardMenu.h"
#include "Enemy_Shard.h"

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
	logTypePanel = edit->logTypePanel;

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

	gridSizeTextbox = mainPanel->AddBasicTextBox("gridsize", Vector2i(0, 0), 50, 5, "");
	gridSizeTextbox->SetToolTip("Set the grid spacing");
	SetGridSize(edit->graph->GetSpacing());

	transformBrushButton = mainPanel->AddButton("transform", Vector2i(0, 0), Vector2f(200, 28 + 4), "transform");
	transformBrushButton->SetToolTip("Transform the current brush (N)");

	deleteBrushButton = mainPanel->AddButton("delete", Vector2i(0, 0), Vector2f(200, 28 + 4), "delete selected");
	deleteBrushButton->SetToolTip("Delete the current brush (X / Delete)");

	matPanelPos = Vector2i(960 - matTypePanel->size.x / 2, 540 - matTypePanel->size.y / 2);
	shardPanelPos = Vector2i(960 - shardTypePanel->size.x / 2, 540 - shardTypePanel->size.y / 2);
	logPanelPos = Vector2i(960 - logTypePanel->size.x / 2, 540 - logTypePanel->size.y / 2);

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

	int rectSize = 100;
	int row, col;
	for (int i = 0; i < numGrassTypes; ++i)
	{
		row = i / 4;
		col = i % 4;
		grassRects[i] = grassTypePanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_GRASSLIBRARY,
			Vector2f(col * rectSize, row * rectSize), ts_grassTypes, i, rectSize);
		grassRects[i]->SetInfo((void*)i);
		//physRects[i]->Init();
		grassRects[i]->SetShown(true);
		grassRects[i]->SetName(Grass::GetGrassStringFromType(i));
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


void EditModeUI::AddKinOption( int world, const std::string &text, const std::string &toolTipText, int upgradeIndex)
{
	kinCheckboxes[world][upgradeIndex] = kinOptionsPanel->AddLabeledCheckBox(
		text, Vector2i(0, 0), text, edit->defaultStartingPlayerOptionsField.GetBit(upgradeIndex));
	kinCheckboxes[world][upgradeIndex]->SetToolTip(toolTipText);
}

void EditModeUI::AddKinOptionSlider( int world, const std::string &text,
	const std::string &toolTipText, int upgradeIndex, int maxQuant)
{
	kinSliders[world][upgradeIndex] = kinOptionsPanel->AddLabeledSlider(text, Vector2i(0, 0),
		text, 100, 0, maxQuant, 0, 24);
	kinSliders[world][upgradeIndex]->SetToolTip(toolTipText);
	kinSliders[world][upgradeIndex]->SetInnerDisplayMode(true);
}

void EditModeUI::CreateKinOptionsPanel()
{
	kinOptionsPanel = new Panel("kinoptions", 1920, 1080, this, true);
	kinOptionsPanel->SetPosition(Vector2i(960 - kinOptionsPanel->size.x / 2,
		540 - kinOptionsPanel->size.y / 2 + 20));

	int vertSpacing = 40;//130;//250;
	int inBetweenSpacing = 40;

	int currVerticalSpacing = 10;

	kinOptionsPanel->SetAutoSpacing(true, false, Vector2i(10, currVerticalSpacing), Vector2i(inBetweenSpacing, 0));
	allOptionsCheckbox = kinOptionsPanel->AddLabeledCheckBox("all", Vector2i(0, 0),
		"All Abilities:", false);
	allWorldOptionsCheckBox[0] = kinOptionsPanel->AddLabeledCheckBox("all1", Vector2i(0, 0),
		"All W1:", false);
	allWorldOptionsCheckBox[1] = kinOptionsPanel->AddLabeledCheckBox("all2", Vector2i(0, 0),
		"All W2:", false);
	allWorldOptionsCheckBox[2] = kinOptionsPanel->AddLabeledCheckBox("all3", Vector2i(0, 0),
		"All W3:", false);
	allWorldOptionsCheckBox[3] = kinOptionsPanel->AddLabeledCheckBox("all4", Vector2i(0, 0),
		"All W4:", false);
	allWorldOptionsCheckBox[4] = kinOptionsPanel->AddLabeledCheckBox("all5", Vector2i(0, 0),
		"All W5:", false);
	allWorldOptionsCheckBox[5] = kinOptionsPanel->AddLabeledCheckBox("all6", Vector2i(0, 0),
		"All W6:", false);
	allWorldOptionsCheckBox[6] = kinOptionsPanel->AddLabeledCheckBox("all7", Vector2i(0, 0),
		"All W7:", false);

	currVerticalSpacing += vertSpacing;

	kinOptionsPanel->SetAutoSpacing(true, false, Vector2i(10, currVerticalSpacing), Vector2i(inBetweenSpacing, 0));
	AddKinOption(0, "Airdash:", "Toggle Airdash Power", Actor::UPGRADE_POWER_AIRDASH);
	AddKinOption(1, "Gravity Reverse: ", "Toggle Gravity Reverse Power", Actor::UPGRADE_POWER_GRAV);
	AddKinOption(2, "Bounce Scorpion:", "Toggle Bounce Scorpion Power", Actor::UPGRADE_POWER_BOUNCE);
	AddKinOption(3, "Grind Wheel:", "Toggle Grind Power", Actor::UPGRADE_POWER_GRIND);
	AddKinOption(4, "Time Slow Bubble:", "Toggle Time Slow", Actor::UPGRADE_POWER_TIME);
	AddKinOption(5, "Left Wire:", "Toggle Left Wire Power", Actor::UPGRADE_POWER_LWIRE);
	AddKinOption(5, "Right Wire:", "Toggle Right Wire Power", Actor::UPGRADE_POWER_RWIRE);

	//currVerticalSpacing += vertSpacing;//10;

	int totalShards = Shard::GetNumShardsTotal();
	int w, li;
	string currName;
	int currWorld = -1;
	int upgradeIndex;
	int numUpgradesPerLine = 3;
	int numUpgradesCurrLine = 0;
	for (int i = 0; i < totalShards; ++i)
	{
		w = i / ShardInfo::MAX_SHARDS_PER_WORLD;
		li = i % ShardInfo::MAX_SHARDS_PER_WORLD;

		if (w > currWorld)
		{
			currWorld = w;
			numUpgradesCurrLine = 0;
			currVerticalSpacing += vertSpacing;
			kinOptionsPanel->SetAutoSpacing(true, false, Vector2i(10, 
				currVerticalSpacing), 
				Vector2i(inBetweenSpacing, 0));
		}
		else if( numUpgradesCurrLine == numUpgradesPerLine )
		{
			currVerticalSpacing += vertSpacing;
			kinOptionsPanel->SetAutoSpacing(true, false, Vector2i(10,
				currVerticalSpacing),
				Vector2i(inBetweenSpacing, 0));
			numUpgradesCurrLine = 0;
		}

	
		currName = edit->shardMenu->GetShardName(w, li);
		upgradeIndex = i + (Actor::UPGRADE_POWER_LWIRE + 1);

		if (currName == "")
		{
			continue;
		}
		//multiple shard
		/*if (currName.at(currName.size() - 2) == '/')
		{
			int numberOfUpgrades = currName.at(currName.size() - 1) - '0';

			string modifiedName = currName.substr(0, currName.size() - 4);

			modifiedName += ":";

			AddKinOptionSlider(w, modifiedName,
				edit->shardMenu->GetShardDesc(w, li),
				upgradeIndex, numberOfUpgrades);
			i += (numberOfUpgrades - 1);
		}
		else*/
		{
			string modifiedName = currName + ":";

			AddKinOption(w, modifiedName,
				edit->shardMenu->GetShardDesc( w, li ), upgradeIndex );
		}

		++numUpgradesCurrLine;
	}
	
	
	

	

	//currVerticalSpacing += worldSpacing;
	//kinOptionsPanel->SetAutoSpacing(true, false, Vector2i(10, currVerticalSpacing), Vector2i(inBetweenSpacing, 0));
	////kinOptionsPanel->SetAutoSpacing(false, true, Vector2i(10 + worldSpacing, 10), Vector2i(0, 20));
	//AddKinOption("Dash Boost:", "Toggle Dash Boost", Actor::UPGRADE_W1_DASH_BOOST);
	//AddKinOption("Airdash Boost:", "Toggle Airdash Boost", Actor::UPGRADE_W1_AIRDASH_BOOST);
	//AddKinOption("WJ Restores Doublejump :", "Toggle walljump restoring doublejump", Actor::UPGRADE_W1_WALLJUMP_RESTORES_DOUBLEJUMP);
	//AddKinOption("WJ Restores Airdash:", "Toggle walljump restoring airdash", Actor::UPGRADE_W1_WALLJUMP_RESTORES_AIRDASH);
	////AddKinOptionSlider("Increased Base Dash speed:", "Increase base dash speed", Actor::UPGRADE_W1_INCREASE_BASE_DASH_1, 3);
	//AddKinOptionSlider("Improve highspeed dash boost:", "increase dash boost performance at high speed", Actor::UPGRADE_W1_DASH_BOOST_HIGH_SPEED_1, 3);
	////AddKinOptionSlider("Improve highspeed airdash boost:", "increase airdash boost performance at high speed", Actor::UPGRADE_W1_AIRDASH_BOOST_HIGH_SPEED_1, 3);

	//currVerticalSpacing += worldSpacing;
	//kinOptionsPanel->SetAutoSpacing(true, false, Vector2i(10, currVerticalSpacing), Vector2i(inBetweenSpacing, 0));
	//AddKinOptionSlider("Increase passive ceiling accel:", "increase passive accel on the ceiling", Actor::UPGRADE_W2_INCREASE_PASSIVE_CEILING_ACCEL_1, 3);
	//AddKinOptionSlider("Increase spring on ceiling accel:", "increase sprint accel on the ceiling", Actor::UPGRADE_W2_INCREASE_CEILING_SPRINT_ACCEL_1, 3);

	//currVerticalSpacing += worldSpacing;
	//kinOptionsPanel->SetAutoSpacing(true, false, Vector2i(10, currVerticalSpacing), Vector2i(inBetweenSpacing, 0));
	////kinOptionsPanel->SetAutoSpacing(false, true, Vector2i(10 + worldSpacing * 3, 10), Vector2i(0, 20));
	////AddKinOptionSlider("Max speed increase:", "increases max speed", Actor::UPGRADE_W3_MAX_SPEED_1, 8);
	//AddKinOptionSlider("Increaed passive ground accel:", "Change increased passive ground accel", Actor::UPGRADE_W3_INCREASE_PASSIVE_GROUND_ACCEL_1, 3);
	//AddKinOptionSlider("Increaed sprint accel:", "Change increased sprint accel", Actor::UPGRADE_W3_INCREASE_SPRINT_ACCEL_1, 3);

	//
	////AddKinOption("Dash Boost: ", "Toggle Dash Boost", Actor::UPGRADE_W1_DASH_BOOST);
	////AddKinOption("Airdash Boost: ", "Toggle Airdash Boost", Actor::UPGRADE_W1_AIRDASH_BOOST);
	////AddKinOption("WJ Restores Doublejump : ", "Toggle walljump restoring doublejump", Actor::UPGRADE_W1_WALLJUMP_RESTORES_DOUBLEJUMP);
	////AddKinOption("WJ Restores Airdash: ", "Toggle walljump restoring airdash", Actor::UPGRADE_W1_WALLJUMP_RESTORES_AIRDASH);

	//currVerticalSpacing += worldSpacing;
	//kinOptionsPanel->SetAutoSpacing(true, false, Vector2i(10, currVerticalSpacing), Vector2i(inBetweenSpacing, 0));
	//AddKinOptionSlider("Extra timeslow bubbles:", "increases number of timeslow bubbles", Actor::UPGRADE_W5_MAX_BUBBLES_1, 4);
	//AddKinOption("Infinite Airdash within bubbles:", "Toggle infinite airdash within bubbles", Actor::UPGRADE_W5_INFINITE_AIRDASH_WITHIN_BUBBLES);
	//AddKinOption("Infinite doublejump within bubbles:", "Toggle infinitely refreshing doublejump within bubbles", Actor::UPGRADE_W5_INFINITE_DOUBLEJUMP_WITHIN_BUBBLES);

	//currVerticalSpacing += worldSpacing;
	//kinOptionsPanel->SetAutoSpacing(true, false, Vector2i(10, currVerticalSpacing), Vector2i(inBetweenSpacing, 0));
	//AddKinOption("Wire enemies:", "Allows wire to attach to enemies", Actor::UPGRADE_W6_WIRE_ENEMIES);
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
		LoadKinOptions();
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
		edit->showChoiceNameText = false;
	}
	else
	{
		shardTypePanel->SetPosition(shardPanelPos);
		shardTypePanel->handler = this;

		edit->AddActivePanel(shardTypePanel);
	}
}

void EditModeUI::ExpandLogLibrary()
{
	if (logTypePanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(logTypePanel);
		edit->showChoiceNameText = false;
	}
	else
	{
		logTypePanel->SetPosition(logPanelPos);
		logTypePanel->handler = this;

		edit->AddActivePanel(logTypePanel);
	}
}

void EditModeUI::CreateLayerPanel()
{
	layerPanel = new Panel("layers", 300, 900, this);
	lpSlider = new PanelSlider(layerPanel, Vector2i(-270, 100), Vector2i(0, 100));
	layerPanel->extraUpdater = lpSlider;

	int currLayerIndex = 0;

	layerMap[LAYER_ACTOR] = "actors";
	layerMap[LAYER_SEQUENCE] = "sequence objects";
	layerMap[LAYER_IMAGE] = "images";
	layerMap[LAYER_TERRAIN] = "terrain";
	layerMap[LAYER_WATER] = "water";
	/*layerMap[LAYER_CAMERA] = "camera";
	layerMap[LAYER_POI] = "poi";*/


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
	AddLayerToPanel(layerMap[LAYER_SEQUENCE], 1, startY);
	AddLayerToPanel(layerMap[LAYER_IMAGE], 2, startY);
	AddLayerToPanel(layerMap[LAYER_TERRAIN], 3, startY);
	AddLayerToPanel(layerMap[LAYER_WATER], 4, startY);
	//AddLayerToPanel(layerMap[LAYER_POI], 5, startY);


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
		case LAYER_SEQUENCE:
		{
			edit->DeselectActorType("camerashot");
			edit->DeselectActorType("poi");
			break;
		}
		/*case LAYER_POI:
			
			break;*/

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
		for (int w = 0; w < 7; ++w)
		{
			if (kinCheckboxes[w].count( i ) > 0)
			{
				edit->defaultStartingPlayerOptionsField.SetBit(i, kinCheckboxes[w][i]->checked);
			}
			else if (kinSliders[w].count( i ) > 0 )
			{
				Slider *currSlider = kinSliders[w][i];
				int sliderVal = currSlider->GetCurrValue();
				int maxVal = currSlider->maxValue;
				for (int j = 0; j < maxVal; ++j)
				{
					if (sliderVal > j)
					{
						edit->defaultStartingPlayerOptionsField.SetBit(i + j, true);
					}
					else
					{
						edit->defaultStartingPlayerOptionsField.SetBit(i + j, false);
					}
				}

			}
		}

	}
}

void EditModeUI::LoadKinOptions()
{
	for (int w = 0; w < 7; ++w)
	{
		for (auto it = kinCheckboxes[w].begin(); it != kinCheckboxes[w].end(); ++it)
		{
			if ((*it).second != NULL)
			{
				(*it).second->checked = edit->defaultStartingPlayerOptionsField.GetBit((*it).first);
			}
		}
	}
	

	int currNumOn = 0;
	int currMax;

	for (int w = 0; w < 7; ++w)
	{
		for (auto it = kinSliders[w].begin(); it != kinSliders[w].end(); ++it)
		{
			if ((*it).second != NULL)
			{
				currNumOn = 0;
				currMax = (*it).second->maxValue;
				for (int i = 0; i < currMax; ++i)
				{
					if (edit->defaultStartingPlayerOptionsField.GetBit((*it).first + i))
					{
						currNumOn++;
					}
				}
				(*it).second->SetCurrValue(currNumOn);
			}
		}

		UpdateAllAbilitiesPerWorldCheckboxes(w);
	}


	UpdateAllAbilitiesCheckbox();
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
				int y = icRect->pos.y / (edit->shardGridSize * 2);//*2 because of the spacing being x2

				int world = y / edit->shardNumY;
				int localIndex = (y % edit->shardNumY) * edit->shardNumX + x;

				edit->SetCurrSelectedShardType(world, localIndex);

				edit->RemoveActivePanel(shardTypePanel);

				edit->showChoiceNameText = false;
			}
			else if (icRect->rectIdentity == ChooseRect::I_LOGLIBRARY)
			{
				int x = icRect->pos.x / edit->logGridSize;
				int y = icRect->pos.y / edit->logGridSize;

				int world = y / edit->logNumY;
				int localIndex = (y % edit->logNumY) * edit->logNumX + x;

				edit->SetCurrSelectedLogType(world, localIndex);

				edit->RemoveActivePanel(logTypePanel);

				edit->showChoiceNameText = false;

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
	else if (eventType == ChooseRect::E_FOCUSED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_SHARDLIBRARY)
			{
				edit->showChoiceNameText = true;
				edit->choiceNameText.setString(icRect->nameText.getString());
				edit->choiceNameText.setOrigin(edit->choiceNameText.getLocalBounds().left
					+ edit->choiceNameText.getLocalBounds().width / 2, 0);
				edit->choiceNameText.setPosition(icRect->GetGlobalCenterPos() + Vector2f(0, -(icRect->boxSize.y + 20)));
			}
			else if (icRect->rectIdentity == ChooseRect::I_LOGLIBRARY)
			{
				edit->showChoiceNameText = true;
				edit->choiceNameText.setString(icRect->nameText.getString());
				edit->choiceNameText.setOrigin(edit->choiceNameText.getLocalBounds().left
					+ edit->choiceNameText.getLocalBounds().width / 2, 0);
				edit->choiceNameText.setPosition(icRect->GetGlobalCenterPos() + Vector2f(0, -(icRect->boxSize.y + 20)));
			}
		}
	}
	else if (eventType == ChooseRect::E_UNFOCUSED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_LOGLIBRARY)
			{
				//edit->showLogNameText = false;
			}
		}
	}
	
	/*else if (eventType == ChooseRect::E_FOCUSED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_LOGLIBRARY)
			{
				icRect->ShowName(true);
			}
		}
	}
	else if (eventType == ChooseRect::E_UNFOCUSED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_LOGLIBRARY)
			{
				icRect->ShowName(false);
			}
		}
	}*/
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
	else if (tb->panel == currEnemyPanel)
	{
		edit->UpdateCurrEnemyParamsFromPanel();
	}
}

void EditModeUI::GridSelectorCallback(GridSelector *gs, const std::string & e)
{

}

void EditModeUI::CheckBoxCallback(CheckBox *cb, const std::string & e)
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
	else if (cb->panel == kinOptionsPanel)
	{
		if (cb == allOptionsCheckbox)
		{
			for (int w = 0; w < 7; ++w)
			{
				for (auto it = kinCheckboxes[w].begin(); it != kinCheckboxes[w].end(); ++it)
				{
					if ((*it).second != NULL)
					{
						(*it).second->checked = isChecked;
					}
				}
				for (auto it = kinSliders[w].begin(); it != kinSliders[w].end(); ++it)
				{
					if ((*it).second != NULL)
					{
						if (isChecked)
						{
							(*it).second->SetCurrValue((*it).second->maxValue);
						}
						else
						{
							(*it).second->SetCurrValue((*it).second->minValue);
						}
					}
				}
			}

			for (int i = 0; i < 7; ++i)
			{
				UpdateAllAbilitiesPerWorldCheckboxes(i);
			}
		}
		else
		{
			int foundIndex = -1;

			for (int w = 0; w < 7; ++w)
			{
				if (cb == allWorldOptionsCheckBox[w])
				{
					foundIndex = w;
					for (auto it = kinCheckboxes[w].begin(); it != kinCheckboxes[w].end(); ++it)
					{
						if ((*it).second != NULL)
						{
							(*it).second->checked = isChecked;
						}
						
					}
					for (auto it = kinSliders[w].begin(); it != kinSliders[w].end(); ++it)
					{
						if ((*it).second != NULL)
						{
							if (isChecked)
							{
								(*it).second->SetCurrValue((*it).second->maxValue);
							}
							else
							{
								(*it).second->SetCurrValue((*it).second->minValue);
							}
						}
					}
					break;
				}
			}

			
			for (int i = 0; i < 7; ++i)
			{
				if (i == foundIndex)
				{
					continue;
				}
				UpdateAllAbilitiesPerWorldCheckboxes(i);
			}

			UpdateAllAbilitiesCheckbox();
		}
	}

	

	
	/*if (currParams != NULL)
	{
		currParams->CheckBoxCallback(cb, e);
	}*/
}

void EditModeUI::UpdateAllAbilitiesCheckbox()
{
	bool allChecked = true;
	for (int i = 0; i < Actor::UPGRADE_Count; ++i)
	{
		if (!edit->defaultStartingPlayerOptionsField.GetBit(i))
		{
			allChecked = false;
			break;
		}
	}

	allOptionsCheckbox->checked = allChecked;

	if (allChecked)
	{
		for (int w = 0; w < 7; ++w)
		{
			allWorldOptionsCheckBox[w]->checked = true;
		}
	}
}

void EditModeUI::UpdateAllAbilitiesPerWorldCheckboxes( int w)
{
	bool allChecked = true;
	int shardIndex;

	for (auto it = kinCheckboxes[w].begin(); it != kinCheckboxes[w].end(); ++it)
	{
		if ((*it).second != NULL)
		{
			if (!edit->defaultStartingPlayerOptionsField.GetBit((*it).first))
			{
				allChecked = false;
				break;
			}
		}
	}
	for (auto it = kinSliders[w].begin(); it != kinSliders[w].end(); ++it)
	{
		if ((*it).second != NULL)
		{
			int maxVal = (*it).second->maxValue;
			for (int i = 0; i < maxVal; ++i)
			{
				if (!edit->defaultStartingPlayerOptionsField.GetBit((*it).first + i))
				{
					allChecked = false;
					break;
				}
			}

			if (!allChecked)
			{
				break;
			}
		}
	}

	allWorldOptionsCheckBox[w]->checked = allChecked;
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
		edit->showChoiceNameText = false;
	}
}