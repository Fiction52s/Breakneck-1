#include "PracticeInviteDisplay.h"
#include "Tileset.h"
#include "Session.h"
#include "Actor.h"
#include "MainMenu.h"
#include "NetplayManager.h"

using namespace sf;
using namespace std;

PracticePlayerDisp::PracticePlayerDisp(PracticeInviteDisplay *p_disp )
	:playerSkinShader("player")
{
	disp = p_disp;

	ts_kin = disp->sess->GetPlayer(0)->STAND_GetTileset();

	kinSprite.setTexture(*ts_kin->texture);
	kinSprite.setTextureRect(ts_kin->GetSubRect(0));
	//kinSprite.setScale(.5, .5);

	playerSkinShader.SetSubRect(ts_kin, ts_kin->GetSubRect(0));

	playerSkinShader.SetSkin(0);

	nameText.setFont(disp->sess->mainMenu->arial);
	nameText.setFillColor(Color::White);
	nameText.setCharacterSize(24);
}

void PracticePlayerDisp::Reset()
{
	playerSkinShader.SetSkin(0);
	SetName("");

	//SetName("MBJHAIHETHT");
}

void PracticePlayerDisp::SetTopLeft(sf::Vector2f p_topLeft)
{
	topLeft = p_topLeft;

	//kinSprite.setScale(.5, .5);
	kinSprite.setOrigin(0, 0);
	kinSprite.setPosition(topLeft);
	nameText.setPosition(topLeft + Vector2f( kinSprite.getGlobalBounds().width, 0 ));
}

void PracticePlayerDisp::SetSkin(int index)
{
	playerSkinShader.SetSkin(index);
}

void PracticePlayerDisp::SetName(const std::string &name)
{
	nameText.setString(name);
}

float PracticePlayerDisp::GetKinBotPos()
{
	return kinSprite.getGlobalBounds().top + kinSprite.getGlobalBounds().height;
}

void PracticePlayerDisp::Draw(sf::RenderTarget *target)
{
	target->draw(kinSprite, &playerSkinShader.pShader);
	target->draw(nameText);
}


PracticeUserBox::PracticeUserBox(PracticeInviteDisplay *p_disp)
	:playerDisp( p_disp )
{
	disp = p_disp;

	Color bottomTextColor = Color::Black;

	ts_raceButton = disp->sess->GetSizedTileset("Menu/practiceracebutton_100x32.png");

	

	selected = false;

	Reset();
}

void PracticeUserBox::Reset()
{
	SetAction(A_EMPTY);
	
	practicePlayer = NULL;
	playerDisp.Reset();
}

void PracticeUserBox::Update()
{
	UpdateBGQuad();

	//testing
	/*if (true)
	{
		if (selected)
		{
			int f = (frame / 15) % 2;
			SetRectSubRect(raceButtonQuad, ts_raceButton->GetSubRect(f));
		}
		else
		{
			SetRectSubRect(raceButtonQuad, ts_raceButton->GetSubRect(2));
		}
	}

	UpdateBGQuad();

	++frame;*/

	if (action == A_EMPTY)
		return;

	if (!practicePlayer->isConnectedTo)
	{
		SetAction(A_EMPTY);
		return;
	}

	if (practicePlayer != NULL && practicePlayer->isRaceClient)
	{
		SetAction(A_RUNNING);
	}


	switch (action)
	{
	case A_HAS_PLAYER:
	{
		if (practicePlayer->wantsToPlay)
		{
			SetAction(A_PLAYER_WANTS_TO_PLAY);
		}
		break;
	}
	/*case A_WAITING_FOR_CONFIRM:
	{
		if (practicePlayer->hasConfirmedRaceStart)
		{
			SetAction(A_RACE_CONFIRMED);
		}
		break;
	}*/
	}

	if (action == A_PLAYER_WANTS_TO_PLAY)
	//if( true )
	{
		if (selected)
		{
			int f = (frame / 4) % 2;
			SetRectSubRect(raceButtonQuad, ts_raceButton->GetSubRect(f));
		}
		else
		{
			SetRectSubRect(raceButtonQuad, ts_raceButton->GetSubRect(2));
		}
	}

	++frame;
}

