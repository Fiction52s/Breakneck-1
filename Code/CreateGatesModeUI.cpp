#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"
#include "EditorGraph.h"
#include "Action.h"
#include "VectorMath.h"

using namespace std;
using namespace sf;

CreateGatesModeUI::CreateGatesModeUI()
{
	edit = EditSession::GetSession();
	mainPanel = new Panel("creategates", 1190, 120, this, false);
	mainPanel->SetPosition(Vector2i(0, edit->generalUI->height));

	mainPanel->SetAutoSpacing(true, false, Vector2i(10, 10), Vector2i(20, 0));

	shardGateTypePanel = edit->shardTypePanel;
	shardGateTypeRects = &edit->shardTypeRects;

	int labelCharHeight = 24;
	int labelExtraSpacing = 30;
	int labelExtraY = 10;
	Vector2i labelExtra(30, 10);
	currVariation.resize(Gate::Count);

	gateGridSize = 64;
	modifyGate = NULL;
	origModifyGate = new GateInfo;

	modifyGateRect.setFillColor(Color::Transparent);
	modifyGateRect.setOutlineThickness(-5);
	modifyGateRect.setOutlineColor(Color::Green);

	ts_gateCategories = edit->GetSizedTileset("Editor/gatecategories_128x128.png");
	ts_bossGateTypes = edit->GetSizedTileset("Editor/bossgatetypes_128x128.png");
	ts_pickupGateTypes = edit->GetSizedTileset("Editor/pickupgatetypes_128x128.png");

	std::vector<string> gateCatOptions = { "All keys", "Number of keys", "Enemy", "Global time", "Room time", "Shard", "Boss", "Secret", "Pickup", "Black", "Power" };
	gateCategoryDropdown = mainPanel->AddDropdown(
		"catdrop", Vector2i(0, 0), Vector2i(200, 28), gateCatOptions, 0);
	gateCategoryDropdown->SetToolTip("Choose the gate category\n(E to expand types)");

	int numGateCategories = gateCatOptions.size();
	currGateTypeRects.resize(numGateCategories);
	mainPanel->ReserveImageRects(numGateCategories);

	mainPanel->PauseAutoSpacing();

	currGateTypeRectPos = Vector2f(0, 0);/*Vector2f(gateCategoryDropdown->pos)
										 + Vector2f(gateCategoryDropdown->size.x + 20, 0);*/
	for (int i = 0; i < numGateCategories; ++i)
	{
		if (i == numGateCategories - 1)
		{
			mainPanel->UnpauseAutoSpacing();
		}
		currGateTypeRects[i] = mainPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_GATESEARCH,
			currGateTypeRectPos, ts_gateCategories, i, 100);
		currGateTypeRects[i]->Init();
	}
	currGateTypeRects[0]->SetShown(true);

	numToOpen = 0;
	seconds = 0;


	mainPanel->AddLabel("numkeyslabel", Vector2i( 0, labelExtraY ), labelCharHeight, "Number of keys/pickups:");
	numToOpenTextbox = mainPanel->AddBasicTextBox("numtoopen", Vector2i(0, 0), 50, 5, "");
	numToOpenTextbox->SetToolTip("Set number of keys/pickups to open a gate");
	numToOpenTextbox->SetString(to_string(numToOpen));

	mainPanel->AddLabel("secondslabel", Vector2i(0, labelExtraY), labelCharHeight, "Time before lock:");
	secondsTextbox = mainPanel->AddBasicTextBox("secondsbox", Vector2i(0, 0), 50, 5, "");
	secondsTextbox->SetToolTip("Set number of seconds before the gate closes");
	secondsTextbox->SetString(to_string(seconds));

	deleteGateButton = mainPanel->AddButton("delete", Vector2i(30, 0), Vector2f(200, 28 + 4), "delete");
	deleteGateButton->SetToolTip("Delete selected gate (X / Delete)");

	//OKGateButton = mainPanel->AddButton("ok", Vector2i(0, 0), Vector2f(200, 28 + 4), "OK");
	//OKGateButton->SetToolTip("Finish editing a selected gate (Enter)");

	popupPanelPos = Vector2i(currGateTypeRects[0]->pos.x, currGateTypeRects[0]->pos.y + 100 + 10);

	

	//CreateShardTypePanel();
	ChooseShardType(shardGateTypeRects->at(0));

	CreateBossGateTypePanel();
	ChooseBossGateType(bossGateTypeRects[0]);

	CreatePickupGateTypePanel();
	ChoosePickupGateType(pickupGateTypeRects[0]);
}

