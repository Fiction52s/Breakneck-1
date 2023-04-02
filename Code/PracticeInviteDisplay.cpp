#include "PracticeInviteDisplay.h"
#include "Tileset.h"
#include "Session.h"
#include "Actor.h"
#include "MainMenu.h"
#include "NetplayManager.h"

using namespace sf;
using namespace std;

InvitePlayerBox::InvitePlayerBox(PracticeInviteDisplay *p_disp)
	:playerSkinShader( "player" )
{
	disp = p_disp;

	ts_kin = disp->sess->GetPlayer(0)->STAND_GetTileset();

	kinSprite.setTexture(*ts_kin->texture);
	kinSprite.setTextureRect(ts_kin->GetSubRect(0));
	//kinSprite.setScale(2, 2 );
	kinSprite.setOrigin(kinSprite.getLocalBounds().width / 2, 0);
	playerSkinShader.SetSubRect(ts_kin, ts_kin->GetSubRect(0));

	playerSkinShader.SetSkin(0);

	ts_buttons = NULL;

	nameText.setFont(disp->sess->mainMenu->arial);
	nameText.setFillColor(Color::White);
	nameText.setCharacterSize(24);

	Color bottomTextColor = Color::Black;

	inviteText.setFont(disp->sess->mainMenu->arial);
	inviteText.setFillColor(bottomTextColor);
	inviteText.setCharacterSize(24);
	inviteText.setString("Invite");
	auto lb = inviteText.getLocalBounds();
	inviteText.setOrigin(0, lb.top + lb.height / 2);

	acceptText.setFont(disp->sess->mainMenu->arial);
	acceptText.setFillColor(bottomTextColor);
	acceptText.setCharacterSize(24);
	acceptText.setString("Accept invite");
	lb = acceptText.getLocalBounds();
	acceptText.setOrigin(0, lb.top + lb.height / 2);

	runText.setFont(disp->sess->mainMenu->arial);
	runText.setFillColor(bottomTextColor);
	runText.setCharacterSize(24);
	runText.setString("Start\nMatch");
	lb = runText.getLocalBounds();
	runText.setOrigin(0, lb.top + lb.height / 2);

	waitingText.setFont(disp->sess->mainMenu->arial);
	waitingText.setFillColor(bottomTextColor);
	waitingText.setCharacterSize(24);
	waitingText.setString("Waiting\nfor response...");
	lb = waitingText.getLocalBounds();
	waitingText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);

	//testing code
	//SetName("test");
	UpdateButtonIconsWhenControllerIsChanged();
}

void InvitePlayerBox::Reset()
{
	action = A_EMPTY;
	practicePlayer = NULL;
	nameText.setString("");

	//SetName("test");
}

void InvitePlayerBox::Update()
{
	/*A_EMPTY,
		A_HAS_PLAYER,
		A_INVITED_PLAYER,
		A_PLAYER_HAS_INVITED_ME,
		A_READY_TO_RUN,
		A_RUNNING,*/
	if (action == A_EMPTY)
		return;

	if (!practicePlayer->isConnectedTo)
	{
		SetAction(A_EMPTY);
		return;
	}

	switch (action)
	{
	case A_HAS_PLAYER:
	{
		if (practicePlayer->hasBeenInvited)
		{
			SetAction(A_INVITED_PLAYER);
		}
		break;
	}
	case A_INVITED_PLAYER:
	{
		if (practicePlayer->hasBeenInvited && practicePlayer->hasAcceptedInvite)
		{
			SetAction(A_READY_TO_RUN);
		}
		break;
	}
	}
}

bool InvitePlayerBox::IsEmpty()
{
	return action == A_EMPTY;
}

void InvitePlayerBox::SetAction(int a)
{
	action = a;
	UpdateButtonIconAndTextPositions();
}

void InvitePlayerBox::SetPlayer(PracticePlayer *pp )//const std::string &name, int skinIndex)
{
	assert(IsEmpty());

	UpdateButtonIconsWhenControllerIsChanged();

	SetAction(A_HAS_PLAYER);

	practicePlayer = pp;

	SetName(practicePlayer->name);
	SetSkin(practicePlayer->skinIndex);
}


void InvitePlayerBox::UpdateButtonIconsWhenControllerIsChanged()
{
	MainMenu *mainMenu = MainMenu::GetInstance();

	int cType = disp->sess->controllerStates[0]->GetControllerType();

	ts_buttons = disp->sess->GetButtonIconTileset(cType);

	auto button = XBoxButton::XBOX_X;
	SetRectSubRect(buttonQuads, mainMenu->GetButtonIconTileForMenu(cType, button));

	button = XBoxButton::XBOX_A;
	SetRectSubRect(buttonQuads + 4, mainMenu->GetButtonIconTileForMenu(cType, button));
}

