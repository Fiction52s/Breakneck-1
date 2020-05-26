#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"
#include "EditorGraph.h"

using namespace std;
using namespace sf;

CreateGatesModeUI::CreateGatesModeUI()
{
	edit = EditSession::GetSession();
	mainPanel = new Panel("creategates", 1920, 200, this, false);
	//gateTypePanel = new Panel("gatetype", 600, 600, this, true);

	
	pickupGateTypePanel = new Panel("pickuptype", 600, 600, this, true);
	bossGateTypePanel = new Panel("bosstype", 600, 600, this, true);
	Color c(100, 100, 100);
	c.a = 180;
	
	pickupGateTypePanel->SetColor(c);
	bossGateTypePanel->SetColor(c);

	numKeysTextbox = mainPanel->AddTextBox("numkeys", Vector2i(100, 10), 50, 5, "");
	completeGateButton = mainPanel->AddButton("complete", Vector2i(200, 10), Vector2f(80, 80), "complete");
	deleteGateButton = mainPanel->AddButton("delete", Vector2i(300, 10), Vector2f(80, 80), "delete");

	std::vector<string> gateCatOptions = { "Key", "Shard", "Boss", "Secret", "Pickup", "Black" };
	gateCategoryDropdown = mainPanel->AddDropdown(
		"catdrop", Vector2i(500, 10), Vector2i(200, 28), gateCatOptions, 0);

	gateGridSize = 64;

	ts_gateCategories = edit->GetSizedTileset("Editor/gatecategories_128x128.png");

	int numGateCategories = gateCatOptions.size();//EditSession::TERRAINLAYER_Count;
	currGateTypeRects.resize(numGateCategories);
	mainPanel->ReserveImageRects(numGateCategories);
	Vector2f currGateTypeRectPos = Vector2f(gateCategoryDropdown->pos)
		+ Vector2f(gateCategoryDropdown->size.x + 20, 0);
	for (int i = 0; i < numGateCategories; ++i)
	{
		currGateTypeRects[i] = mainPanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_GATESEARCH,
			currGateTypeRectPos, ts_gateCategories, i, 100);
		currGateTypeRects[i]->Init();
	}

	currGateTypeRects[0]->SetShown(true);

	CreateShardTypePanel();
	ChooseShardType(shardGateTypeRects[0]);
	
	pickupGateTypePanel->SetPosition(Vector2i(currGateTypeRectPos.x, currGateTypeRectPos.y + 100 + 10));
	bossGateTypePanel->SetPosition(Vector2i(currGateTypeRectPos.x, currGateTypeRectPos.y + 100 + 10));
}

