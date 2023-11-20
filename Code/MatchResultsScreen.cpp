#include "MatchResultsScreen.h"
#include "GameSession.h"
#include "UIWindow.h"
#include "MapHeader.h"
#include "MainMenu.h"
#include "PlayerSkinShader.h"
#include "NetplayManager.h"
#include "LoadingPopup.h"

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

VictoryScreen4Player::VictoryScreen4Player( MatchStats *mStats )
	:MatchResultsScreen( mStats )
{
	maxFramesToShowResults = 30 * 60;
	for (int i = 0; i < 4; ++i)
	{
		if (matchStats->playerStats[i] != NULL)
		{
			playerBar[i] = new PlayerInfoBar(this, i);
		}
		else
		{
			playerBar[i] = NULL;
		}
	}

	Tileset *ts_bg = GetSizedTileset("Menu/Multiplayer/Results/win_bg_960x540.png");
	Tileset *ts_win = GetSizedTileset("Menu/Multiplayer/Results/win_960x540.png");
	Tileset *ts_kin = GetSizedTileset("Menu/Multiplayer/Results/win_kin_1_960x540.png");

	action = A_SHOW_WINNER;
	frame = 0;

	SetRectCenter(fadeQuad, 1920, 1080, Vector2f(960, 540));

	ts_bg->SetSpriteTexture(bgSpr);
	bgSpr.setScale(2, 2);

	ts_win->SetSpriteTexture(winSpr);
	winSpr.setScale(2, 2);

	ts_kin->SetSpriteTexture(kinSpr);
	kinSpr.setScale(2, 2);

	fadeFrames = 60;
	SetRectColor(fadeQuad, Color(0, 0, 0, 0));

	//waitingPopup = new LoadingPopup;
	//player2Bar->SetBottomLeftPos( Vector2f( 0, 128 ) );
}

VictoryScreen4Player::~VictoryScreen4Player()
{
	for (int i = 0; i < 4; ++i)
	{
		if (playerBar[i] != NULL)
			delete playerBar[i];
	}

	//delete waitingPopup;
}

void VictoryScreen4Player::WaitForOthers()
{
	//Finish()

	action = A_WAITING_FOR_OTHERS;
	frame = 0;
	//waitingPopup->Pop("Waiting for other players", "");


	MainMenu *mm = MainMenu::GetInstance();

	assert(mm->netplayManager->playerIndex != -1);

	playerBar[mm->netplayManager->playerIndex]->Finish();

	mm->netplayManager->SendFinishedResultsScreenSignalToPeers();

	if (mm->netplayManager->IsHost())
	{
		mm->netplayManager->HostFinishResultsScreen();
	}
}

bool VictoryScreen4Player::Update()
{
	int opacity = 0;
	int finalOpacity = 200;

	if (action == A_SHOW_WINNER && frame == 30)
	{
		action = A_FADE_WINNER;
		frame = 0;
	}
	else if (action == A_FADE_WINNER && frame == fadeFrames)
	{
		action = A_SHOW_RESULTS;
		frame = 0;
		opacity = finalOpacity;
	}


	if (action == A_SHOW_RESULTS || action == A_WAITING_FOR_OTHERS)
	{
		assert(matchStats->netplay);
		MainMenu *mm = MainMenu::GetInstance();

		for (int i = 0; i < 4; ++i)
		{
			if (i == mm->netplayManager->playerIndex)
			{
				continue;
			}

			if (mm->netplayManager->netplayPlayers[i].finishedWithResultsScreen && playerBar[i]->action == PlayerInfoBar::A_READY)
			{
				playerBar[i]->Finish();
			}
		}

	}
	
	if (action == A_WAITING_FOR_OTHERS)
	{	
		//waitingPopup->Update();

		assert(matchStats->netplay);
		MainMenu *mm = MainMenu::GetInstance();

		if (mm->netplayManager->receivedPostOptionsSignal)
		{
			cout << "receivedPostOptionSignal, waiting is over" << "\n";
			return false;
		}
	}


	if (action == A_SHOW_RESULTS)
	{
		MainMenu *mm = MainMenu::GetInstance();


		bool confirmComplete = false;
		if (matchStats->netplay)
		{
			confirmComplete = CONTROLLERS.ButtonPressed_Start();

			if (frame >= maxFramesToShowResults)
			{
				if (matchStats->quickplay)
				{

				}
				else
				{
					WaitForOthers();
					return true;
				}
			}
			

			/*if (!mm->netplayManager->IsHost())
			{
				
			}*/
			
			/*for (int i = 0; i < 4; ++i)
			{
				if (mm->GetCurrInput(i).start && mm->GetPrevInput(i).start)
				{
					confirmComplete = true;
					break;
				}
			}*/
		}

		

		for (int i = 0; i < 4; ++i)
		{
			if (playerBar[i] != NULL)
			{
				playerBar[i]->Update(confirmComplete);
			}
		}

		bool done = true;
		for (int i = 0; i < 4; ++i)
		{
			if (playerBar[i] != NULL)
			{
				if (playerBar[i]->action != PlayerInfoBar::A_READY && playerBar[i]->action != PlayerInfoBar::A_DONE )
				{
					done = false;
					break;
				}
			}
		}

		if (done)
		{
			if (confirmComplete)
			{
				if (matchStats->quickplay)
				{
					return false;
				}
				else
				{
					WaitForOthers();
					return true;
				}
			}
				
		}

		int activateWait = 15;
		if (frame % activateWait == 0 )
		{
			int index = frame / activateWait;

			if (index < 4 && playerBar[index] != NULL)
			{
				playerBar[index]->Activate();
			}
		}
		/*for (int i = 0; i < 4; ++i)
		{
			if (playerBar[i] != NULL)
			{
				playerBar[i]->Activate();
			}
		}*/
	}


	if (action == A_FADE_WINNER)
	{
		float factor = ((float)frame) / fadeFrames;

		opacity = finalOpacity * factor;
		SetRectColor(fadeQuad, Color(0, 0, 0, opacity));
	}

	++frame;

	return true;
}