bool PracticeUserBox::IsEmpty()
{
	return action == A_EMPTY;
}

void PracticeUserBox::UpdateBGQuad()
{
	Color bgColor;
	switch (action)
	{
	case A_EMPTY:
	{
		if (selected)
		{
			bgColor = Color(100, 100, 100);
		}
		else
		{
			bgColor = Color(50, 50, 50);
		}

		break;
	}
	case A_HAS_PLAYER:
	{
		if (selected)
		{
			bgColor = Color(200, 200, 200);
		}
		else
		{
			bgColor = Color(150, 150, 150);
		}

		break;
	}
	case A_PLAYER_WANTS_TO_PLAY:
	{
		if (selected)
		{
			bgColor = Color::Green;
		}
		else
		{
			bgColor = Color::Magenta;
		}
		break;
	}
	}

	SetRectColor(bgQuad, bgColor);
}

void PracticeUserBox::SetAction(int a)
{
	action = a;

	frame = 0;

	/*A_EMPTY,
	A_HAS_PLAYER,
	A_PLAYER_WANTS_TO_PLAY,
	A_RUNNING,*/

	
}

void PracticeUserBox::SetPlayer(PracticePlayer *pp )//const std::string &name, int skinIndex)
{
	assert(IsEmpty());

	practicePlayer = pp;

	if (practicePlayer->wantsToPlay)
	{
		SetAction(A_PLAYER_WANTS_TO_PLAY);
	}
	else
	{
		SetAction(A_HAS_PLAYER);
	}

	playerDisp.SetName(practicePlayer->name);
	playerDisp.SetSkin(practicePlayer->skinIndex);
}

void PracticeUserBox::SetTopLeft( sf::Vector2f & p_topLeft)
{
	topLeft = p_topLeft;

	Vector2f halfSize(disp->boxSize.x / 2.f, disp->boxSize.y / 2.f);

	playerDisp.SetTopLeft( topLeft );

	SetRectTopLeft(raceButtonQuad, ts_raceButton->tileWidth, ts_raceButton->tileHeight, Vector2f(playerDisp.nameText.getPosition().x, topLeft.y + 30));

	SetRectTopLeft(bgQuad, disp->boxSize.x, disp->boxSize.y, topLeft);
	//SetRectTopLeft(buttonQuads, buttonSize, buttonSize, Vector2f( topLeft.x, bottomOfKin + 10));
	//SetRectTopLeft(buttonQuads + 4, buttonSize, buttonSize, Vector2f(topLeft.x, bottomOfKin + 10 + buttonSize + 10));
}

void PracticeUserBox::InvitePlayer()
{
	assert(!IsEmpty());
	//if (!practicePlayer->hasBeenInvited)
	{
		//disp->sess->netplayManager->TryInvitePracticePlayer(*practicePlayer);
	}
	//else
	{
		//cant invite someone who has already been invited!
	}
}

void PracticeUserBox::RequestRace()
{
	if (disp->sess->netplayManager->RequestPracticePlayerToRace(*practicePlayer))
	{
		disp->sess->netplayManager->SetupNetplayPlayersFromPractice(false);
	}
	//SetAction(A_WAITING_FOR_CONFIRM);
}

void PracticeUserBox::SetSelected(bool sel)
{
	selected = sel;
}

void PracticeUserBox::Draw(sf::RenderTarget *target)
{
	/*A_EMPTY,
	A_HAS_PLAYER,
	A_INVITED_PLAYER,
	A_HAS_ALREADY_BEEN_INVITED,
	A_HAS_INVITED_ME_AND_I_INVITED_THEM,
	A_READY_TO_RUN,
	A_RUNNING,*/

	if (!IsEmpty())
	{
		target->draw( bgQuad, 4, sf::Quads);

		playerDisp.Draw(target);

		
		if (action == A_PLAYER_WANTS_TO_PLAY)
		{
			target->draw(raceButtonQuad, 4, sf::Quads, ts_raceButton->texture);
		}
	}
}