void InvitePlayerBox::UpdateButtonIconAndTextPositions()
{
	float bottomOfKin = kinSprite.getGlobalBounds().top + kinSprite.getGlobalBounds().height;
	float bottomOfRect = topLeft.y + disp->boxSize.y;

	float iconMiddleY = (bottomOfKin + bottomOfRect) / 2.f;

	float buttonSize = 48;

	ClearRect(buttonQuads);
	ClearRect(buttonQuads + 4);

	float spacingBetweenIconAndText = 10;
	
	SetRectTopLeft(modeQuad, disp->boxSize.x, bottomOfRect - bottomOfKin, Vector2f(topLeft.x, bottomOfKin));

	float totalTextAndIconWidth = 0, iconX = 0;

	float centerX = topLeft.x + disp->boxSize.x / 2.f;

	totalTextAndIconWidth = inviteText.getGlobalBounds().width + spacingBetweenIconAndText + buttonSize;
	
	iconX = centerX - totalTextAndIconWidth / 2.f;


	
	/*A_EMPTY,
		A_HAS_PLAYER,
		A_INVITED_PLAYER,
		A_HAS_ALREADY_BEEN_INVITED,
		A_HAS_INVITED_ME_AND_I_INVITED_THEM,
		A_READY_TO_RUN,
		A_RUNNING,*/

	if (action == A_HAS_PLAYER)
	{
		totalTextAndIconWidth = inviteText.getGlobalBounds().width + spacingBetweenIconAndText + buttonSize;
		iconX = centerX - totalTextAndIconWidth / 2.f;


		SetRectTopLeft(buttonQuads, buttonSize, buttonSize, Vector2f(iconX, iconMiddleY - buttonSize / 2));
		inviteText.setPosition(iconX + buttonSize + spacingBetweenIconAndText, iconMiddleY);

		Color c(Color::Cyan);
		//c.a = 50;

		SetRectColor(modeQuad, c);
	}
	else if (action == A_INVITED_PLAYER)
	{
		waitingText.setPosition(centerX, iconMiddleY);
		Color c(Color::Yellow);
		//c.a = 50;
		SetRectColor(modeQuad, c);
	}
	else if (action == A_PLAYER_HAS_INVITED_ME)
	{
		totalTextAndIconWidth = acceptText.getGlobalBounds().width + spacingBetweenIconAndText + buttonSize;
		iconX = centerX - totalTextAndIconWidth / 2.f;

		Color c(Color::Magenta);
		//c.a = 50;
		SetRectColor(modeQuad, c);

		acceptText.setPosition(iconX + buttonSize + spacingBetweenIconAndText, iconMiddleY);
	}
	else if (action == A_READY_TO_RUN)
	{
		totalTextAndIconWidth = runText.getGlobalBounds().width + spacingBetweenIconAndText + buttonSize;
		iconX = centerX - totalTextAndIconWidth / 2.f;

		Color c(Color::Green);
		//c.a = 50;
		SetRectColor(modeQuad, c);
		SetRectTopLeft(buttonQuads + 4, buttonSize, buttonSize, Vector2f(iconX, iconMiddleY - buttonSize / 2));
		runText.setPosition(iconX + buttonSize + spacingBetweenIconAndText, iconMiddleY);
	}
}

void InvitePlayerBox::SetSkin(int index)
{
	playerSkinShader.SetSkin(index);
}

void InvitePlayerBox::SetName(const std::string &name)
{
	nameText.setString(name);
	auto lb = nameText.getLocalBounds();
	nameText.setOrigin(lb.left + lb.width / 2, 0);
}

void InvitePlayerBox::SetTopLeft( sf::Vector2f & p_topLeft)
{
	topLeft = p_topLeft;

	Vector2f halfSize(disp->boxSize.x / 2.f, disp->boxSize.y / 2.f);

	kinSprite.setPosition(topLeft + Vector2f(halfSize.x, 24));
	nameText.setPosition(topLeft + Vector2f(halfSize.x, 0));

	UpdateButtonIconAndTextPositions();
	//SetRectTopLeft(buttonQuads, buttonSize, buttonSize, Vector2f( topLeft.x, bottomOfKin + 10));
	//SetRectTopLeft(buttonQuads + 4, buttonSize, buttonSize, Vector2f(topLeft.x, bottomOfKin + 10 + buttonSize + 10));
}

void InvitePlayerBox::InvitePlayer()
{
	assert(!IsEmpty());
	if (!practicePlayer->hasBeenInvited)
	{
		disp->sess->netplayManager->TryInvitePracticePlayer(*practicePlayer);
	}
	else
	{
		//cant invite someone who has already been invited!
	}
}

void InvitePlayerBox::Confirm()
{
	if (practicePlayer->hasInvitedMe && !practicePlayer->hasInvitedMeAndIAccepted )
	{
		disp->sess->netplayManager->TryAcceptPracticePlayerInvite(*practicePlayer);
	}
	else if ( action == A_READY_TO_RUN )
	{
		//practicePlayer->hasBeenInvited && practicePlayer->hasAcceptedInvite
		action = A_RUNNING;
	}
}

