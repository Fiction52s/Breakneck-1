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

	if (practicePlayer != NULL )
	{
		if (disp->sess->netplayManager->receivedPracticeRaceStartRequestIndex == practicePlayer->practicePlayerIndex)
		{
			SetAction(A_RUNNING);
		}
		else if (action == A_RUNNING)
		{
			SetAction(A_HAS_PLAYER);
		}
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
	if (disp->sess->netplayManager->SendRequestPracticePlayerToRace(*practicePlayer))
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






PracticeRaceCountdownBox::PracticeRaceCountdownBox(PracticeInviteDisplay *p_disp)
{
	disp = p_disp;
	totalFrames = 300;

	defaultSize.x = 300;
	defaultSize.y = 300;
	size = defaultSize;

	action = A_ACTIVE;

	matchTitleText.setFont(disp->sess->mainMenu->arial);
	matchTitleText.setFillColor(Color::White);
	matchTitleText.setCharacterSize(24);

	timerText.setFont(disp->sess->mainMenu->arial);
	timerText.setFillColor(Color::White);
	timerText.setCharacterSize(64);

	cancelText.setFont(disp->sess->mainMenu->arial);
	cancelText.setFillColor(Color::White);
	cancelText.setCharacterSize(40);
	cancelText.setString("CANCEL");

	SetRectColor(bgQuad, Color(100, 100, 100, 200));

	SetCenter(Vector2f(960, 540- 200));
}

void PracticeRaceCountdownBox::Activate(int mode, const std::string &oppName)
{
	framesRemaining = totalFrames;

	action = A_ACTIVE;

	string modeStr;
	switch (mode)
	{
	case MatchParams::GAME_MODE_PARALLEL_RACE:
	{
		modeStr = "Speedrun race";
		break;
	}
	default:
	{
		modeStr = "mode not set";
		break;
	}
	}

	matchTitleText.setString(modeStr + " with\n" + oppName + " starting in...");

	UpdateButtonIconsWhenControllerIsChanged();

	if (matchTitleText.getGlobalBounds().width > defaultSize.x)
	{
		size.x = matchTitleText.getGlobalBounds().width + 20;
	}
	else
	{
		size.x = defaultSize.x;
	}

	SetCenter(Vector2f(960, 540 - 200));
}

bool PracticeRaceCountdownBox::IsCancelledByMe()
{
	return action == A_CANCELLED_BY_ME;
}

bool PracticeRaceCountdownBox::IsCancelledByOpponent()
{
	return action == A_CANCELLED_BY_OPPONENT;
}

bool PracticeRaceCountdownBox::IsReadyToStart()
{
	return action == A_READY_TO_START;
}

void PracticeRaceCountdownBox::SetTopLeft(Vector2f p_topLeft)
{
	topLeft = p_topLeft;

	SetRectTopLeft(bgQuad, size.x, size.y, topLeft);

	matchTitleText.setPosition(topLeft + Vector2f( 10, 5 ));

	Vector2f center = topLeft + size / 2.f;
	timerText.setPosition(center);

	float xCenter = topLeft.x + size.x / 2.f;
	float buttonSize = 48;
	float textWidth = cancelText.getGlobalBounds().width;
	float spacing = 10;
	float textAndButtonWidth = buttonSize + textWidth + spacing;

	Vector2f cancelButtonPos = Vector2f(xCenter - textAndButtonWidth / 2.f, topLeft.y + size.y - 70);
	SetRectTopLeft(buttonQuads, buttonSize, buttonSize, cancelButtonPos);

	cancelText.setPosition(Vector2f(cancelButtonPos.x + buttonSize + spacing, cancelButtonPos.y));
}

void PracticeRaceCountdownBox::SetCenter(Vector2f p_center)
{
	SetTopLeft(p_center - size / 2.f);
}

void PracticeRaceCountdownBox::UpdateButtonIconsWhenControllerIsChanged()
{
	MainMenu *mainMenu = MainMenu::GetInstance();

	int cType = disp->sess->controllerStates[0]->GetControllerType();

	auto button = XBoxButton::XBOX_B;
	SetRectSubRect(buttonQuads, mainMenu->GetButtonIconTileForMenu(cType, button));

	//button = XBoxButton::XBOX_A;
	//SetRectSubRect(buttonQuads + 4, mainMenu->GetButtonIconTileForMenu(cType, button));
}

void PracticeRaceCountdownBox::Update(const ControllerState & curr, const ControllerState &prev)
{
	if (action != A_ACTIVE)
	{
		return;
	}

	int oppIndex = disp->sess->netplayManager->GetPracticeRaceOpponentPracticeIndex();
	if (oppIndex == -1)
	{
		action = A_CANCELLED_BY_OPPONENT;
	}

	if (framesRemaining == 0)
	{
		//framesRemaining = totalFrames;
		action = A_READY_TO_START;
	}
	else if (curr.B && !prev.B)
	{
		action = A_CANCELLED_BY_ME;
	}
	else
	{
		int remainingSecs = ceil(framesRemaining / 60.f);

		timerText.setString(to_string(remainingSecs));
		auto lb = timerText.getLocalBounds();
		timerText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);

		--framesRemaining;
	}
}

void PracticeRaceCountdownBox::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);

	target->draw(matchTitleText);

	target->draw(timerText);
	
	target->draw(buttonQuads, 4, sf::Quads, disp->ts_buttons->texture);
	target->draw(cancelText);
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

	countdownBox = new PracticeRaceCountdownBox(this);

	SetAction(A_IDLE);
}

PracticeInviteDisplay::~PracticeInviteDisplay()
{
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		delete userBoxVec[i];
	}

	delete countdownBox;
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
		//doesn't happen
		return false;
	}

	if (action == A_SHOW_PLAYERS)
	{
		//PrepareToLeave();
		//return true;

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
				opponentIndex = selectedIndex;
				PrepareToLeave();
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
				opponentIndex = i;
				sess->netplayManager->SetupNetplayPlayersFromPractice(true);
				//sess->netplayManager->ConfirmPracticePlayerRace(*(userBoxVec[i]->practicePlayer));
				PrepareToLeave();
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
		countdownBox->Update( curr, prev );

		if (countdownBox->IsCancelledByMe())
		{
			sess->netplayManager->SendCancelPracticePlayerRace();
			SetAction(A_SHOW_PLAYERS);
		}
		else if (countdownBox->IsCancelledByOpponent())
		{
			//can also occur if you get disconnected
			SetAction(A_SHOW_PLAYERS);
		}
		else if (countdownBox->IsReadyToStart())
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


	int ind = sess->netplayManager->GetPracticeRaceOpponentPracticeIndex();//receivedPracticeRaceStartRequestIndex;
	assert( ind >= 0);

	PracticePlayer &pracPlayer = sess->netplayManager->practicePlayers[ind];

	assert(pracPlayer.isConnectedTo);

	countdownBox->Activate(MatchParams::GAME_MODE_PARALLEL_RACE, pracPlayer.name);
	
	//countdownBox->Activate( MatchParams::GAME_MODE_PARALLEL_RACE, "WWWWWWWWWWWWWWWWWWWW");
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
		countdownBox->Draw(target);
		//target->draw(startHostingQuad, 4, sf::Quads);
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