PracticeInviteDisplay::PracticeInviteDisplay()
{
	sess = Session::GetSession();

	userBoxVec.resize(NUM_BOXES);

	ts_buttons = NULL;

	boxSize.x = 300;
	boxSize.y = 70;

	totalSize.x = boxSize.x;
	totalSize.y = boxSize.y * NUM_BOXES;

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		userBoxVec[i] = new PracticeUserBox(this);
	}

	availableText.setFont(sess->mainMenu->arial);
	availableText.setFillColor(Color::Green);
	availableText.setCharacterSize(24);
	availableText.setString("Availability: ON");
	auto lb = availableText.getLocalBounds();
	availableText.setOrigin(0, lb.top + lb.height / 2);

	unavailableText.setFont(sess->mainMenu->arial);
	unavailableText.setFillColor(Color::Red);
	unavailableText.setCharacterSize(24);
	unavailableText.setString("Availability: OFF");
	lb = unavailableText.getLocalBounds();
	unavailableText.setOrigin(0, lb.top + lb.height / 2);

	selectedIndex = 0;
	userBoxVec[0]->SetSelected(true);

	SetTopCenter(Vector2f(960, 115));

	isHosting = false;
	hostMode = -1;
	hostNumMaxPlayers - 1;
	hostPowerMode = -1;

	SetAction(A_IDLE);
}

PracticeInviteDisplay::~PracticeInviteDisplay()
{
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		delete userBoxVec[i];
	}
}

void PracticeInviteDisplay::Reset()
{
	frame = 0;
	Populate();
	UpdateButtonIconsWhenControllerIsChanged();
}

void PracticeInviteDisplay::SetAction(int a)
{
	action = a;
	frame = 0;
}

bool PracticeInviteDisplay::Update(const ControllerState & curr, const ControllerState &prev)
{
	if (action == A_IDLE)
	{
		return false;
	}

	if (action == A_SHOW_PLAYERS)
	{
		if (curr.start && !prev.start)
		{
			sess->netplayManager->SetPracticeWantsToPlayStatus(!sess->netplayManager->wantsToPracticeRace);
		}

		if (curr.PUp() && !prev.PUp())
		{
			return false;
		}
		else if (curr.B && !prev.B)
		{
			return false;
		}
		else if (curr.A && !prev.A)
		{
			if (!userBoxVec[selectedIndex]->IsEmpty() && userBoxVec[selectedIndex]->practicePlayer->wantsToPlay)
			{
				userBoxVec[selectedIndex]->RequestRace();
				SetAction(A_PREPARING_TO_LEAVE);
			}
		}
		else if (curr.LUp() && !prev.LUp())
		{
			userBoxVec[selectedIndex]->SetSelected(false);

			--selectedIndex;
			if (selectedIndex < 0)
			{
				selectedIndex = NUM_BOXES - 1;
			}

			userBoxVec[selectedIndex]->SetSelected(true);
		}
		else if (curr.LDown() && !prev.LDown())
		{
			userBoxVec[selectedIndex]->SetSelected(false);

			++selectedIndex;
			if (selectedIndex == NUM_BOXES)
			{
				selectedIndex = 0;
			}

			userBoxVec[selectedIndex]->SetSelected(true);
		}

		for (int i = 0; i < NUM_BOXES; ++i)
		{
			userBoxVec[i]->Update();
		}

		for (int i = 0; i < NUM_BOXES; ++i)
		{
			if (userBoxVec[i]->action == PracticeUserBox::A_RUNNING)
			{
				sess->netplayManager->SetupNetplayPlayersFromPractice(true);
				//sess->netplayManager->ConfirmPracticePlayerRace(*(userBoxVec[i]->practicePlayer));
				SetAction(A_PREPARING_TO_LEAVE);
				break;
			}
		}

		assert(sess->netplayManager != NULL);
		if (sess->netplayManager->wantsToPracticeRace)
		{
			wantsToPlay = true;
		}
		else
		{
			wantsToPlay = false;
		}
	}
	/*else if (action == A_REQUESTING_TO_RACE)
	{
		for (int i = 0; i < NUM_BOXES; ++i)
		{
			userBoxVec[i]->Update();
		}

		for (int i = 0; i < NUM_BOXES; ++i)
		{
			if (userBoxVec[i]->action == PracticeUserBox::A_RACE_CONFIRMED)
			{
				SetAction(A_PREPARING_TO_LEAVE);
				break;
			}
		}
	}*/
	else if (action == A_PREPARING_TO_LEAVE)
	{
		if (frame == 120)
		{
			SetAction(A_RUN_GAME);
		}
	}

	++frame;
	return true;
}