CreateGatesModeUI::~CreateGatesModeUI()
{
	delete mainPanel;
	//delete shardGateTypePanel;
	delete pickupGateTypePanel;
	delete bossGateTypePanel;
	delete origModifyGate;
}

void CreateGatesModeUI::SetShard(int world, int localIndex)
{
	int ind = world * (edit->shardNumX * edit->shardNumY) + localIndex;
	ChooseShardType(shardGateTypeRects->at(ind));
}

void CreateGatesModeUI::Draw(sf::RenderTarget *target)
{
	if (modifyGate != NULL)
	{
		target->draw(modifyGateRect);
	}
}

void CreateGatesModeUI::SetEditGate(GateInfo *gi)
{
	CompleteEditingGate();
	modifyGate = gi;
	//sf::IntRect aabb = gi->GetAABB();
	int extra = 10;

	Vector2f diff = Vector2f(modifyGate->point1->pos - modifyGate->point0->pos);
	Vector2f center = Vector2f(modifyGate->point1->pos + modifyGate->point0->pos) / 2.f;
	float height = length(diff);
	modifyGateRect.setSize(Vector2f(GateInfo::selectWidth, height));
	modifyGateRect.setOrigin(modifyGateRect.getLocalBounds().width / 2,
		modifyGateRect.getLocalBounds().height / 2);
	float angle = GetVectorAngleCW(normalize(diff));
	angle = angle * 180 / PI + 90;
	modifyGateRect.setRotation(angle);
	modifyGateRect.setPosition(center);
	
	*origModifyGate = *gi;
	SetFromGateInfo(gi);
}

void CreateGatesModeUI::SetFromGateInfo(GateInfo *gi)
{
	gateCategoryDropdown->SetSelectedIndex(gi->category);

	//switch (gi->category)
	//{
	//case Gate::ALLKEY:
	//	gateCategoryDropdown->SetSelectedIndex(0);
	//	break;
	//case Gate::NUMBER_KEY:
	//	gateCategoryDropdown->SetSelectedIndex(1);
	//	//set key num text based on gate params
	//	break;
	//case Gate::SHARD:
	//	gateCategoryDropdown->SetSelectedIndex(2);
	//	SetShard(gi->shardWorld, gi->shardIndex);
	//	break;
	//case Gate::BOSS:
	//	gateCategoryDropdown->SetSelectedIndex(3);
	//	break;
	//case Gate::SECRET:
	//	gateCategoryDropdown->SetSelectedIndex(4);
	//	break;
	//case Gate::BLACK:
	//	gateCategoryDropdown->SetSelectedIndex(6);
	//	break;
	//}

	UpdateCategoryDropdownType();

	numToOpen = gi->numToOpen;
	seconds = gi->seconds;
	numToOpenTextbox->SetString(to_string(numToOpen));
	secondsTextbox->SetString(to_string(seconds));
}

void CreateGatesModeUI::SetGateInfo(GateInfo *gi)
{
	int gateCat = GetGateCategory();
	gi->category = gateCat;
	gi->variation = currVariation[gateCat];

	if (gateCat == Gate::SHARD)
	{
		gi->SetShard(currShardWorld, currShardLocalIndex);
	}

	gi->SetNumToOpen(numToOpen);
	gi->SetTime(seconds);
	gi->UpdateLine();
}

