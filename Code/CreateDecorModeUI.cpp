#include "GUI.h"
#include "EditorActors.h"
#include "VectorMath.h"
#include "ActorParamsBase.h"
#include "Enemy.h"
#include "EditSession.h"

using namespace std;
using namespace sf;


CreateDecorModeUI::CreateDecorModeUI()
{
	activeHotbarSize = 0;
	hotbarImages.reserve(8);
	libraryImagesVec.resize(9);
	for (int i = 0; i < 9; ++i)
	{
		libraryImagesVec[i].resize(20); //just a random number
	}

	edit = EditSession::GetSession();

	int totalHotbarCount = 12;
	int hotbarRectSize = 100;
	int hotbarSpacing = 20;
	int totalHotbarSize = hotbarRectSize * totalHotbarCount + hotbarSpacing * (totalHotbarCount - 1);
	int extraHotbarSpacing = (1920 - totalHotbarSize) / 2;

	topbarPanel = new Panel("topbarpanel", 1920, 120, edit, false);
	topbarPanel->SetPosition(Vector2i(0, 20));

	Color panelColor;
	panelColor = Color::Cyan;
	panelColor.a = 80;

	topbarPanel->SetColor(panelColor);

	int imageCounter = 0;
	for (auto it = edit->decorTileIndexMap.begin(); it != edit->decorTileIndexMap.end(); ++it)
	{
		imageCounter += (*it).second.size();
	}

	allImageRects.reserve(imageCounter);

	int numEnemyWorlds = 9;

	int numExtraRects = 1; //search library
	//numAllQuads = (imageCounter + totalHotbarCount + numEnemyWorlds + numExtraRects);

	topbarPanel->ReserveImageRects(totalHotbarCount + numExtraRects );

	Tileset *ts_worldChoosers = edit->GetSizedTileset("worldselector_64x64.png");

	librarySearchRect = topbarPanel->AddImageRect(ChooseRect::I_SEARCHDECORLIBRARY,
		Vector2f(10 + 100, 10), ts_worldChoosers, 8);
	librarySearchRect->SetShown(true);
	librarySearchRect->Init();

	int worldSize = 100;
	int worldSpacing = 30;
	int totalWorldSize = worldSize * numEnemyWorlds + worldSpacing * (numEnemyWorlds - 1);

	int extraWorldSpacing = (1920 - totalWorldSize) / 2;

	libPanel = new Panel("libpanel", totalWorldSize + 20, 600, edit, false);
	libPanel->SetPosition(Vector2i(100, 140));
	libPanel->SetColor(panelColor);

	libPanel->ReserveImageRects(imageCounter + numEnemyWorlds);
	{
		int i = 0;
		for (auto it = edit->decorTileIndexMap.begin(); it != edit->decorTileIndexMap.end(); ++it)
		{
			for (auto indexIt = (*it).second.begin(); indexIt != (*it).second.end(); ++indexIt)
			{
				allImageRects.push_back(libPanel->AddImageRect(ChooseRect::I_DECORLIBRARY,
					Vector2f(0, 0), edit->decorTSMap[(*it).first], (*indexIt)));
				allImageRects.back()->decorName = (*it).first;

				/*allImageRects.push_back(ImageChooseRect(ChooseRect::I_DECORLIBRARY, allImageQuads + i * 4,
					Vector2f(0, 0), edit->decorTSMap[(*it).first], (*indexIt), libPanel));
				allImageRects.back().decorName = (*it).first;
				++i;*/
			}
		}
	}

	for (int i = 0; i < totalHotbarCount; ++i)
	{
		hotbarImages.push_back(topbarPanel->AddImageRect(ChooseRect::I_DECORHOTBAR,
			Vector2f(extraHotbarSpacing + 10 + i * (hotbarRectSize + hotbarSpacing), 10),
			NULL, 0));
		hotbarImages[i]->SetShown(false);
		hotbarImages[i]->Init();

		/*hotbarImages.push_back(ImageChooseRect(ChooseRect::I_DECORHOTBAR, hotbarQuads + i * 4,
			Vector2f(extraHotbarSpacing + 10 + i * (hotbarRectSize + hotbarSpacing), 10),
			NULL, 0, topbarPanel));*/
		
	}



	worldSelectRects.reserve(numEnemyWorlds);

	for (int i = 0; i < numEnemyWorlds; ++i)
	{
		worldSelectRects.push_back(libPanel->AddImageRect(ChooseRect::I_WORLDCHOOSER,
			Vector2f(i * (worldSize + worldSpacing) + 10, 10), ts_worldChoosers, i));
		worldSelectRects[i]->SetShown(false);
		worldSelectRects[i]->Init();
		/*worldSelectRects.push_back(ImageChooseRect(ChooseRect::I_WORLDCHOOSER, worldSelectQuads + i * 4,
			Vector2f(i * (worldSize + worldSpacing) + 10, 10), ts_worldChoosers, i, libPanel));*/
		
	}

	int counter;
	for (int w = 0; w < 1; ++w)
	{
		counter = 0;
		for (int i = 0; i < allImageRects.size(); ++i)
		{
			++counter;
			/*if (allImageRects[i].actorType->info.world == w)
			{
				
			}*/
		}
		libraryImagesVec[w].reserve(counter);
		//libraryEnemiesVec[0].push_back( )
	}

	ImageChooseRect *icRect;
	int row, col;
	int maxCol = 9;
	for (int w = 0; w < 1; ++w)
	{
		counter = 0;
		for (int i = 0; i < allImageRects.size(); ++i)
		{
			//if (allImageRects[i].actorType->info.world == w)
			{
				libraryImagesVec[w][counter] = allImageRects[i];
				icRect = libraryImagesVec[w][counter];
				col = counter % maxCol;
				row = counter / maxCol;
				icRect->SetPosition(Vector2f(10 + col * 120, 240 + row * 120));
				//ecRect->SetShown(true);
				icRect->Init();
				++counter;
			}
		}
	}

	activeLibraryWorld = -1;
	showLibrary = false;
	show = true;

	UpdateHotbarTypes();
}