void PracticeInviteDisplay::PrepareToLeave()
{
	SetAction(A_PREPARING_TO_LEAVE);
}

bool PracticeInviteDisplay::IsTryingToStartMatch()
{
	if (action == A_RUN_GAME)
		return true;
	//if (boxVec[selectedIndex]->action == InvitePlayerBox::A_RUNNING)
	//	return true;

	return false;
}

void PracticeInviteDisplay::Populate()
{
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		userBoxVec[i]->Reset();
	}

	int boxIndex = 0;
	assert(sess->netplayManager != NULL);
	for (int i = 0; i < NetplayManager::MAX_PRACTICE_PLAYERS; ++i)
	{
		if (sess->netplayManager->practicePlayers[i].isConnectedTo)
		{
			userBoxVec[boxIndex]->SetPlayer(&sess->netplayManager->practicePlayers[i]);
			++boxIndex;

			if (boxIndex == NUM_BOXES)
				break;
		}
	}

	action = A_SHOW_PLAYERS;
}

void PracticeInviteDisplay::SetTopLeft(sf::Vector2f & p_topLeft)
{
	topLeft = p_topLeft;

	Vector2f startHostingQuadSize(boxSize.x, 50);
	Vector2f startHostingQuadPos(topLeft.x + totalSize.x / 2 - startHostingQuadSize.x / 2, topLeft.y);
	SetRectTopLeft(startHostingQuad, startHostingQuadSize.x, startHostingQuadSize.y, startHostingQuadPos);
	SetRectColor(startHostingQuad, Color(100, 100, 100));

	float buttonSize = 48;
	SetRectTopLeft(buttonQuads, buttonSize, buttonSize, startHostingQuadPos);

	availableText.setPosition(startHostingQuadPos + Vector2f(buttonSize + 10, buttonSize / 2));
	unavailableText.setPosition(startHostingQuadPos + Vector2f(buttonSize + 10, buttonSize / 2));


	Vector2f currBoxtopLeft = topLeft + Vector2f( 0, startHostingQuadSize.y + 15 );
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		userBoxVec[i]->SetTopLeft(currBoxtopLeft);
		currBoxtopLeft.y += boxSize.y;
	}
}

void PracticeInviteDisplay::UpdateButtonIconsWhenControllerIsChanged()
{
	MainMenu *mainMenu = MainMenu::GetInstance();

	assert(!sess->IsParallelSession());
	assert(sess->controllerStates[0] != NULL);

	int cType = sess->controllerStates[0]->GetControllerType();

	ts_buttons = sess->GetButtonIconTileset(cType);

	auto button = XBoxButton::XBOX_START;
	SetRectSubRect(buttonQuads, mainMenu->GetButtonIconTileForMenu(cType, button));

	button = XBoxButton::XBOX_A;
	SetRectSubRect(buttonQuads + 4, mainMenu->GetButtonIconTileForMenu(cType, button));
}

void PracticeInviteDisplay::SetCenter(sf::Vector2f center)
{
	SetTopLeft(Vector2f(center.x - totalSize.x / 2, center.y - totalSize.y / 2));
}

void PracticeInviteDisplay::SetTopCenter(sf::Vector2f pos)
{
	SetTopLeft(Vector2f(pos.x - totalSize.x / 2, pos.y));
}

void PracticeInviteDisplay::Draw(sf::RenderTarget *target)
{
	if ( action == A_PREPARING_TO_LEAVE || action == A_RUN_GAME ) //action == A_REQUESTING_TO_RACE || 
	{
		target->draw(startHostingQuad, 4, sf::Quads);
	}
	else
	{
		for (int i = 0; i < NUM_BOXES; ++i)
		{
			userBoxVec[i]->Draw(target);
		}

		target->draw(startHostingQuad, 4, sf::Quads);

		if (wantsToPlay)
		{
			target->draw(availableText);
		}
		else
		{
			target->draw(unavailableText);
		}


		target->draw(buttonQuads, 2 * 4, sf::Quads, ts_buttons->texture);
	}
}