#include "TransferLoadingScreen.h"
#include "ShardAndLogDisplay.h"
#include "MainMenu.h"
#include "AdventureManager.h"

using namespace sf;
using namespace std;

TransferLoadingScreen::TransferLoadingScreen()
{
	MainMenu *mm = MainMenu::GetInstance();

	ts_mapPreview = NULL;

	shardAndLogDisplay = new ShardAndLogDisplay(this);
	shardAndLogDisplay->SetTopLeft(Vector2f(779, 902));

	levelNameText.setFont(mm->arial);
	levelNameText.setCharacterSize(60);
	levelNameText.setFillColor(Color::White);
}

TransferLoadingScreen::~TransferLoadingScreen()
{
	delete shardAndLogDisplay;
}

void TransferLoadingScreen::SetLevel(Level *lev)
{
	MainMenu *mm = MainMenu::GetInstance();
	level = lev;

	shardAndLogDisplay->SetLevel(level);

	levelNameText.setString(mm->adventureManager->adventureFile.GetMap(level->index).name);

	auto lb = levelNameText.getLocalBounds();

	levelNameText.setOrigin(lb.left + lb.width / 2, lb.height / 2);
	levelNameText.setPosition(960, 800);

	UpdateMapPreview();
}

void TransferLoadingScreen::DrawLevelInfo(sf::RenderTarget *target)
{
	target->draw(levelNameText);
	shardAndLogDisplay->Draw(target);
}


void TransferLoadingScreen::DestroyMapPreview()
{
	if (ts_mapPreview != NULL)
	{
		DestroyTileset(ts_mapPreview);
		ts_mapPreview = NULL;
	}
}


void TransferLoadingScreen::UpdateMapPreview()
{
	DestroyMapPreview();

	bool allSecretsCollected = false;

	MainMenu *mm = MainMenu::GetInstance();

	int totalShards = 0;
	int numShardsCaptured = 0;
	int totalLogs = 0;
	int numLogsCaptured = 0;

	if (mm->adventureManager != NULL)
	{
		AdventureMapHeaderInfo &amhi =
			mm->adventureManager->adventureFile.GetMapHeaderInfo(level->index);
		totalShards = amhi.shardInfoVec.size();

		SaveFile *saveFile = mm->adventureManager->currSaveFile;
		for (int j = 0; j < totalShards; ++j)
		{
			if (saveFile->IsShardCaptured(amhi.shardInfoVec[j].GetTrueIndex()))
			{
				++numShardsCaptured;
			}
		}

		totalLogs = amhi.logInfoVec.size();
		for (int j = 0; j < totalLogs; ++j)
		{
			if (saveFile->HasLog(amhi.logInfoVec[j].GetTrueIndex()))
			{
				++numLogsCaptured;
			}
		}
	}
	else
	{
		assert(0);
	}

	if (totalShards == numShardsCaptured && totalLogs == numLogsCaptured)
	{
		allSecretsCollected = true;
	}

	//mm->adventureManager->adventureFile.GetAdventureSector(sec).maps[GetSelectedIndex()].GetFilePath();
	string fPath = mm->adventureManager->adventureFile.GetMap(level->index).GetFilePath();
	string previewPath;
	if (allSecretsCollected)
	{
		previewPath = "Maps\\" + fPath + ".png";
	}
	else
	{
		previewPath = "Maps\\" + fPath + "_basic.png";
	}

	int mapPreviewHeight = 180;

	ts_mapPreview = GetTileset(previewPath, 912, 492);
	mapPreviewSpr.setTexture(*ts_mapPreview->texture);
	//mapPreviewSpr.setScale(.75, .75);
	mapPreviewSpr.setScale(.6, .6);
	mapPreviewSpr.setOrigin(mapPreviewSpr.getLocalBounds().width / 2, mapPreviewSpr.getLocalBounds().height / 2);
	mapPreviewSpr.setPosition(960, mapPreviewHeight);
}

void TransferLoadingScreen::DrawMapPreview(sf::RenderTarget *target)
{
	if (ts_mapPreview != NULL)
	{
		target->draw(mapPreviewSpr);
	}
}