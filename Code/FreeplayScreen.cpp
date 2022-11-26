#include <iostream>
#include "MainMenu.h"
#include "FreeplayScreen.h"
#include "UIMouse.h"
//#include "Input.h"

using namespace std;
using namespace sf;

FreeplayPlayerBox::FreeplayPlayerBox(FreeplayScreen *p_fps, int p_index)
{
	index = p_index;

	fps = p_fps;

	Font &f = MainMenu::GetInstance()->arial;

	playerNameText.setCharacterSize(30);
	playerNameText.setFont(f);
	playerNameText.setFillColor(Color::White);

	numberText.setCharacterSize(30);
	numberText.setFont(f);
	numberText.setFillColor(Color::White);

	pressText.setCharacterSize(30);
	pressText.setFont(f);
	pressText.setFillColor(Color::White);

	pressText.setString("Press Start");
	auto localB = pressText.getLocalBounds();
	pressText.setOrigin(localB.left + localB.width / 2, localB.top + localB.height / 2);

	SetRectColor(bgQuad, Color::Red);

	SetTopLeft(Vector2i(0, 0));//Vector2i(100, 700) + Vector2i(index * (spacing + width), 0));

	ClearInfo();

	Show();
}

void FreeplayPlayerBox::ClearInfo()
{
	controller = NULL;
	SetName("");
	action = A_WAITING_FOR_JOIN;
}

void FreeplayPlayerBox::Show()
{
	show = true;
	SetTopLeft(topLeft);
	//numberText->setString(to_string(index));
	//numberText->setOrigin(numberText->getLocalBounds().left + numberText->getLocalBounds().width / 2,
	//	numberText->getLocalBounds().top + numberText->getLocalBounds().height / 2);
}
void FreeplayPlayerBox::Hide()
{
	show = false;
	//playerNameText->setString("");
	//numberText->setString("");
}

void FreeplayPlayerBox::SetTopLeft(sf::Vector2i &pos)
{
	topLeft = pos;

	Vector2i center(topLeft.x + fps->playerBoxWidth / 2, topLeft.y + fps->playerBoxHeight / 2);

	pressText.setPosition(Vector2f(center));
	//numberText->setPosition(Vector2f(center));

	Vector2i namePos(fps->playerBoxWidth / 2, 0);

	SetRectTopLeft(bgQuad, fps->playerBoxWidth, fps->playerBoxHeight, Vector2f(topLeft));

	//playerName->setPosition(Vector2f(pos + namePos));
	//auto &bounds = playerName->getLocalBounds();
	//playerName->setPosition(Vector2f(playerName->getPosition().x - (bounds.left + bounds.width / 2), playerName->getPosition().y));
}

void FreeplayPlayerBox::SetController(GameController *con)
{
	controller = con;
	action = A_HAS_PLAYER;
}

void FreeplayPlayerBox::SetName(const std::string &name)
{
	playerNameStr = name;

	if (show)
	{
		playerNameText.setString(playerNameStr);
		SetTopLeft(topLeft);
	}
}

void FreeplayPlayerBox::Draw(sf::RenderTarget *target)
{
	if (!show)
		return;

	target->draw(bgQuad, 4, sf::Quads);

	if (action == A_HAS_PLAYER)
	{
		target->draw(playerNameText);
		target->draw(numberText);
	}
	else
	{
		target->draw(pressText);
	}
	
}

FreeplayScreen::FreeplayScreen(MainMenu *mm)
{
	mainMenu = mm;

	panel = new Panel("freeplayscreen", 1920, 1080, this, true);
	panel->SetColor(Color::Transparent);
	panel->SetCenterPos(Vector2i(960, 540));

	//startButton = panel->AddButton("start", Vector2i(20, panel->size.y - 100), Vector2f(100, 40), "START");
	//leaveButton = panel->AddButton("leave", Vector2i(20 + 200, panel->size.y - 100), Vector2f(100, 40), "LEAVE");

	//inviteButton = panel->AddButton("invite", Vector2i(20 + 200, panel->size.y - 200), Vector2f(270, 40), "INVITE FRIEND");

	SetRectColor(bgQuad, Color( 100, 100, 100 ));
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	for (int i = 0; i < 4; ++i)
	{
		playerBoxes[i] = new FreeplayPlayerBox(this, i);
	}

	int numBoxes = 4;
	playerBoxWidth = 300;
	playerBoxHeight = 300;
	playerBoxSpacing = 100;

	Vector2i left(960, 540 + 100);//50);//150
	if (numBoxes % 2 == 0)
	{
		left.x -= playerBoxSpacing / 2 + playerBoxWidth + (playerBoxSpacing + playerBoxWidth) * (numBoxes / 2 - 1);
	}
	else
	{
		left.x -= playerBoxWidth / 2 + (playerBoxSpacing + playerBoxWidth) * (numBoxes / 2);
	}

	//left + Vector2f(nodeSize / 2, 0) + Vector2f((pathLen + nodeSize) * node, 0);
	for (int i = 0; i < numBoxes; ++i)
	{
		playerBoxes[i]->SetTopLeft(left + Vector2i((playerBoxSpacing + playerBoxWidth) * i, 0));
	}

	joinHoldFrames = 1;

	Start();
}

