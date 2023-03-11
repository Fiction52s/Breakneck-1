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

	rankSpacing = 20;
	nameSpacing = rankSpacing + 100;
	scoreSpacing = nameSpacing + 200 + 100;
	ghostSpacing = scoreSpacing + 150;
	watchSpacing = ghostSpacing + 100;

	Clear();
}

void LeaderboardEntryRow::Init( int p_index, Panel *p)
{
	index = p_index;
	string indexStr = to_string(index);
	nameLink = p->AddHyperLink("namelink" + indexStr, Vector2i(), LeaderboardDisplay::CHAR_HEIGHT, "", "");
	ghostCheckBox = p->AddCheckBox("ghostcheckbox" + indexStr, Vector2i(), false);
	watchButton = p->AddButton("watchbutton" + indexStr, Vector2i(), Vector2f(100, LeaderboardDisplay::CHAR_HEIGHT), "Watch");
}

void LeaderboardEntryRow::Clear()
{
	set = false;

	if (index >= 0)
	{
		ghostCheckBox->checked = false;
		nameLink->HideMember();
		ghostCheckBox->HideMember();
		watchButton->HideMember();
	}
}

void LeaderboardEntryRow::Set(KineticLeaderboardEntry &entryInfo)
{
	nameLink->ShowMember();
	ghostCheckBox->ShowMember();
	watchButton->ShowMember();

	if (entryInfo.ghostOn)
	{
		ghostCheckBox->checked = true;
	}

	set = true;

	rankText.setString(to_string(entryInfo.steamEntry.m_nGlobalRank));

	nameLink->SetString(entryInfo.name);
	nameLink->SetLinkURL("https://steamcommunity.com/profiles/" + to_string(entryInfo.steamEntry.m_steamIDUser.ConvertToUint64()));

	scoreText.setString(entryInfo.timeStr);
}

void LeaderboardEntryRow::SetTopLeft(const sf::Vector2f &p_topLeft)
{
	//nameText.setPosition(topLeft);
	

	Vector2f topLeft = p_topLeft;

	float yExtra = 4;

	rankText.setPosition(topLeft.x + rankSpacing, topLeft.y + yExtra);
	nameLink->SetPos(Vector2i(topLeft.x + nameSpacing, topLeft.y + yExtra));
	scoreText.setPosition(topLeft.x + scoreSpacing, topLeft.y + yExtra);

	ghostCheckBox->SetPos(Vector2i(topLeft.x + ghostSpacing, topLeft.y + yExtra + 8));
	watchButton->SetPos(Vector2i(topLeft.x + watchSpacing, topLeft.y + yExtra + 5));
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