#include "ShardAndLogDisplay.h"
#include "MainMenu.h"
#include "SaveFile.h"
#include "AdventureManager.h"

using namespace sf;
using namespace std;

ShardAndLogDisplay::ShardAndLogDisplay(TilesetManager *tm)
{
	ts_statIcons = tm->GetSizedTileset("HUD/score_icons_128x96.png");

	MainMenu *mm = MainMenu::GetInstance();

	ts_statIcons->SetSpriteTexture(shardIconSpr);
	ts_statIcons->SetSubRect(shardIconSpr, 2);
	ts_statIcons->SetSpriteTexture(logIconSpr);
	ts_statIcons->SetSubRect(logIconSpr, 3);

	shardText.setFont(mm->arial);
	shardText.setCharacterSize(40);
	shardText.setFillColor(Color::White);

	logText.setFont(mm->arial);
	logText.setCharacterSize(40);
	logText.setFillColor(Color::White);
}

void ShardAndLogDisplay::SetTopLeft(sf::Vector2f pos)
{
	topLeft = pos;
	Vector2f levelStatsTopLeft = pos;//Vector2f(960, 750) + Vector2f(-181, 152); //these seemingly random numbers just center it and put it at the correct height
	Vector2f logDiff(180, 0);

	shardIconSpr.setPosition(levelStatsTopLeft);
	shardText.setPosition(shardIconSpr.getPosition() + Vector2f(96 + 10, 20));

	logIconSpr.setPosition(levelStatsTopLeft + logDiff);
	logText.setPosition(logIconSpr.getPosition() + Vector2f(96 + 10, 20));
}

void ShardAndLogDisplay::SetLevel(Level *lev)
{
	int totalShards = 0;
	int numShardsCaptured = 0;
	int totalLogs = 0;
	int numLogsCaptured = 0;

	MainMenu *mm = MainMenu::GetInstance();

	AdventureMapHeaderInfo &amhi =
		mm->adventureManager->adventureFile.GetMapHeaderInfo(lev->index);
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

	stringstream ss;

	ss.str("");
	ss.clear();

	ss << numShardsCaptured << "/" << totalShards;

	shardText.setString(ss.str());

	ss.str("");
	ss.clear();

	ss << numLogsCaptured << "/" << totalLogs;

	logText.setString(ss.str());

	SetTopLeft(topLeft);
}

void ShardAndLogDisplay::Draw(sf::RenderTarget *target)
{
	target->draw(shardIconSpr);
	target->draw(shardText);
	target->draw(logIconSpr);
	target->draw(logText);
}