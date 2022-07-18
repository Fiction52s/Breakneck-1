#include "WaitingRoom.h"
#include "MainMenu.h"
#include "NetplayManager.h"

using namespace std;
using namespace sf;

WaitingRoom::WaitingRoom()
{
	panel = new Panel("listchooser", 1000, 500, this, true);
	panel->SetCenterPos(Vector2i(960, 540));

	startButton = panel->AddButton("start", Vector2i(20, panel->size.y - 100), Vector2f(100, 40), "START");
	leaveButton = panel->AddButton("leave", Vector2i(20 + 200, panel->size.y - 100), Vector2f(100, 40), "LEAVE");
}

WaitingRoom::~WaitingRoom()
{
	delete panel;
}

void WaitingRoom::Update()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	switch (action)
	{
	case A_WAITING_FOR_MEMBERS:
	{
		/*if (netplayManager->lobbyManager->GetNumMembers())
		{

		}*/
		break;
	}
	case A_READY_TO_START:
	{
		break;
	}
	case A_STARTING:
	{
		break;
	}

	}
}

void WaitingRoom::Draw(sf::RenderTarget *target)\
{
	panel->Draw(target);
}

void WaitingRoom::OpenPopup()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	assert(netplayManager->lobbyManager->IsInLobby());

	startButton->HideMember();

	/*if (netplayManager->IsHost())
	{
		startButton->ShowMember();
	}
	else
	{
		
	}*/

	SetAction(A_WAITING_FOR_MEMBERS);
}

void WaitingRoom::ClosePopup()
{

}

void WaitingRoom::SetAction(Action a)
{
	action = a;
	frame = 0;

}

bool WaitingRoom::HandleEvent(sf::Event ev)
{

}


void WaitingRoom::ChooseRectEvent(ChooseRect *cr, int eventType)
{

}

void WaitingRoom::ButtonCallback(Button *b, const std::string & e)
{
	if (b == startButton)
	{
		
	}
	else if (b == leaveButton)
	{
		//go back
	}
}

void WaitingRoom::PanelCallback(Panel *p, const std::string & e)
{

}