void CreateGatesModeUI::CreateBossGateTypePanel()
{
	bossGateTypePanel = new Panel("bosstype", 600, 600, this, true);
	Color c(100, 100, 100);
	c.a = 180;
	bossGateTypePanel->SetColor(c);
	bossGateTypePanel->SetPosition(popupPanelPos);

	int numBosses = 7;

	bossGateTypePanel->ReserveImageRects(numBosses);
	bossGateTypeRects.resize(numBosses);
	for (int i = 0; i < numBosses; ++i)
	{
		bossGateTypeRects[i] =
			bossGateTypePanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_GATEBOSSLIBRARY,
				Vector2f(0, i * gateGridSize),
				ts_bossGateTypes, i, gateGridSize);
		bossGateTypeRects[i]->Init();
		bossGateTypeRects[i]->SetShown(true);
	}
}

void CreateGatesModeUI::CreatePickupGateTypePanel()
{
	pickupGateTypePanel = new Panel("pickuptype", 600, 600, this, true);
	Color c(100, 100, 100);
	c.a = 180;

	pickupGateTypePanel->SetColor(c);
	pickupGateTypePanel->SetPosition(popupPanelPos);

	int numPickups = 8;

	pickupGateTypePanel->ReserveImageRects(numPickups);
	pickupGateTypeRects.resize(numPickups);
	for (int i = 0; i < numPickups; ++i)
	{
		pickupGateTypeRects[i] =
			pickupGateTypePanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_GATEPICKUPLIBRARY,
				Vector2f(0, i * gateGridSize),
				ts_pickupGateTypes, i, gateGridSize);
		pickupGateTypeRects[i]->Init();
		pickupGateTypeRects[i]->SetShown(true);
	}
}

void CreateGatesModeUI::ExpandShardLibrary()
{
	if (shardGateTypePanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(shardGateTypePanel);
	}
	else
	{
		shardGateTypePanel->SetPosition(popupPanelPos);
		shardGateTypePanel->handler = this;
		edit->AddActivePanel(shardGateTypePanel);
	}
	
}

void CreateGatesModeUI::ExpandBossLibrary()
{
	if (bossGateTypePanel == edit->focusedPanel)
	{
		edit->RemoveActivePanel(bossGateTypePanel);
	}
	else
	{
		edit->AddActivePanel(bossGateTypePanel);
	}
}

void CreateGatesModeUI::ExpandPickupLibrary()
{
	if (bossGateTypePanel == edit->focusedPanel)
	{
		edit->AddActivePanel(pickupGateTypePanel);
	}
	else
	{
		edit->RemoveActivePanel(pickupGateTypePanel);
	}
	
}

void CreateGatesModeUI::ChooseShardType(ImageChooseRect *icRect)
{
	currGateTypeRects[Gate::SHARD]->SetImage(icRect->ts, icRect->spr.getTextureRect());

	int x = icRect->pos.x / gateGridSize;
	int y = icRect->pos.y / ( gateGridSize * 2 );

	int world = y / edit->shardNumY;
	int localIndex = (y % edit->shardNumY) * edit->shardNumX + x;

	currShardWorld = world;
	currShardLocalIndex = localIndex;

	if (modifyGate != NULL)
	{
		modifyGate->SetShard(currShardWorld, currShardLocalIndex);
	}


	edit->RemoveActivePanel(shardGateTypePanel);
}

void CreateGatesModeUI::ChooseBossGateType(ImageChooseRect *icRect)
{
	currGateTypeRects[Gate::BOSS]->SetImage(icRect->ts, icRect->spr.getTextureRect());

	int x = icRect->pos.x / gateGridSize;
	int y = icRect->pos.y / gateGridSize;

	currVariation[Gate::BOSS] = y;

	edit->RemoveActivePanel(bossGateTypePanel);
}

void CreateGatesModeUI::ChoosePickupGateType(ImageChooseRect *icRect)
{
	currGateTypeRects[Gate::PICKUP]->SetImage(icRect->ts, icRect->spr.getTextureRect());

	int x = icRect->pos.x / gateGridSize;
	int y = icRect->pos.y / gateGridSize;
	
	currVariation[Gate::PICKUP] = y;

	edit->RemoveActivePanel(pickupGateTypePanel);
}




int CreateGatesModeUI::GetGateCategory()
{
	return gateCategoryDropdown->selectedIndex;
}

void CreateGatesModeUI::SetShown(bool s)
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