void InvitePlayerBox::Draw(sf::RenderTarget *target)
{
	/*A_EMPTY,
	A_HAS_PLAYER,
	A_INVITED_PLAYER,
	A_HAS_ALREADY_BEEN_INVITED,
	A_HAS_INVITED_ME_AND_I_INVITED_THEM,
	A_READY_TO_RUN,
	A_RUNNING,*/

	//if (!IsEmpty())
	{
		target->draw(modeQuad, 4, sf::Quads);

		target->draw(kinSprite, &playerSkinShader.pShader);
		target->draw(nameText);
		target->draw(buttonQuads, 2 * 4, sf::Quads, ts_buttons->texture);
		

		if( action == A_HAS_PLAYER )
		{
			target->draw(inviteText);
		}
		else if (action == A_INVITED_PLAYER)
		{
			target->draw(waitingText);
		}
		else if (action == A_PLAYER_HAS_INVITED_ME)
		{
			target->draw(acceptText);
		}
		else if (true)//(action == A_READY_TO_RUN)
		{
			target->draw(runText);
		}

	}
}

PracticeInviteDisplay::PracticeInviteDisplay()
{
	sess = Session::GetSession();

	boxVec.resize(NUM_BOXES);

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		boxVec[i] = new InvitePlayerBox(this);
	}

	boxSize.x = 200;
	boxSize.y = 200;

	totalSize.x = boxSize.x * NUM_BOXES;
	totalSize.y = boxSize.y;

	

	selectedIndex = 0;

	SetCenter(Vector2f(960, 540 - 200));


	SetRectColor(selectedQuad, Color(100, 100, 100, 200));
	SetRectColor(bgQuad, Color(100, 100, 100, 100));

	action = A_IDLE;
}

PracticeInviteDisplay::~PracticeInviteDisplay()
{
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		delete boxVec[i];
	}
}

void PracticeInviteDisplay::Reset()
{
	Populate();
}

bool PracticeInviteDisplay::Update(const ControllerState & curr, const ControllerState &prev)
{
	if (action == A_IDLE)
	{
		return false;
	}

	for (int i = 0; i < NUM_BOXES; ++i)
	{
		boxVec[i]->Update();
	}

	if (action == A_SHOW_PLAYERS)
	{
		if ((curr.PUp() && !prev.PUp()) || (curr.B && !prev.B) )
		{
			return false;
		}


		if (curr.LLeft() && !prev.LLeft())
		{
			--selectedIndex;
			if (selectedIndex < 0)
			{
				selectedIndex = NUM_BOXES - 1;
				/*for (int i = NUM_BOXES - 1; i >= 0; --i)
				{
					selectedIndex = i;

					if (!boxVec[i]->IsEmpty())
					{
						break;
					}
				}*/
			}

			UpdateSelectedQuad();
		}
		else if (curr.LRight() && !prev.LRight())
		{
			++selectedIndex;
			if (selectedIndex == NUM_BOXES )//|| boxVec[selectedIndex]->IsEmpty())
			{
				selectedIndex = 0;
			}

			UpdateSelectedQuad();
		}
		

		if (curr.X && !prev.X)
		{
			if (!boxVec[selectedIndex]->IsEmpty())
			{
				boxVec[selectedIndex]->InvitePlayer();
			}
		}
		else if (curr.A && !prev.A)
		{
			if (!boxVec[selectedIndex]->IsEmpty())
			{
				boxVec[selectedIndex]->Confirm();
			}
		}
	}

	return true;
}

bool PracticeInviteDisplay::IsTryingToStartMatch()
{
	if (boxVec[selectedIndex]->action == InvitePlayerBox::A_RUNNING)
		return true;

	return false;
}

void PracticeInviteDisplay::Populate()
{
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		boxVec[i]->Reset();
	}

	int boxIndex = 0;
	assert(sess->netplayManager != NULL);
	for (int i = 0; i < NetplayManager::MAX_PRACTICE_PLAYERS; ++i)
	{
		if (sess->netplayManager->practicePlayers[i].isConnectedTo)
		{
			boxVec[boxIndex]->SetPlayer(&sess->netplayManager->practicePlayers[i]);
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
	Vector2f currTopLeft = topLeft;
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		boxVec[i]->SetTopLeft(currTopLeft);
		currTopLeft.x += boxSize.x;
	}

	SetRectTopLeft(bgQuad, totalSize.x, totalSize.y, topLeft);

	UpdateSelectedQuad();
	
}

void PracticeInviteDisplay::UpdateSelectedQuad()
{
	auto gb = boxVec[0]->kinSprite.getGlobalBounds();
	float kinBottom = gb.top + gb.height;

	SetRectTopLeft(selectedQuad, boxSize.x, boxSize.y, boxVec[selectedIndex]->topLeft);
}

void PracticeInviteDisplay::SetCenter(sf::Vector2f center)
{
	SetTopLeft(Vector2f(center.x - totalSize.x / 2, center.y - totalSize.y / 2));
}

void PracticeInviteDisplay::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
	target->draw(selectedQuad, 4, sf::Quads );
	for (int i = 0; i < NUM_BOXES; ++i)
	{
		boxVec[i]->Draw(target);
	}
}