CreateDecorModeUI::~CreateDecorModeUI()
{
	delete topbarPanel;
	delete libPanel;
}

void CreateDecorModeUI::UpdateHotbarTypes()
{
	/*int i = 0;
	for (auto it = edit->recentEnemies.begin(); it != edit->recentEnemies.end(); ++it)
	{
		hotbarImages[i].SetType((*it).first, (*it).second);
		hotbarImages[i].SetShown(true);
		++i;
	}
	activeHotbarSize = i;
	for (; i < EditSession::MAX_RECENT_ENEMIES; ++i)
	{
		hotbarEnemies[i].SetType(NULL, 0);
	}*/
}

void CreateDecorModeUI::SetShown(bool s)
{
	show = s;
	if (show)
	{
		edit->AddActivePanel(topbarPanel);
		if (showLibrary)
		{
			edit->AddActivePanel(libPanel);
		}
	}
	else
	{
		edit->RemoveActivePanel(topbarPanel);
		if (showLibrary)
		{
			edit->RemoveActivePanel(libPanel);
		}
	}
}

void CreateDecorModeUI::SetActiveLibraryWorld(int w)
{
	if (w == activeLibraryWorld)
		return;

	if (activeLibraryWorld >= 0)
	{
		for (auto it = libraryImagesVec[activeLibraryWorld].begin();
			it != libraryImagesVec[activeLibraryWorld].end(); ++it)
		{
			if ((*it) != NULL)
				(*it)->SetShown(false);
		}
	}

	activeLibraryWorld = w;

	if (activeLibraryWorld >= 0)
	{
		for (auto it = libraryImagesVec[activeLibraryWorld].begin();
			it != libraryImagesVec[activeLibraryWorld].end(); ++it)
		{
			if ((*it) != NULL)
				(*it)->SetShown(true);
		}
	}
}

void CreateDecorModeUI::SetLibraryShown(bool s)
{
	if (showLibrary != s)
	{
		showLibrary = s;
		SetActiveLibraryWorld(-1);

		if (showLibrary)
		{
			edit->AddActivePanel(libPanel);
			for (int i = 0; i < 9; ++i)
			{
				worldSelectRects[i]->SetShown(true);
			}
		}
		else
		{
			edit->RemoveActivePanel(libPanel);
			for (int i = 0; i < 9; ++i)
			{
				worldSelectRects[i]->SetShown(false);
			}
		}

	}
}

void CreateDecorModeUI::FlipLibraryShown()
{
	SetLibraryShown(!showLibrary);
}
