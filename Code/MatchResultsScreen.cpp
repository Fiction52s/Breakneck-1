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

VictoryScreen2PlayerVS::VictoryScreen2PlayerVS( MatchStats *mStats )
	:MatchResultsScreen( mStats )
{
	player1Bar = new PlayerInfoBar(this, 0);
	player2Bar = new PlayerInfoBar(this, 1);
	//player2Bar->SetBottomLeftPos( Vector2f( 0, 128 ) );
}

void MatchResultsScreen::Reset()
{
	frame = 0;
}

bool VictoryScreen2PlayerVS::Update()
{
	MainMenu *mm = MainMenu::GetInstance();

	player1Bar->Update(mm->GetCurrInput( 0 ).A && !mm->GetPrevInput( 0 ).A );
	player2Bar->Update(mm->GetCurrInput( 1 ).A && !mm->GetPrevInput( 1 ).A );

	if (player1Bar->action == PlayerInfoBar::A_DONE &&
		player2Bar->action == PlayerInfoBar::A_DONE)
	{
		if (mm->GetCurrInput(0).start || mm->GetCurrInput(1).start)
		{
			return false;
		}
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

//ResultsScreen::ResultsScreen(GameSession *p_owner)
//	:owner( p_owner )
//{
//	frame = 0;
//	state = FADEIN;
//
//	memset(columnReady, 0, sizeof(columnReady));
//
//	for (int i = 0; i < 4; ++i)
//	{
//		SetBoxPos(i, 1080);
//	}
//
//	memset(ts_column, 0, sizeof(ts_column));
//
//	slideInStartFrame[0] = 0;
//	slideInStartFrame[1] = 60;
//	slideInStartFrame[2] = 120;
//	slideInStartFrame[3] = 180;
//
//	slideInFrames[0] = 60;
//	slideInFrames[1] = 60;
//	slideInFrames[2] = 60;
//	slideInFrames[3] = 60;
//
//	slideOutFrames = 60;
//}
//
//void ResultsScreen::SetBoxPos(int boxIndex, float yHeight)
//{
//	columnSprites[boxIndex].setPosition(boxIndex * 1920 / 4, yHeight);
//	/*boxes[boxIndex * 4].position = Vector2f(boxIndex * 1920 / 4, yHeight);
//	boxes[boxIndex * 4+1].position = Vector2f((boxIndex+1)* 1920 / 4, yHeight);
//	boxes[boxIndex * 4+2].position = Vector2f((boxIndex + 1) * 1920 / 4, yHeight + 1080);
//	boxes[boxIndex * 4+3].position = Vector2f(boxIndex * 1920 / 4, yHeight + 1080);*/
//}
//
//void ResultsScreen::SetTile(int boxIndex, int tile)
//{
//	if (ts_column[boxIndex] != NULL )
//	{
//		sf::Texture *tex = ts_column[boxIndex]->texture;
//		sf::IntRect rect = ts_column[boxIndex]->GetSubRect(tile);
//		columnSprites[boxIndex].setTexture(*tex);
//		columnSprites[boxIndex].setTextureRect(rect);
//	}
//	/*boxes[boxIndex * 4].texCoords = Vector2f(rect.left, rect.top);
//	boxes[boxIndex * 4 + 1].texCoords = Vector2f(rect.left + rect.width, rect.top);
//	boxes[boxIndex * 4 + 2].texCoords = Vector2f(rect.left + rect.width, rect.top + rect.height);
//	boxes[boxIndex * 4 + 3].texCoords = Vector2f(rect.left, rect.top + rect.height);*/
//}
//
//Tileset * ResultsScreen::GetTeamTileset(int teamIndex, bool win )
//{
//	switch (teamIndex)
//	{
//	case 0:
//		if (win)
//		{
//			return owner->GetTileset("Menu/Results/blue_win_480x1080.png", 480, 1080);
//		}
//		else
//		{
//			return owner->GetTileset("Menu/Results/blue_lose_480x1080.png", 480, 1080);
//		}
//		
//		break;
//	case 1:
//		if (win)
//		{
//			return owner->GetTileset("Menu/Results/red_win_480x1080.png", 480, 1080);
//		}
//		else
//		{
//			return owner->GetTileset("Menu/Results/red_lose_480x1080.png", 480, 1080);
//		}
//	case 2:
//		if (win)
//		{
//			return owner->GetTileset("Menu/Results/green_win_480x1080.png", 480, 1080);
//		}
//		else
//		{
//			return owner->GetTileset("Menu/Results/green_lose_480x1080.png", 480, 1080);
//		}
//	case 3:
//		if (win)
//		{
//			return owner->GetTileset("Menu/Results/purp_win_480x1080.png", 480, 1080);
//		}
//		else
//		{
//			return owner->GetTileset("Menu/Results/purp_lose_480x1080.png", 480, 1080);
//		}
//	
//		break;
//	}
//}
//
//Tileset * ResultsScreen::GetSoloTilset(int soloIndex, bool win)
//{
//	return NULL;
//}
//
//void ResultsScreen::SetupColumns()
//{
//	maxPlacing = 0;
//	for (int i = 0; i < 4; ++i)
//	{
//		SetBoxPos(i, 1080);
//	}
//
//	int currPlace = 0;
//	switch (owner->gameModeType)
//	{
//	case MatchParams::GAME_MODE_REACHENEMYBASE:
//	{
//		for (int i = 0; i < 4; ++i)
//		{
//			//currPlace = owner->raceFight->place[i];
//			if (currPlace == 1)
//			{
//				ts_column[i] = GetTeamTileset(owner->GetPlayerTeamIndex(i), true);
//				maxPlacing = 1;
//			}
//			else if (currPlace > 1)
//			{
//				ts_column[i] = GetTeamTileset(owner->GetPlayerTeamIndex(i), false);
//				if (currPlace > maxPlacing)
//					maxPlacing = currPlace;
//			}
//			else
//			{
//				ts_column[i] = NULL;
//			}
//		}
//		break;
//	}
//
//	}
//}
//
//bool ResultsScreen::Update()
//{
//	switch (state)
//	{
//	case FADEIN:
//		state = SLIDEIN;
//		frame = 0;
//		break;
//	case SLIDEIN:
//		if (frame <= slideInStartFrame[maxPlacing-1] + slideInFrames[maxPlacing-1])
//		{
//			for (int i = 0; i < 4; ++i)
//			{
//				if (owner->gameModeType == MatchParams::GAME_MODE_REACHENEMYBASE)
//				{
//					/*if (owner->raceFight->place[i] == 1 && frame <= slideInFrames[0])
//					{
//						double f = (double)frame / slideInFrames[0];
//						double a = slideInBez[1].GetValue(f);
//
//						SetBoxPos(i, 1080.0 * (1.0 - a));
//					}
//					else if (owner->raceFight->place[i] == 2 && frame <= slideInFrames[1] + slideInStartFrame[1])
//					{
//						double f = (double)(frame - slideInFrames[1]) / slideInStartFrame[1];
//						double a = slideInBez[1].GetValue(f);
//
//						SetBoxPos(i, 1080.0 * (1.0 - a));
//					}*/
//				}
//
//				
//			}
//		}
//		else
//		{
//			state = WAIT;
//			frame = 0;
//		}
//		break;
//	case WAIT:
//	{
//		for (int i = 0; i < 4; ++i)
//		{
//			if ((owner->GetCurrInput(i).A && !owner->GetPrevInput(i).A)
//				|| (owner->GetCurrInput(i).start && !owner->GetPrevInput(i).start))
//			{
//				columnReady[i] = !columnReady[i];
//			}
//		}
//
//		bool allReady = true;
//		for (int i = 0; i < 4; ++i)
//		{
//			if (ts_column[i] != NULL && !columnReady[i] )
//			{
//				allReady = false;
//				break;
//			}
//		}
//
//		if (allReady)
//		{
//			state = SLIDEOUT;
//			frame = 0;
//		}
//
//		break;
//	}
//	case SLIDEOUT:
//		/*if (frame == 60)
//		{
//			state = FADEOUT;
//			frame = 0;
//		}*/
//
//		if (frame <= slideOutFrames )
//		{
//			for (int i = 0; i < 4; ++i)
//			{
//				if (owner->gameModeType == MatchParams::GAME_MODE_REACHENEMYBASE)
//				{
//					double f = (double)frame / slideOutFrames;
//					double a = slideOutBez.GetValue(f);
//					SetBoxPos(i, 1080.0 * a);
//				}
//			}
//		}
//		else
//		{
//			state = DONE;
//			frame = 0;
//		}
//		break;
//	case FADEOUT:
//		if (frame == 0)
//		{
//			state = DONE;
//			frame = 0;
//		}
//		break;
//	}
//
//	switch (state)
//	{
//	case FADEIN:
//		break;
//	case SLIDEIN:
//		break;
//	case WAIT:
//		break;
//	case SLIDEOUT:
//		break;
//	case FADEOUT:
//		break;
//	}
//
//	for (int i = 0; i < 4; ++i)
//	{
//		SetTile(i, (int)( !columnReady[i] ));
//	}
//
//	++frame;
//
//	return true;
//}
//
//void ResultsScreen::ResetSprites()
//{
//
//}
//
//bool ResultsScreen::IsDone()
//{
//	return (state == DONE);
//}
//
//void ResultsScreen::UpdateSprites()
//{
//
//}
//
//void ResultsScreen::Reset()
//{
//	for (int i = 0; i < 4; ++i)
//	{
//		columnReady[i] = false;
//		columnSprites[i] = sf::Sprite();
//		ts_column[i] = NULL;
//	}
//	frame = 0;
//	state = SLIDEIN;
//}
//
//void ResultsScreen::Draw(RenderTarget *target)
//{
//	switch (owner->gameModeType)
//	{
//	case MatchParams::GAME_MODE_REACHENEMYBASE:
//	{
//		for (int i = 0; i < 4; ++i)
//		{
//			//if( owner->raceFight->place[i] > 0 )
//			//	target->draw(columnSprites[i]);
//		}
//		break;
//	}
//	}
//	
//}