void CreateGatesModeUI::CreateShardTypePanel()
{
	int xSize = 11;
	int ySize = 2;

	shardGridSize = 64;

	Vector2f currGateTypeRectPos = Vector2f(gateCategoryDropdown->pos)
		+ Vector2f(gateCategoryDropdown->size.x + 20, 0);
	
	//GridSelector *gs = p->AddGridSelector("shardselector", pos, xSize, ySize * 7, 64, 64, true, true);
	//Sprite spr;

	shardGateTypePanel = new Panel("shardtype", 600, 600, this, true);
	shardGateTypePanel->SetPosition(Vector2i(currGateTypeRectPos.x, currGateTypeRectPos.y + 100 + 10));
	Color c(100, 100, 100);
	c.a = 180;
	shardGateTypePanel->SetColor(c);

	int numWorlds = 7;
	for (int i = 0; i < numWorlds; ++i)
	{
		ts_shards[i] = edit->GetSizedTileset("Shard/shards_w" + to_string(i+1) + "_48x48.png");
	}

	/*ts_shards[0] = edit->GetSizedTileset("Shard/shards_w1_48x48.png");
	ts_shards[1] = edit->GetSizedTileset("Shard/shards_w2_48x48.png");
	ts_shards[2] = edit->GetSizedTileset("Shard/shards_w2_48x48.png");
	ts_shards[3] = edit->GetSizedTileset("Shard/shards_w2_48x48.png");
	ts_shards[4] = edit->GetSizedTileset("Shard/shards_w2_48x48.png");
	ts_shards[5] = edit->GetSizedTileset("Shard/shards_w2_48x48.png");
	ts_shards[6] = edit->GetSizedTileset("Shard/shards_w2_48x48.png");*/

	int totalShards = xSize * ySize * 7;

	Tileset *ts_currShards;
	int sInd = 0;

	shardGateTypePanel->ReserveImageRects(totalShards);
	shardGateTypeRects.resize(totalShards);

	for (int w = 0; w < numWorlds; ++w)
	{
		ts_currShards = ts_shards[w];
		if (ts_currShards == NULL)
			continue;
		//spr.setTexture(*ts_currShards->texture);
		for (int y = 0; y < ySize; ++y)
		{
			for (int x = 0; x < xSize; ++x)
			{
				//ts_currShards->GetSubRect(sInd);
				sInd = y * xSize + x;
				//spr.setTextureRect(ts_currShards->GetSubRect(sInd));
				int shardT = (sInd + (xSize * ySize) * w);
				if (shardT >= SHARD_Count)
				{
					shardGateTypeRects[shardT] = NULL;
					//gs->Set(x, y + ySize * w, spr, "---"); //need a way to set the names later
				}
				else
				{
					shardGateTypeRects[shardT] =
						shardGateTypePanel->AddImageRect(ChooseRect::ChooseRectIdentity::I_SHARDLIBRARY,
						Vector2f( x * shardGridSize, y * shardGridSize + w * 2 * shardGridSize ), 
							ts_currShards, sInd, shardGridSize);
					shardGateTypeRects[shardT]->Init();
					shardGateTypeRects[shardT]->SetShown(true);
					//shardGateTypeRects[sInd]->set

					//Shard::GetShardString((ShardType)shardT) for later when I add a label
				}

			}
		}
	}

	//ts_currShards = ts_shards[w];
	//spr.setTexture(*ts_currShards->texture);
	//for (int y = 0; y < ySize; ++y)
	//{
	//	for (int x = 0; x < xSize; ++x)
	//	{
	//		sInd = y * xSize + x;
	//		spr.setTextureRect(ts_currShards->GetSubRect(sInd));
	//		int shardT = (sInd + (xSize * ySize) * w);
	//		if (shardT >= SHARD_Count)
	//		{
	//			gs->Set(x, y + ySize * w, spr, "---"); //need a way to set the names later
	//		}
	//		else
	//		{
	//			gs->Set(x, y + ySize * w, spr, Shard::GetShardString((ShardType)shardT));
	//		}

	//	}
	//}
}

void CreateGatesModeUI::ExpandShardLibrary()
{
	edit->AddActivePanel(shardGateTypePanel);
}

void CreateGatesModeUI::ChooseShardType(ImageChooseRect *icRect)
{
	//currGateTypeRects[layerIndex]->SetImage(icRect->ts, icRect->spr.getTextureRect());
	currGateTypeRects[1]->SetImage(icRect->ts, icRect->spr.getTextureRect());

	int x = icRect->pos.x / shardGridSize;
	int y = icRect->pos.y / shardGridSize;

	//edit->currTerrainWorld[layerIndex] = world;
	//edit->currTerrainVar[layerIndex] = variation;

	edit->RemoveActivePanel(shardGateTypePanel);
	//edit->justCompletedPolyWithClick = true;
}

CreateGatesModeUI::~CreateGatesModeUI()
{
	delete mainPanel;
	delete shardGateTypePanel;
	delete pickupGateTypePanel;
	delete bossGateTypePanel;
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

void CreateGatesModeUI::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::E_LEFTCLICKED ||
		eventType == ChooseRect::E_LEFTRELEASED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_GATESEARCH && GetGateCategory() == 1)
			{
				ExpandShardLibrary();
			}
			else if (icRect->rectIdentity == ChooseRect::I_SHARDLIBRARY)
			{
				ChooseShardType(icRect);
			}
		}
	}
}

void CreateGatesModeUI::ButtonCallback(Button *b, const std::string & e)
{
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
	/*if (tb == gridSizeTextbox)
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
	}*/
}

void CreateGatesModeUI::GridSelectorCallback(GridSelector *gs, const std::string & e)
{

}

void CreateGatesModeUI::CheckBoxCallback(CheckBox *cb, const std::string & e)
{

}

void CreateGatesModeUI::SliderCallback(Slider *slider, const std::string & e)
{

}

void CreateGatesModeUI::DropdownCallback(Dropdown *dropdown, const std::string & e)
{
	if (dropdown == gateCategoryDropdown)
	{
		int selectedIndex = dropdown->selectedIndex;
		for (int i = 0; i < currGateTypeRects.size(); ++i)
		{
			currGateTypeRects[i]->SetShown(false);
		}

		currGateTypeRects[selectedIndex]->SetShown(true);
	}
}

void CreateGatesModeUI::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup" )
	{
		edit->RemoveActivePanel(p);
	}
}