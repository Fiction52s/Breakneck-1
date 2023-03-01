#include "Leaderboard.h"
#include <iostream>
#include "MainMenu.h"

using namespace std;
using namespace sf;

LeaderboardEntryRow::LeaderboardEntryRow()
{
	MainMenu *mainMenu = MainMenu::GetInstance();

	int charHeight = LeaderboardDisplay::CHAR_HEIGHT;
	Color col = Color::Black;

	scoreText.setFont(mainMenu->arial);
	scoreText.setFillColor(col);
	scoreText.setCharacterSize(charHeight);

	rankText.setFont(mainMenu->arial);
	rankText.setFillColor(col);
	rankText.setCharacterSize(charHeight);

	index = -1;

	Clear();
}

void LeaderboardEntryRow::Init( int p_index, Panel *p)
{
	index = p_index;
	nameLink = p->AddHyperLink("namelink" + to_string(index), Vector2i(), LeaderboardDisplay::CHAR_HEIGHT, "", "");
}

void LeaderboardEntryRow::Clear()
{
	set = false;
}

void LeaderboardEntryRow::Set(KineticLeaderboardEntry &entryInfo)
{
	set = true;

	rankText.setString(to_string(entryInfo.steamEntry.m_nGlobalRank));

	nameLink->SetString(entryInfo.name);
	nameLink->SetLinkURL("https://steamcommunity.com/profiles/" + to_string(entryInfo.steamEntry.m_steamIDUser.ConvertToUint64()));

	scoreText.setString(entryInfo.timeStr);
}

void LeaderboardEntryRow::SetTopLeft(const sf::Vector2f &p_topLeft)
{
	//nameText.setPosition(topLeft);
	float rankSpacing = 20;
	float nameSpacing = rankSpacing + 100;
	float scoreSpacing = nameSpacing + 200;

	Vector2f topLeft = p_topLeft;

	rankText.setPosition(topLeft.x + rankSpacing, topLeft.y + 4);
	nameLink->SetPos(Vector2i(topLeft.x + nameSpacing, topLeft.y + 4));
	scoreText.setPosition(topLeft.x + scoreSpacing, topLeft.y + 4);
}

void LeaderboardEntryRow::Draw(sf::RenderTarget *target)
{
	if (set)
	{
		//target->draw(nameText);
		target->draw(scoreText);
		target->draw(rankText);
	}
}