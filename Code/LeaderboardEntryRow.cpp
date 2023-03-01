#include "Leaderboard.h"
#include <iostream>
#include "MainMenu.h"

using namespace std;
using namespace sf;

LeaderboardEntryRow::LeaderboardEntryRow()
{
	MainMenu *mainMenu = MainMenu::GetInstance();

	int charHeight = LeaderboardDisplay::ROW_HEIGHT - 2;
	Color col = Color::Black;

	nameText.setFont(mainMenu->arial);
	nameText.setFillColor(col);
	nameText.setCharacterSize(charHeight);

	nameText.setFont(mainMenu->arial);
	nameText.setFillColor(col);
	nameText.setCharacterSize(charHeight);

	Clear();
}

void LeaderboardEntryRow::Clear()
{
	set = false;
}

void LeaderboardEntryRow::Set(KineticLeaderboardEntry &entryInfo)
{
	set = true;
	nameText.setString(entryInfo.name);
	scoreText.setString(entryInfo.timeStr);
}

void LeaderboardEntryRow::SetTopLeft(const sf::Vector2f &topLeft)
{
	nameText.setPosition(topLeft);
	scoreText.setPosition(topLeft + Vector2f(200, 0));
}

void LeaderboardEntryRow::Draw(sf::RenderTarget *target)
{
	if (set)
	{
		target->draw(nameText);
		target->draw(scoreText);
	}
}