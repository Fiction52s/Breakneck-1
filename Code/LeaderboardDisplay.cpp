#include "Leaderboard.h"
#include <iostream>
#include "MainMenu.h"
#include "Session.h"
#include "UIMouse.h"

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

	panel = new Panel("leaderboard", 1920, 1080, this);
	panel->SetColor(Color::Transparent);

	Hide();

	Color evenColor = Color(200, 200, 200, 200);
	Color oddColor = Color(150, 150, 150, 200);
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		if (i % 2 == 0)
		{
			SetRectColor(rowQuads + i * 4, evenColor);
		}
		else
		{
			SetRectColor(rowQuads + i * 4, oddColor );
		}
		
	}

	SetRectColor(bgQuad, Color(100, 100, 200, 150));
	SetRectTopLeft(bgQuad, 1920, 1080, Vector2f(0, 0));

	for (int i = 0; i < NUM_ROWS; ++i)
	{
		rows[i].Init(i, panel);
	}

	SetTopLeft(Vector2f(960 - ROW_WIDTH / 2, 540 - (NUM_ROWS / 2 * ROW_HEIGHT)));
}

LeaderboardDisplay::~LeaderboardDisplay()
{
	delete panel;
}

bool LeaderboardDisplay::IsHidden()
{
	return action == A_HIDDEN;
}

void LeaderboardDisplay::Show()
{
	action = A_LOADING;
	frame = 0;

	MOUSE.Show();
	MOUSE.SetControllersOn(true);
}

void LeaderboardDisplay::Hide()
{
	action = A_HIDDEN;
	frame = 0;

	MOUSE.Hide();
	MOUSE.SetControllersOn(false);
}

void LeaderboardDisplay::Start( const std::string &boardName )
{
	Show();

	manager.DownloadBoard(boardName);
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

void LeaderboardDisplay::HandleEvent(sf::Event ev)
{
	if (IsHidden())
		return;

	if (action == A_UPLOAD_FAILED_POPUP)
	{
		messagePop.panel->HandleEvent(ev);
	}
	else
	{
		panel->HandleEvent(ev);
	}
}

void LeaderboardDisplay::Update()
{
	if (action == A_HIDDEN)
		return;

	if (action == A_LOADING)
	{
		if (manager.IsIdle())
		{
			action = A_SHOWING;
			frame = 0;

			int numEntries = manager.currBoard.entries.size();

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

				rows[i].Set(manager.currBoard.entries[i]);
			}
			//if (false)//manager.lastUploadSuccess)
			//{
			//	
			//}
			//else
			//{
			//	messagePop.Pop("Score upload failed. Try again later");
			//	action = A_UPLOAD_FAILED_POPUP;
			//}
			
		}
	}

	Session *sess = Session::GetSession();

	ControllerState currState = sess->GetCurrInput(0);
	ControllerState prevState = sess->GetPrevInput(0);

	if (currState.start && !prevState.start)
	{
		Hide();
	}

	if (action == A_UPLOAD_FAILED_POPUP)
	{
		messagePop.Update();

		if (!messagePop.IsActive())
		{
			action = A_LOADING;
		}
	}
	else
	{
		panel->MouseUpdate();
	}

	++frame;
}

void LeaderboardDisplay::Draw(sf::RenderTarget *target)
{
	if (action == A_HIDDEN)
	{
		return;
	}

	target->draw(bgQuad, 4, sf::Quads);

	target->draw(rowQuads, NUM_ROWS * 4, sf::Quads);

	if (action == A_SHOWING)
	{
		for (int i = 0; i < NUM_ROWS; ++i)
		{
			rows[i].Draw(target);
		}

		panel->Draw(target);
	}

	if (action == A_UPLOAD_FAILED_POPUP)
	{
		messagePop.Draw(target);
	}
}