void VictoryScreen4Player::ResetSprites()
{
}
void VictoryScreen4Player::UpdateSprites()
{
}

void VictoryScreen4Player::Reset()
{
	MatchResultsScreen::Reset();
}

void VictoryScreen4Player::Draw(sf::RenderTarget *target)
{
	target->draw(bgSpr);
	target->draw(kinSpr);
	target->draw(winSpr);
	target->draw(fadeQuad, 4, sf::Quads );

	for (int i = 0; i < 4; ++i)
	{
		if (playerBar[i] != NULL)
			playerBar[i]->Draw(target);
	}

	/*if (action == A_WAITING_FOR_OTHERS)
	{
		sf::RectangleShape rect;
		rect.setFillColor(Color(0, 0, 0, 100));
		rect.setSize(Vector2f(1920, 1080));
		rect.setPosition(0, 0);
		target->draw(rect);
		waitingPopup->Draw(target);
	}*/
}

PlayerInfoBar::PlayerInfoBar( MatchResultsScreen *mrs, int playerIndex )
	:resultsScreen( mrs )
{
	//action = A_IDLE;
	//frame = 0;
	pIndex = playerIndex;

	width = 1920  / 4;

	PlayerStats *ps = resultsScreen->matchStats->playerStats[playerIndex];

	Tileset *ts_placing = resultsScreen->GetSizedTileset("HUD/keynum_dark_80x80.png");

	ts_placing->SetSpriteTexture(placingSpr);
	ts_placing->SetSubRect( placingSpr, ps->placing + 1);

	placingSpr.setOrigin(placingSpr.getLocalBounds().width / 2, placingSpr.getLocalBounds().height);
	placingSpr.setScale(2, 2);

	skinShader = new PlayerSkinShader("player");

	skinShader->SetSkin(ps->skinIndex);

	Tileset *ts_kin = resultsScreen->GetSizedTileset("Kin/stand_64x64.png");

	ts_kin->SetSpriteTexture(kinSprite);
	ts_kin->SetSubRect(kinSprite, 0);

	kinSprite.setOrigin(kinSprite.getLocalBounds().width / 2, kinSprite.getLocalBounds().height);
	kinSprite.setScale(4, 4);

	nameText.setFont(MainMenu::GetInstance()->arial);
	nameText.setCharacterSize(64);
	nameText.setFillColor(Color::White);
	nameText.setString(ps->name);
	auto lb = nameText.getLocalBounds();
	nameText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);

	infoText.setFont(MainMenu::GetInstance()->arial);
	infoText.setCharacterSize(64);
	infoText.setFillColor(Color::White);

	if (resultsScreen->matchStats->gameModeType == MatchParams::GAME_MODE_FIGHT)
	{
		string infoStr = "Kills: " + to_string(ps->kills);// +"\nKills: " + to_string(ps->kills);
		infoText.setString(infoStr);
	}

	//auto lbInfoText = nameText.getLocalBounds();
	//nameText.setOrigin(lbInfoText.left + lb.width / 2, lb.top + lb.height / 2);

	startHeight = -300;
	waitHeight = 300;
	switch (ps->placing)
	{
	case 0:
		waitHeight += 300;
		break;
	case 1:
		waitHeight += 150;
		break;
	case 2:
		waitHeight += 80;
		break;
	case 3:
		waitHeight += 0;
		break;
	}

	

	actionLength[A_IDLE] = 1;
	actionLength[A_RISE] = 30;
	actionLength[A_WAIT] = 1;
	actionLength[A_READY] = 1;

	action = A_IDLE;
	frame = 0;
	SetHeight(startHeight);

	sf::Color rectColors[] = { Color::Green, Color::Red, Color::Cyan, Color::Yellow };

	SetRectColor(quad, rectColors[pIndex]);

	//SetHeight(0);
}

PlayerInfoBar::~PlayerInfoBar()
{
	delete skinShader;
}

void PlayerInfoBar::Finish()
{
	SetRectColor(quad, Color::White);
	action = A_DONE;
}

void PlayerInfoBar::Draw( sf::RenderTarget *target )
{
	if (action != A_IDLE)
	{
		target->draw(quad, 4, sf::Quads);
		target->draw(nameText);
		target->draw(infoText);

		target->draw(kinSprite, &skinShader->pShader);

		target->draw(placingSpr);
	}
}

void PlayerInfoBar::Activate()
{
	sf::Color rectColors[] = { Color::Green, Color::Red, Color::Cyan, Color::Yellow };
	SetRectColor(quad, rectColors[pIndex]);
	//^testing changing the color as a temporary indicator

	action = A_RISE;
	frame = 0;
	SetHeight(startHeight);
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
		//action = A_RISE;
		//frame = 0;
		//SetHeight(0);
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
		action = A_READY;
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

	nameText.setPosition(topLeft + Vector2f(width / 2, -24 - kinSprite.getGlobalBounds().height - 64));

	SetRectTopLeft(quad, width, currHeight, topLeft);

	kinSprite.setPosition(topLeft + Vector2f(width / 2, -24));

	infoText.setPosition(topLeft + Vector2f(10, 10));

	placingSpr.setPosition(topLeft + Vector2f(width / 2, waitHeight - 80));
}