FreeplayScreen::~FreeplayScreen()
{
	delete panel;
	for (int i = 0; i < 4; ++i)
	{
		delete playerBoxes[i];
	}
}

void FreeplayScreen::Start()
{
	SetAction(A_WAITING_FOR_PLAYERS);
	for (int i = 0; i < 4; ++i)
	{
		playerBoxes[i]->ClearInfo();
		gccHeldStartFrames[i] = 0;
		windowsHeldStartFrames[i] = 0;
	}
}

void FreeplayScreen::Quit()
{

}

bool FreeplayScreen::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void FreeplayScreen::Update()
{
	panel->MouseUpdate();

	for (int i = 0; i < 4; ++i)
	{
		if (CONTROLLERS.GetWindowsController(i)->GetUnfilteredState().start)
		{
			windowsHeldStartFrames[i]++;
		}
		else
		{
			windowsHeldStartFrames[i] = 0;
		}

		if (CONTROLLERS.GetGCController(i)->GetUnfilteredState().start)
		{
			gccHeldStartFrames[i]++;
		}
		else
		{
			gccHeldStartFrames[i] = 0;
		}
	}

	GameController *testController = NULL;
	for (int i = 0; i < 4; ++i)
	{
		TryControllerJoin(CONTROLLERS.GetWindowsController(i));
		TryControllerJoin(CONTROLLERS.GetGCController(i));
	}


	if (MOUSE.IsMouseRightClicked())
	{
		SetAction(A_READY);
	//mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN);
	}


	/*if (MOUSE.IsMouseRightClicked())
	{
		mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN);
	}*/
}

bool FreeplayScreen::IsFull()
{
	for (int i = 0; i < 4; ++i)
	{
		if (playerBoxes[i]->controller == NULL)
		{
			return false;
		}
	}

	return true;
}

bool FreeplayScreen::AlreadyJoined(GameController *con)
{
	for (int i = 0; i < 4; ++i)
	{
		if (playerBoxes[i]->controller == con)
		{
			return true;
		}
	}

	return false;
}

FreeplayPlayerBox *FreeplayScreen::GetNextOpenBox()
{
	for (int i = 0; i < 4; ++i)
	{
		if (playerBoxes[i]->controller == NULL)
		{
			return playerBoxes[i];
		}
	}

	return NULL;
}

void FreeplayScreen::TryControllerJoin(GameController *con)
{
	if( IsFull() )
		return;

	switch (con->GetCType())
	{
	case CTYPE_XBOX:
		if (windowsHeldStartFrames[con->GetIndex()] < joinHoldFrames)
			return;
		break;
	case CTYPE_GAMECUBE:
		if (gccHeldStartFrames[con->GetIndex()] < joinHoldFrames)
			return;
		break;
	}

	if (!AlreadyJoined(con))
	{
		FreeplayPlayerBox *next = GetNextOpenBox();
		assert(next != NULL);

		next->SetController(con);
		cout << "added controller: " << con->GetCType() << ", index: " << con->GetIndex() << endl;
	}
}

void FreeplayScreen::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);

	for (int i = 0; i < 4; ++i)
	{
		playerBoxes[i]->Draw(target);
	}

	panel->Draw(target);
}

void FreeplayScreen::SetAction(int a)
{
	action = a;
	frame = 0;
}

void FreeplayScreen::CancelCallback(Panel *p)
{
	SetAction(A_BACK);
}