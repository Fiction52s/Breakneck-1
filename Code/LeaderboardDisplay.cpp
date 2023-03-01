#include "Leaderboard.h"
#include <iostream>
#include "MainMenu.h"

using namespace std;
using namespace sf;

KineticLeaderboardEntry::KineticLeaderboardEntry()
{
	Clear();
}

void KineticLeaderboardEntry::Init()
{
	name = SteamFriends()->GetFriendPersonaName(steamEntry.m_steamIDUser);
	timeStr = GetTimeStr(steamEntry.m_nScore);
}

void KineticLeaderboardEntry::Clear()
{
	name = "";
	timeStr = "";
	memset(&steamEntry, 0, sizeof(steamEntry));
}

LeaderboardDisplay::LeaderboardDisplay()
{
	topIndex = 0;

	for (int i = 0; i < NUM_ROWS; ++i)
	{
		SetRectColor(rowQuads + i * 4, Color( 100, 100, 100, 100 ));
	}
}

void LeaderboardDisplay::Start(LeaderboardInfo *p_currBoard)
{
	currBoard = p_currBoard;

	int numEntries = currBoard->entries.size();

	for (int i = 0; i < NUM_ROWS; ++i)
	{
		rows[i].Clear();
	}

	for (int i = 0; i < NUM_ROWS; ++i)
	{
		if (i == numEntries)
		{
			break;
		}

		rows[i].Set(currBoard->entries[i]);
	}
}

void LeaderboardDisplay::SetTopLeft(const sf::Vector2f &p_pos)
{
	topLeft = p_pos;

	Vector2f rowTopLeft;
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		rowTopLeft = topLeft + Vector2f(0, i * ROW_HEIGHT);
		SetRectTopLeft(rowQuads + i * 4, ROW_WIDTH, ROW_HEIGHT, rowTopLeft);
		rows[i].SetTopLeft(rowTopLeft);
	}
}

void LeaderboardDisplay::Update()
{

}

void LeaderboardDisplay::Draw(sf::RenderTarget *target)
{
	target->draw(rowQuads, NUM_ROWS * 4, sf::Quads);

	for (int i = 0; i < NUM_ROWS; ++i)
	{
		rows[i].Draw(target);
	}
}