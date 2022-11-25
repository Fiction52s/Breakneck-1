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

	playerName = fps->panel->AddLabel("p" + to_string(index) + "label", Vector2i(0, 0), 30, "");

	numberText = fps->panel->AddLabel("p" + to_string(index) + "numberlabel", Vector2i(0, 0), 60, "");

	SetRectColor(bgQuad, Color::Red);

	SetTopLeft(Vector2i(0, 0));//Vector2i(100, 700) + Vector2i(index * (spacing + width), 0));

	ClearInfo();

	Show();
}

void FreeplayPlayerBox::ClearInfo()
{
	controllerType = -1;
	controllerIndex = -1;
	joinOrder = -1;
	SetName("");
	numberText->setString("");
}

void FreeplayPlayerBox::Show()
{
	show = true;
	playerName->setString(playerNameStr);
	SetTopLeft(topLeft);
	numberText->setString(to_string(index));
	numberText->setOrigin(numberText->getLocalBounds().left + numberText->getLocalBounds().width / 2,
		numberText->getLocalBounds().top + numberText->getLocalBounds().height / 2);
}
void FreeplayPlayerBox::Hide()
{
	show = false;
	playerName->setString("");
	numberText->setString("");
}

void FreeplayPlayerBox::SetTopLeft(sf::Vector2i &pos)
{
	topLeft = pos;

	Vector2i center(topLeft.x + fps->playerBoxWidth / 2, topLeft.y + fps->playerBoxHeight / 2);

	numberText->setPosition(Vector2f(center));

	Vector2i namePos(fps->playerBoxWidth / 2, 0);

	SetRectTopLeft(bgQuad, fps->playerBoxWidth, fps->playerBoxHeight, Vector2f(topLeft));

	playerName->setPosition(Vector2f(pos + namePos));
	auto &bounds = playerName->getLocalBounds();
	playerName->setPosition(Vector2f(playerName->getPosition().x - (bounds.left + bounds.width / 2), playerName->getPosition().y));
}

void FreeplayPlayerBox::SetName(const std::string &name)
{
	playerNameStr = name;

	if (show)
	{
		playerName->setString(playerNameStr);
		SetTopLeft(topLeft);
	}
}

void FreeplayPlayerBox::Draw(sf::RenderTarget *target)
{
	if (!show)
		return;

	target->draw(bgQuad, 4, sf::Quads);
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

	joinHoldFrames = 30;

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
	joinedControllers.clear();
}

void FreeplayScreen::Quit()
{

}

bool FreeplayScreen::HandleEvent(sf::Event ev)
{
	return false;//browserHandler->chooser->panel->HandleEvent(ev);
}

void FreeplayScreen::Update()
{
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

void FreeplayScreen::TryControllerJoin(GameController *con)
{
	if (joinedControllers.size() == 4)
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

	bool alreadyJoined = false;
	for (auto it = joinedControllers.begin(); it != joinedControllers.end(); ++it)
	{
		if ((*it) == con)
		{
			alreadyJoined = true;
			break;
		}
	}

	if (!alreadyJoined)
	{
		joinedControllers.push_back(con);
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