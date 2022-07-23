#include "WaitingRoom.h"
#include "MainMenu.h"
#include "NetplayManager.h"
#include "LobbyManager.h"

using namespace std;
using namespace sf;

WaitingRoom::WaitingRoom()
{
	panel = new Panel("waitingroom", 1000, 500, this, true);
	panel->SetCenterPos(Vector2i(960, 540));

	startButton = panel->AddButton("start", Vector2i(20, panel->size.y - 100), Vector2f(100, 40), "START");
	leaveButton = panel->AddButton("leave", Vector2i(20 + 200, panel->size.y - 100), Vector2f(100, 40), "LEAVE");

	panel->ReserveTextRects(4);

	panel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));
	for (int i = 0; i < 4; ++i)
	{
		memberNameRects[i] = panel->AddTextRect(ChooseRect::I_LOBBY_MEMBER, Vector2f(), sf::Vector2f(200, 40), "");
		memberNameRects[i]->Init();
		memberNameRects[i]->SetShown(false);

	}
	panel->StopAutoSpacing();

	panel->SetCancelButton(leaveButton);
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
		if (netplayManager->lobbyManager->GetNumMembers() == 2) //fix this later for more members
		{
			if (netplayManager->IsHost())
			{
				startButton->ShowMember();
			}
			action = A_READY_TO_START;
		}
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

	

	panel->MouseUpdate();
}

void WaitingRoom::Draw(sf::RenderTarget *target)\
{
	panel->Draw(target);
}

void WaitingRoom::OpenPopup()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	assert(netplayManager->lobbyManager->IsInLobby());

	netplayManager->lobbyManager->currWaitingRoom = this;

	startButton->HideMember();

	UpdateMemberList();

	ownerID = netplayManager->GetHostID();

	/*if (netplayManager->IsHost())
	{
		startButton->ShowMember();
	}
	else
	{
		startButton->HideMember();
	}*/

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
	return panel->HandleEvent(ev);
}


void WaitingRoom::ChooseRectEvent(ChooseRect *cr, int eventType)
{

}

void WaitingRoom::ButtonCallback(Button *b, const std::string & e)
{
	if (b == startButton)
	{
		SetAction(A_STARTING);
	}
	else if (b == leaveButton)
	{
		SetAction(A_LEAVE_ROOM);
		//go back
	}
}

void WaitingRoom::PanelCallback(Panel *p, const std::string & e)
{

}

void WaitingRoom::UpdateMemberList()
{
	for (int i = 0; i < 4; ++i)
	{
		memberNameRects[i]->SetShown(false);

	}

	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	auto &memberList = netplayManager->lobbyManager->currentLobby.memberList;
	int index = 0;
	for (auto it = memberList.begin();
		it != memberList.end(); ++it)
	{
		memberNameRects[index]->SetText((*it).name);
		memberNameRects[index]->SetShown(true);
		++index;
	}
}

void WaitingRoom::OnLobbyChatUpdateCallback(LobbyChatUpdate_t *pCallback)
{
	if (pCallback->m_ulSteamIDUserChanged == ownerID.ConvertToUint64())
	{
		uint32 flags = pCallback->m_rgfChatMemberStateChange;
		if ((flags & k_EChatMemberStateChangeLeft) || (flags & k_EChatMemberStateChangeDisconnected))
		{
			cout << "HOST LEFT THE LOBBY, SO I'LL LEAVE" << endl;
			SetAction(A_LEAVE_ROOM);
		}
	}
}
