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

	topbarCont = new ChooseRectContainer(Vector2i(0, 20), Vector2f(1920, 120));

	int imageCounter = 0;
	for (auto it = edit->decorTileIndexMap.begin(); it != edit->decorTileIndexMap.end(); ++it)
	{
		imageCounter += (*it).second.size();
	}

	allImageRects.reserve(imageCounter);

	int numEnemyWorlds = 9;

	//allEnemyQuads = new Vertex[enemyCounter * 4];
	//hotbarQuads = new Vertex[totalHotbarCount * 4];
	//worldSelectQuads = new Vertex[numEnemyWorlds * 4];

	int numExtraRects = 1; //search library
	numAllQuads = (imageCounter + totalHotbarCount + numEnemyWorlds + numExtraRects);
	allQuads = new Vertex[numAllQuads * 4];
	Vertex *allImageQuads = allQuads;
	Vertex *hotbarQuads = allQuads + imageCounter * 4;
	Vertex *worldSelectQuads = hotbarQuads + totalHotbarCount * 4;
	Vertex *extraQuads = worldSelectQuads + numEnemyWorlds * 4;

	Tileset *ts_worldChoosers = edit->GetSizedTileset("worldselector_64x64.png");

	librarySearchRect = new ImageChooseRect(ChooseRect::I_SEARCHDECORLIBRARY, extraQuads, topbarCont,
		Vector2f(10, 10), ts_worldChoosers, 8);
	librarySearchRect->SetShown(true);
	librarySearchRect->Init();

	int worldSize = 100;
	int worldSpacing = 30;
	int totalWorldSize = worldSize * numEnemyWorlds + worldSpacing * (numEnemyWorlds - 1);

	int extraWorldSpacing = (1920 - totalWorldSize) / 2;

	libCont = new ChooseRectContainer(Vector2i(0, 140), Vector2f(totalWorldSize + 20, 600));

	{
		int i = 0;
		for (auto it = edit->decorTileIndexMap.begin(); it != edit->decorTileIndexMap.end(); ++it)
		{
			for (auto indexIt = (*it).second.begin(); indexIt != (*it).second.end(); ++indexIt)
			{
				allImageRects.push_back(ImageChooseRect(ChooseRect::I_DECORLIBRARY, allImageQuads + i * 4, libCont,
					Vector2f(0, 0), edit->decorTSMap[(*it).first], (*indexIt)));
				allImageRects.back().decorName = (*it).first;
				++i;
			}
		}
	}

	for (int i = 0; i < totalHotbarCount; ++i)
	{
		hotbarImages.push_back(ImageChooseRect(ChooseRect::I_DECORHOTBAR, hotbarQuads + i * 4, topbarCont,
			Vector2f(extraHotbarSpacing + 10 + i * (hotbarRectSize + hotbarSpacing), 10),
			NULL, 0));
		hotbarImages[i].SetShown(false);
		hotbarImages[i].Init();
	}



	worldSelectRects.reserve(numEnemyWorlds);

	for (int i = 0; i < numEnemyWorlds; ++i)
	{
		worldSelectRects.push_back(ImageChooseRect(ChooseRect::I_WORLDCHOOSER, worldSelectQuads + i * 4, libCont,
			Vector2f(i * (worldSize + worldSpacing) + 10, 10), ts_worldChoosers, i));
		worldSelectRects[i].SetShown(false);
		worldSelectRects[i].Init();
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
				libraryImagesVec[w][counter] = &allImageRects[i];
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
	delete topbarCont;
	delete libCont;
	//delete topbarPanel;
	//delete libraryPanel;

	delete[] allQuads;

	delete librarySearchRect;
	//delete [] allEnemyQuads;
	//delete[] worldSelectQuads;
	//delete[] hotbarQuads;
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
	if (!show)
	{
		topbarCont->ResetMouse();
		libCont->ResetMouse();
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

void CreateDecorModeUI::UpdateSprites(int sprUpdateFrames)
{
	for (int i = 0; i < activeHotbarSize; ++i)
	{
		/*if (activeLibraryWorld != hotbarImages[i].actorType->info.world)
		{
			hotbarEnemies[i].UpdateSprite(sprUpdateFrames);
		}*/
	}

	if (activeLibraryWorld >= 0)
	{
		for (auto it = libraryImagesVec[activeLibraryWorld].begin();
			it != libraryImagesVec[activeLibraryWorld].end(); ++it)
		{
			if ((*it) != NULL)
			{
				(*it)->UpdateSprite(sprUpdateFrames);
			}
		}
	}
}

void CreateDecorModeUI::Update(bool mouseDownL, bool mouseDownR, sf::Vector2i &mPos)
{
	if (!show)
	{
		return;
	}


	topbarCont->Update(mouseDownL, mouseDownR, mPos);
	libCont->Update(mouseDownL, mouseDownR, mPos);

	librarySearchRect->Update();

	for (int i = 0; i < hotbarImages.size(); ++i)
	{
		hotbarImages[i].Update();
	}

	if (showLibrary)
	{
		for (int i = 0; i < 9; ++i)
		{
			worldSelectRects[i].Update();
		}

		if (activeLibraryWorld >= 0)
		{
			for (auto it = libraryImagesVec[activeLibraryWorld].begin();
				it != libraryImagesVec[activeLibraryWorld].end(); ++it)
			{
				if ((*it) != NULL)
				{
					(*it)->Update();
				}
			}
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
			for (int i = 0; i < 9; ++i)
			{
				worldSelectRects[i].SetShown(true);
			}
		}
		else
		{
			for (int i = 0; i < 9; ++i)
			{
				worldSelectRects[i].SetShown(false);
			}
		}

	}
}

void CreateDecorModeUI::FlipLibraryShown()
{
	SetLibraryShown(!showLibrary);
}

void CreateDecorModeUI::Draw(sf::RenderTarget *target)
{
	if (!show)
	{
		return;
	}
	sf::View oldView = target->getView();
	target->setView(edit->uiView);


	//topbarPanel->Draw(target);
	//libraryPanel->Draw(target);

	topbarCont->Draw(target);
	if (showLibrary)
	{
		libCont->Draw(target);
	}

	target->draw(allQuads, numAllQuads * 4, sf::Quads);

	librarySearchRect->Draw(target);
	//topbarCont->Draw(target);
	//target->draw(hotbarQuads, activeHotbarSize * 4, sf::Quads);
	for (int i = 0; i < activeHotbarSize; ++i)
	{
		hotbarImages[i].Draw(target);
	}

	//target->draw(allEnemyQuads, allEnemyRects.size() * 4, sf::Quads);
	//target->draw(worldSelectQuads, 9 * 4, sf::Quads);
	if (showLibrary)
	{
		for (int i = 0; i < 9; ++i)
		{
			worldSelectRects[i].Draw(target);
		}


		if (activeLibraryWorld >= 0)
		{
			for (auto it = libraryImagesVec[activeLibraryWorld].begin();
				it != libraryImagesVec[activeLibraryWorld].end(); ++it)
			{
				if ((*it) != NULL)
				{
					(*it)->Draw(target);
				}
			}
		}
	}

	target->setView(oldView);
}