void CreateGatesModeUI::UpdateCategoryDropdownType()
{
	for (int i = 0; i < currGateTypeRects.size(); ++i)
	{
		currGateTypeRects[i]->SetShown(false);
	}

	currGateTypeRects[gateCategoryDropdown->selectedIndex]->SetShown(true);
}

void CreateGatesModeUI::ExpandLibrary()
{
	int cat = GetGateCategory();
	switch (cat)
	{
	case Gate::SHARD:
		ExpandShardLibrary();
		break;
	case Gate::BOSS:
		ExpandBossLibrary();
		break;
	case Gate::PICKUP:
		ExpandPickupLibrary();
		break;
	}
}

void CreateGatesModeUI::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::E_LEFTCLICKED ||
		eventType == ChooseRect::E_LEFTRELEASED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_GATESEARCH )
			{
				if (eventType == ChooseRect::E_LEFTCLICKED)
				{
					ExpandLibrary();
				}
			}
			else if (icRect->rectIdentity == ChooseRect::I_SHARDLIBRARY)
			{
				ChooseShardType(icRect);
				if (modifyGate != NULL)
				{
					SetGateInfo(modifyGate);
				}
			}
			else if (icRect->rectIdentity == ChooseRect::I_GATEBOSSLIBRARY)
			{
				ChooseBossGateType(icRect);
				if (modifyGate != NULL)
				{
					SetGateInfo(modifyGate);
				}
			}
			else if (icRect->rectIdentity == ChooseRect::I_GATEPICKUPLIBRARY)
			{
				ChoosePickupGateType(icRect);
				if (modifyGate != NULL)
				{
					SetGateInfo(modifyGate);
				}
			}
		}
	}
}

void CreateGatesModeUI::CompleteEditingGate()
{
	if (modifyGate != NULL )
	{
		if( !modifyGate->HasSameInfo( origModifyGate ) )
		{
			Action * action = new ModifyGateAction(modifyGate, origModifyGate);
			action->performed = true;
			edit->AddDoneAction(action);
		}
		//modifyGate = NULL;
	}
}

void CreateGatesModeUI::ButtonCallback(Button *b, const std::string & e)
{
	//if (b == OKGateButton)
	//{
	//	CompleteEditingGate();
	//	//modifyGate = NULL;

	//	
	//	//add action here
	//}
	if (b == deleteGateButton)
	{
		if (modifyGate != NULL)
		{
			CompleteEditingGate();
			Action * action = new DeleteGateAction(modifyGate, edit->mapStartBrush);
			action->Perform();
			edit->AddDoneAction(action);
			modifyGate = NULL;
		}
		else
		{
			edit->gatePoints = 0;
		}
	}
	/*if (b == completeButton)
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
	}*/
}

void CreateGatesModeUI::TextBoxCallback(TextBox *tb, const std::string & e)
{
	if (tb == numToOpenTextbox)
	{
		string str = tb->GetString();
		stringstream ss;
		ss << str;
		int num;
		ss >> num;
		if (!ss.fail())
		{
			numToOpen = num;
			if (modifyGate != NULL)
			{
				modifyGate->SetNumToOpen(numToOpen);
			}
		}
	}
	else if (tb == secondsTextbox)
	{
		string str = tb->GetString();
		stringstream ss;
		ss << str;
		int num;
		ss >> num;
		if (!ss.fail())
		{
			seconds = num;
			if (modifyGate != NULL)
			{
				modifyGate->SetTime(seconds);
			}
		}
	}
}

void CreateGatesModeUI::GridSelectorCallback(GridSelector *gs, const std::string & e)
{

}

void CreateGatesModeUI::CheckBoxCallback(CheckBox *cb, const std::string & e)
{

}

void CreateGatesModeUI::SliderCallback(Slider *slider)
{

}



void CreateGatesModeUI::DropdownCallback(Dropdown *dropdown, const std::string & e)
{
	if (dropdown == gateCategoryDropdown)
	{
		UpdateCategoryDropdownType();
		if (modifyGate != NULL)
		{
			SetGateInfo(modifyGate);
		}
	}
}

void CreateGatesModeUI::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup" )
	{
		edit->RemoveActivePanel(p);
	}
}