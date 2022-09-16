#include "MatchResultsScreen.h"
#include "GameSession.h"
#include "UIWindow.h"
#include "MapHeader.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

MatchResultsScreen::MatchResultsScreen(MatchStats *mStats)
{
	assert(mStats != NULL);
	matchStats = mStats;

	Reset();
}

MatchResultsScreen::~MatchResultsScreen()
{
	delete matchStats;
}

void MatchResultsScreen::Reset()
{
	frame = 0;
}

VictoryScreen2PlayerVS::VictoryScreen2PlayerVS( MatchStats *mStats )
	:MatchResultsScreen( mStats )
{
	player1Bar = new PlayerInfoBar(this, 0);
	player2Bar = new PlayerInfoBar(this, 1);
	//player2Bar->SetBottomLeftPos( Vector2f( 0, 128 ) );
}

VictoryScreen2PlayerVS::~VictoryScreen2PlayerVS()
{
	delete player1Bar;
	delete player2Bar;
}


bool VictoryScreen2PlayerVS::Update()
{
	MainMenu *mm = MainMenu::GetInstance();

	bool confirmComplete = false;
	if (matchStats->netplay)
	{
		confirmComplete = mm->menuCurrInput.start;
		/*for (int i = 0; i < 4; ++i)
		{
			if (mm->GetCurrInput(i).start && mm->GetPrevInput(i).start)
			{
				confirmComplete = true;
				break;
			}
		}*/
	}
	

	player1Bar->Update(confirmComplete);
	player2Bar->Update(confirmComplete);

	if (player1Bar->action == PlayerInfoBar::A_DONE &&
		player2Bar->action == PlayerInfoBar::A_DONE)
	{
		if (confirmComplete)
			return false;
	}
	++frame;

	return true;
}

void VictoryScreen2PlayerVS::ResetSprites()
{
}
void VictoryScreen2PlayerVS::UpdateSprites()
{
}

void VictoryScreen2PlayerVS::Reset()
{
	MatchResultsScreen::Reset();
}

void VictoryScreen2PlayerVS::Draw(sf::RenderTarget *target)
{
	player1Bar->Draw(target);
	player2Bar->Draw(target);
}

PlayerInfoBar::PlayerInfoBar( MatchResultsScreen *mrs, int playerIndex )
	:resultsScreen( mrs )
{
	action = A_IDLE;
	frame = 0;
	pIndex = playerIndex;

	width = 1920  / 4;

	PlayerStats *ps = resultsScreen->matchStats->playerStats[playerIndex];

	nameText.setFont(MainMenu::GetInstance()->arial);
	nameText.setCharacterSize(20);
	nameText.setFillColor(Color::White);
	nameText.setString(ps->name);

	startHeight = 0;
	waitHeight = 300;

	actionLength[A_IDLE] = 1;
	actionLength[A_RISE] = 20;
	actionLength[A_WAIT] = 1;
	actionLength[A_DONE] = 1;

	sf::Color rectColors[] = { Color::Green, Color::Red, Color::Cyan, Color::Yellow };

	SetRectColor(quad, rectColors[pIndex]);
}

void PlayerInfoBar::Draw( sf::RenderTarget *target )
{
	target->draw(quad, 4, sf::Quads);
	target->draw(nameText);
}

void PlayerInfoBar::Update( bool pressedA )
{
	if (frame == actionLength[action])
	{
		frame = 0;
		switch (action)
		{
		case A_IDLE:
		{
			break;
		}
		case A_RISE:
		{
			action = A_WAIT;
			SetHeight(waitHeight);
			break;
		}
		case A_WAIT:
		{
			break;
		}
		}
	}

	switch (action)
	{
	case A_IDLE:
	{
		action = A_RISE;
		frame = 0;
		break;
	}
	case A_RISE:
	{
		CubicBezier bez(0, 0, 1, 1);
		double f = bez.GetValue((double)frame / actionLength[A_RISE]);
		double h = startHeight * (1.0 - f) + waitHeight * f;
		SetHeight( h );
		break;
	}
	case A_WAIT:
	{
		action = A_DONE;
		frame = 0;
		break;
	}
	}

	++frame;
}

void PlayerInfoBar::SetHeight( int height )
{
	currHeight = height;
	

	int startLeft = 50;
	Vector2f topLeft(startLeft + pIndex * width, 1080 - currHeight );

	nameText.setPosition(topLeft + Vector2f(10, 30));

	SetRectTopLeft(quad, width, currHeight, topLeft);
}